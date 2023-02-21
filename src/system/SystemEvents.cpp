//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//
// LunokWatch is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later 
// version.
//
// LunokWatch is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
// details.
//
// You should have received a copy of the GNU General Public License along with 
// LunokWatch. If not, see <https://www.gnu.org/licenses/>. 
//

#include <Arduino.h>
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo library
#include <ArduinoNvs.h>
#include "../lunokIoT.hpp"
#include "SystemEvents.hpp"

#include "lunokiot_config.hpp"
#include <FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portable.h>
#include <freertos/task.h>

#include <esp_event.h>
#include <esp_event_base.h>
#include <WiFi.h>

#include <esp_wifi.h>
#include <wifi_provisioning/manager.h>

#include <esp_task_wdt.h>
#include "Network/BLE.hpp"

#include "UI/UI.hpp"

#include "../app/Shutdown.hpp"
#include "../app/Provisioning2.hpp"
#include "../app/WatchfaceAlwaysOn.hpp"

#include <HTTPClient.h>

#include "../app/LogView.hpp"
#include "../app/Battery.hpp"

#include <cmath>

#include "driver/uart.h"

#include <esp_heap_caps.h>
#include <esp_timer.h>

#include <NimBLECharacteristic.h>

#include <esp_ota_ops.h>
#include "Datasources/database.hpp"
#include "esp_heap_task_info.h"
#include <freertos/portable.h>
#include "Network/BLE.hpp"
//https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-reference/system/mem_alloc.html#_CPPv431heap_caps_get_minimum_free_size8uint32_t

#include "../app/Lamp.hpp"

#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>

Ticker LunokIoTSystemTicker; // This loop is the HEART of system <3 <3 <3

uint32_t systemStatsBootCounter = 0;
uint32_t systemStatsRebootCounter = 0;
uint32_t systemStatsCrashCounter = 0;

uint32_t deviceSleepMSecs = 0; // time wasted in lightSleep waiting user interactions (forever alone time x'D)
unsigned long deviceUsageMSecs=0; // time interacted with the user
float deviceUsageRatio=100.0;   // device usage percentage ratio

extern bool UIRunning;
bool systemSleep = false;
// Event loops
esp_event_loop_handle_t systemEventloopHandler;
ESP_EVENT_DEFINE_BASE(SYSTEM_EVENTS);

int hallData = 0;         // hall sensor data
bool vbusPresent = false; // USB connected?
int batteryPercent;       // -1 if no batt
uint8_t bmaRotation;      // bma ground?
float axpTemp;
float bmaTemp;
float cpuTemp;
const char *currentActivity = "None";
uint32_t stepCount = 0;
uint32_t lastBootStepCount = 0;

int16_t accXMax = -5000;
int16_t accXMin = 5000;
int16_t accYMax = -5000;
int16_t accYMin = 5000;
int16_t accZMax = -5000;
int16_t accZMin = 5000;

int16_t pcX = 0; // percent
int16_t pcY = 0;
int16_t pcZ = 0;
int16_t lpcX = 0; // last percent
int16_t lpcY = 0;
int16_t lpcZ = 0;
float degX = 0.0; // degrees
float degY = 0.0;
float degZ = 0.0;
int16_t accX = 0; // accelerator value
int16_t accY = 0;
int16_t accZ = 0;

unsigned long beginBMAActivity = 0;
uint32_t beginStepsBMAActivity = 0;
uint32_t stepsBMAActivityStationary = 0;
unsigned long timeBMAActivityStationary = 0;
uint32_t stepsBMAActivityWalking = 0;
unsigned long timeBMAActivityWalking = 0;
uint32_t stepsBMAActivityRunning = 0;
unsigned long timeBMAActivityRunning = 0;
uint32_t stepsBMAActivityInvalid = 0;
unsigned long timeBMAActivityInvalid = 0;
uint32_t stepsBMAActivityNone = 0;
unsigned long timeBMAActivityNone = 0;

volatile static bool irqAxp = false;
volatile static bool irqBMA = false;
volatile static bool irqRTC = false;


extern NimBLECharacteristic *battCharacteristic;
SemaphoreHandle_t I2cMutex = xSemaphoreCreateMutex();


extern "C" // Extern C is used when we are using a funtion written in "C" language in a C++ code.
{
  uint8_t temprature_sens_read(); // This function is written in C language
}
uint8_t temprature_sens_read(); // on esp_idf 4.4 is faked to 55.33333 C :(

void LunokIoTSystemTickerCallback() { // freeRTOS discourages process on callback due priority and recomends a queue :)
    if (false == ttgo->bl->isOn()) { return; } // only when screen is on
    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_TICK, nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
}

void LunokIoTSystemTickerStop() {
    LunokIoTSystemTicker.detach();
}

void LunokIoTSystemTickerStart() {
    // Start the tick loop
    LunokIoTSystemTicker.attach_ms(LUNOKIOT_SYSTEM_HEARTBEAT_TIME,LunokIoTSystemTickerCallback);
}

bool WakeUpReason() { // this function decides the system must wake or sleep

// if call return, the system remains active but with the screen off (must call DoSleep in other place)
//
// at this time only 3 interrupts returns:
//
// esp timer int
// AXP202 int
// BMA432 int

    esp_sleep_wakeup_cause_t wakeup_reason;
    uint64_t GPIO_reason = 0;
    //int impliedGPIO = -1;
    bool continueSleep=true; // by default return to sleep
    wakeup_reason = esp_sleep_get_wakeup_cause();
    lEvLog("ESP32 Wake up from: ");
    if ( ESP_SLEEP_WAKEUP_UNDEFINED == wakeup_reason) {
        //!< In case of deep sleep, reset was not caused by exit from deep sleep
        lLog("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    } else if ( ESP_SLEEP_WAKEUP_EXT0 == wakeup_reason) {
        //!< Wakeup caused by external signal using RTC_IO
        lLog("Interrupt triggered on ext0 <-- (PMU) AXP202\n");
        irqAxp=true;

        //lLog("Interrupt triggered on ext0 <-- (IMU) BMA423\n");
        //irqBMA = true; // faked

        continueSleep=false; // don't want to sleep more

    } else if ( ESP_SLEEP_WAKEUP_EXT1 == wakeup_reason) {    // ESP_SLEEP_WAKEUP_BT
        lLog("Interrupt triggered on ext1 <-- ");
        //!< Wakeup caused by external signal using RTC_CNTL
        GPIO_reason = esp_sleep_get_ext1_wakeup_status();
        double GPIO_pin = ((log(GPIO_reason))/log(2));
        if ( 0 == GPIO_reason) {
            lLog("WARNING: wake from other source?\n");
        }
        continueSleep=false;

        if (GPIO_SEL_35 == GPIO_reason) {
            lLog("(PMU) AXP202\n");
            irqAxp=true;
        } else if (GPIO_SEL_37 == GPIO_reason) {
            lLog("(RTC) PCF8563\n");
            //continueSleep=false; // wait to get the int
            irqRTC=true;
        } else if (GPIO_SEL_38 == GPIO_reason) {
            lLog("(TOUCH) FocalTech\n");
            //continueSleep=false; // wait to get the int
        } else if (GPIO_SEL_39 == GPIO_reason) {
            lLog("(IMU) BMA423\n");
            irqBMA = true; // faked
            //continueSleep=false; // wait to get the int
        } else {
            lLog("WARNING: Unexpected: GPIO %llu\n",GPIO_reason);
            //Serial.print((log(GPIO_reason))/log(2), 0); 
        }
        /*
        if (GPIO_SEL_37 == GPIO_reason) {
            lLog("(RTC) PCF8563\n");
        } else if (GPIO_SEL_38 == GPIO_reason) {
            lLog("(TOUCH) FocalTech\n");
        } else if (GPIO_SEL_39 == GPIO_reason) {
            lLog("(BMA) BMA423\n");
        } else if (0 == GPIO_reason) {
            lLog("@TODO Wakeup in ext1 ins't a GPIO event?\n");
        } else {
            lLog("WARNING: Unexpected: GPIO %u Reason: %u\n",impliedGPIO, GPIO_reason);
            // lLog((log(GPIO_reason))/log(2), 0);            
        }*/
    } else if ( ESP_SLEEP_WAKEUP_TIMER == wakeup_reason) {
        //!< Wakeup caused by timer
        lLog("Wakeup caused by timer\n");
        esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_TIMER, nullptr, 0, LUNOKIOT_EVENT_FAST_TIME_TICKS);
        continueSleep=false; // don't sleep again, trying to catch bluetooth events
    } else if ( ESP_SLEEP_WAKEUP_TOUCHPAD == wakeup_reason) {
        //!< Wakeup caused by touchpad
    } else if ( ESP_SLEEP_WAKEUP_ULP == wakeup_reason) {
        //!< Wakeup caused by ULP program
    } else if ( ESP_SLEEP_WAKEUP_GPIO == wakeup_reason) {
        //!< Wakeup caused by GPIO (light sleep only)
    } else if ( ESP_SLEEP_WAKEUP_UART == wakeup_reason) {
        //!< Wakeup caused by UART (light sleep only)
    } else if ( ESP_SLEEP_WAKEUP_WIFI == wakeup_reason) {
        //!< Wakeup caused by WIFI (light sleep only)
    } else if ( ESP_SLEEP_WAKEUP_COCPU == wakeup_reason) {
        //!< Wakeup caused by COCPU int
    } else if ( ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG == wakeup_reason) {
        //!< Wakeup caused by COCPU crash
    } else if ( ESP_SLEEP_WAKEUP_BT == wakeup_reason) {
        //!< Wakeup caused by BT (light sleep only)
    }
    if ( continueSleep ) {
        TakeAllSamples();
        DoSleep();
        return true;
    }
    // Start the tick loop
    LunokIoTSystemTickerStart();

    return false;
}

uint16_t doSleepThreads = 0;
SemaphoreHandle_t DoSleepTaskSemaphore = xSemaphoreCreateMutex();

extern bool wifiOverride;
static void DoSleepTask(void *args) {
    // https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-reference/system/esp_timer.html
    // what about change all timers to:
    // https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-reference/system/freertos.html#timer-api
    lEvLog("ESP32: DoSleep Trying to obtain the lock...\n");
    BaseType_t done = xSemaphoreTake(DoSleepTaskSemaphore, LUNOKIOT_EVENT_TIME_TICKS);
    if (pdTRUE != done) {
        lEvLog("ESP32: DoSleep DISCARDED: already in progress?\n");
        vTaskDelete(NULL);
    }

    doSleepThreads++;

    lEvLog("ESP32: DoSleep(%d) began!\n", doSleepThreads);
    BLEKickAllPeers(); // anounche to others ble peers: I'm 'out of duty'
    LunokIoTSystemTickerStop();

    // destroy the app if isn't the watchface
    if ( ( nullptr != currentApplication) && ( true != currentApplication->isWatchface() ) ) {
        // the current app isn't a watchface... get out!
        LaunchApplication(nullptr,false,true); // Synched App Stop
        delay(100);
    }
    while(LoT().IsNetworkInUse()) {
        lNetLog("Network in use, waiting to get a nap! ...\n");
        delay(1000);
    }
    if ( ttgo->bl->isOn()) {
        lSysLog("DoSleep cannot continue, screen is ON\n");
        systemSleep = false;
        xSemaphoreGive(DoSleepTaskSemaphore);
        vTaskDelete(NULL);
    }


    // forcing wifi to get out!
    if (WL_NO_SHIELD != WiFi.status() ) {
        if ( WIFI_MODE_NULL != WiFi.getMode() ) {
            lNetLog("WiFi: Must be disabled now!\n");
            WiFi.disconnect(true,true);
            delay(200);
            WiFi.mode(WIFI_OFF);
            delay(100);
        }
    }

    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_LIGHTSLEEP, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);

    int64_t howMuchTime = esp_timer_get_next_alarm();
    int64_t nowTime = esp_timer_get_time();
    uint32_t diffTime_secs = (howMuchTime-nowTime)/uS_TO_S_FACTOR;
    lEvLog("ESP32: Time from boot:  %lld secs\n",(nowTime/uS_TO_S_FACTOR));
    if ( diffTime_secs > 60 ) {
        uint32_t diffTime_mins = diffTime_secs/60;
        diffTime_secs%=60;
        lEvLog("ESP32: Next alarm in:   %lu mins %lu secs\n",diffTime_mins,diffTime_secs);
    } else {
        lEvLog("ESP32: Next alarm in:   %lu secs\n",diffTime_secs);
    }
    
    // AXP202 interrupt gpio_35
    // RTC interrupt gpio_37
    // touch panel interrupt gpio_38
    // bma interrupt gpio_397

    uint64_t newTime = LUNOKIOT_WAKE_TIME_S; // normal wake time
    if ( IsBLEEnabled() ) { newTime = LUNOKIOT_WAKE_TIME_NOTIFICATIONS_S; } // time for notifications
    esp_err_t wakeTimer = esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * newTime); // periodical wakeup to take samples
    if ( ESP_OK != wakeTimer ) { lSysLog("ERROR: Unable to set timer wakeup in %u seconds\n",newTime); }

    esp_err_t wakeAXP = esp_sleep_enable_ext0_wakeup((gpio_num_t)AXP202_INT, LOW);
    if ( ESP_OK != wakeAXP ) { lSysLog("ERROR: Unable to set ext0 wakeup\n"); }
    /*
    esp_err_t wakeBMA = esp_sleep_enable_ext0_wakeup((gpio_num_t)BMA423_INT1, HIGH);
    if ( ESP_OK != wakeBMA ) { lSysLog("ERROR: Unable to set ext0 wakeup\n"); }
    */
    // the only good ones :(
    esp_err_t wakeBMA = esp_sleep_enable_ext1_wakeup( GPIO_SEL_39, ESP_EXT1_WAKEUP_ANY_HIGH); 
    if ( ESP_OK != wakeBMA ) { lSysLog("ERROR: Unable to set ext1 (BMA) wakeup\n"); }

    //esp_err_t wakeAXP_RTC = esp_sleep_enable_ext1_wakeup( (GPIO_SEL_35 | GPIO_SEL_37) , ESP_EXT1_WAKEUP_ANY_HIGH);
    //esp_err_t wakeBMA_RTC = esp_sleep_enable_ext1_wakeup( (GPIO_SEL_39) , ESP_EXT1_WAKEUP_ANY_HIGH);
    //if ( ESP_OK != wakeBMA_RTC ) { lSysLog("ERROR: Unable to set ext1 (IMU+RTC) wakeup\n"); }

    /*
    esp_err_t uartEnabled=esp_sleep_enable_uart_wakeup(UART_NUM_0);
    if ( ESP_OK != uartEnabled ) {
        lEvLog("ESP32: WARNING: Unable to wake up from UART\n");
    }
    */

   /*
   // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html#_CPPv426esp_sleep_enable_bt_wakeupv
   esp_err_t esp_sleep_enable_bt_wakeup(void)
   */

    FreeSpace();
    lEvLog("ESP32: -- ZZz --\n");
    // stupid take to get clarified about "before sleep and after"
    if( pdTRUE == xSemaphoreTake( lLogAsBlockSemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS) )  {
        xSemaphoreGive( lLogAsBlockSemaphore );
    }
    uart_wait_tx_idle_polling(UART_NUM_0); // dontcare if fail

    int64_t beginSleepTime_us = esp_timer_get_time();
    esp_err_t canSleep = esp_light_sleep_start();    // device sleeps now
    if ( ESP_OK != canSleep ) {
        lEvLog("ESP32: cant sleep due BLE/WiFi isnt stopped? !!!!!!!\n");
    }
    int64_t wakeSleepTime_us = esp_timer_get_time();
    int32_t differenceSleepTime_msec = (wakeSleepTime_us-beginSleepTime_us)/1000;
    deviceSleepMSecs+=differenceSleepTime_msec;
    deviceUsageMSecs=(esp_timer_get_time()/1000)-deviceSleepMSecs;
    //float deviceUsageRatio=(deviceUsageMSecs/float(deviceSleepMSecs+deviceUsageMSecs))*100.0;
    deviceUsageRatio=(deviceUsageMSecs/float(esp_timer_get_time()/1000.0))*100.0;

    lEvLog("ESP32: -- Wake -- o_O' Slippin' time: (this nap: %d secs/total: %u secs) in use: %lu secs (usage ratio: %.2f%%%%)\n",
            differenceSleepTime_msec/1000,deviceSleepMSecs/1000,deviceUsageMSecs/1000,deviceUsageRatio);

    xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    stepCount = ttgo->bma->getCounter();
    xSemaphoreGive(I2cMutex);

    //uint32_t nowSteps = ttgo->bma->getCounter();
    //lLog("@TODO STEPS: %u for LOW BATTERY MODE\n", nowSteps);


    lEvLog("ESP32: DoSleep(%d) dies here!\n", doSleepThreads);
    systemSleep = false;
    xSemaphoreGive(DoSleepTaskSemaphore);
    WakeUpReason();
    vTaskDelete(NULL);
}

void DoSleep() {
    if (false == systemSleep) {
        systemSleep = true;
        BaseType_t intTaskOk = xTaskCreatePinnedToCore(DoSleepTask, "lSleepTask", LUNOKIOT_TASK_STACK_SIZE, NULL,tskIDLE_PRIORITY-3, NULL,1);
        if ( pdPASS != intTaskOk ) {
            lSysLog("ERROR: cannot launch DoSleep\n");
            systemSleep = false;
            if (false == ttgo->bl->isOn()) {                
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_WAKE, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
                FreeSpace();
                ScreenWake();
            }
        }
    }
}

const char *BMAMessages[] = {
    "Activity: Stationary",
    "Activity: Walking",
    "Activity: Running",
    "Activity: Invalid",
    "Activity: None"
};

const uint16_t BMAActivitesBufferMaxEntries=3;
uint8_t *BMAActivitesBuffer=nullptr;
RTC_Date *BMAActivitesTimes=nullptr;
size_t BMAActivitesOffset=0;

static void BMAEventActivity(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    if ( nullptr == BMAActivitesBuffer ) {
        BMAActivitesBuffer = (uint8_t*)ps_malloc(BMAActivitesBufferMaxEntries);
        BMAActivitesTimes=(RTC_Date*)ps_calloc(BMAActivitesBufferMaxEntries,sizeof(RTC_Date)); // mmmm, I'm not sure about this
        BMAActivitesOffset=0;
    }
    xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    const char *nowActivity = ttgo->bma->getActivity();
    xSemaphoreGive(I2cMutex);

    if (0 != strcmp(currentActivity, nowActivity)) {
        unsigned long endBMAActivity = millis();
        unsigned long totalBMAActivity = endBMAActivity - beginBMAActivity;
        uint32_t endStepsBMAActivity = stepCount;
        uint32_t totalStepsBMAActivity = endStepsBMAActivity - beginStepsBMAActivity;
        // time counter attribution
        if (0 == strcmp("BMA423_USER_STATIONARY", currentActivity)) {
            stepsBMAActivityStationary += totalStepsBMAActivity;
            timeBMAActivityStationary += totalBMAActivity;
        }
        else if (0 == strcmp("BMA423_USER_WALKING", currentActivity)) {
            stepsBMAActivityWalking += totalStepsBMAActivity;
            timeBMAActivityWalking += totalBMAActivity;
        }
        else if (0 == strcmp("BMA423_USER_RUNNING", currentActivity)) {
            stepsBMAActivityRunning += totalStepsBMAActivity;
            timeBMAActivityRunning += totalBMAActivity;
        }
        else if (0 == strcmp("BMA423_STATE_INVALID", currentActivity)) {
            stepsBMAActivityInvalid += totalStepsBMAActivity;
            timeBMAActivityInvalid += totalBMAActivity;
        }
        else if (0 == strcmp("None", currentActivity)) {
            stepsBMAActivityNone += totalStepsBMAActivity;
            timeBMAActivityNone += totalBMAActivity;
        }


        //@TODO use array to save this data, and push to sql when WAKE

        int activity=-1; // the offset of strings BMAMessages
        // current activity mark
        if (0 == strcmp("BMA423_USER_STATIONARY", nowActivity)) {
            activity=0;
            //SqlLog("Activity: Stationary");
        }
        else if (0 == strcmp("BMA423_USER_WALKING", nowActivity)) {
            activity=1;
            //SqlLog("Activity: Walking");
        }
        else if (0 == strcmp("BMA423_USER_RUNNING", nowActivity)) {
            activity=2;
            //SqlLog("Activity: Running");
        }
        else if (0 == strcmp("BMA423_STATE_INVALID", nowActivity)) {
            activity=3;
            //SqlLog("Activity: Invalid");
        }
        else if (0 == strcmp("None", nowActivity)) {
            activity=4;
            //SqlLog("Activity: None");
        }
        if ( -1 != activity ) {
            lSysLog("Activity: holding activity and data %d on offset: %u\n",activity,BMAActivitesOffset);
            BMAActivitesBuffer[BMAActivitesOffset]=activity;
            //RTC_Date r = ttgo->rtc->getDateTime();
            if (pdTRUE == xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS)) {
                BMAActivitesTimes[BMAActivitesOffset] = ttgo->rtc->getDateTime();
                xSemaphoreGive(I2cMutex);
            }
            //void * ptrShit=BMAActivitesTimes+(BMAActivitesOffset*sizeof(RTC_Date));
            //memcpy(ptrShit,&r,sizeof(RTC_Date));
            BMAActivitesOffset++;
            if ( BMAActivitesOffset >= BMAActivitesBufferMaxEntries ) {
                SystemBMARestitution();
                BMAActivitesOffset=0;
            }
        }
        FreeSpace();

        lEvLog("BMA423: Event: Last actity: %s\n", currentActivity);
        lEvLog("BMA423: Event: Current actity: %s\n", nowActivity);
        lEvLog("BMA423: Timers:\n");
        lEvLog("BMA423:        Stationary: %lu secs (%u steps)\n", timeBMAActivityStationary / 1000, stepsBMAActivityStationary);
        lEvLog("BMA423:           Walking: %lu secs (%u steps)\n", timeBMAActivityWalking / 1000, stepsBMAActivityWalking);
        lEvLog("BMA423:           Running: %lu secs (%u steps)\n", timeBMAActivityRunning / 1000, stepsBMAActivityRunning);
        lEvLog("BMA423:           Invalid: %lu secs (%u steps)\n", timeBMAActivityInvalid / 1000, stepsBMAActivityInvalid);
        lEvLog("BMA423:              None: %lu secs (%u steps)\n", timeBMAActivityNone / 1000, stepsBMAActivityNone);

        beginBMAActivity = millis();
        beginStepsBMAActivity = stepCount;
        currentActivity = nowActivity;
    }
    if (false == ttgo->bl->isOn()) { DoSleep(); }
}

Ticker TimedDoSleep;

static void SystemEventTimer(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    lEvLog("ESP Wakeup timer triggered\n");
    if (false == ttgo->bl->isOn()) { LoT().CpuSpeed(80); }

    TakeAllSamples();
    // if bluetooth is enabled, do a chance to get notifications
    if ( IsBLEEnabled() ) {
        //delay(100);
        //BLEKickAllPeers(); // force clients to reconnect
        lEvLog("BLE: Wait a little bit for connection/notification...\n");
        //delay(100); // let antena wharm
        TimedDoSleep.once(9,[]() { // get sleep in some seconds if no screen is on
            if (false == ttgo->bl->isOn()) {
                lEvLog("Timer triggered without screen... (going to sleep again)\n")
                DoSleep();
            }
        });
    } else { // no bluetooth, sleep now
        if (false == ttgo->bl->isOn()) {
            lEvLog("Timer triggered without screen... (going to sleep again)\n")
            DoSleep();
        }
    }
}

static void SystemEventPMUPower(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    if ( false == ttgo->bl->isOn()) { return; } // no screen?

    if ( vbusPresent ) { // is plugged?
        ttgo->setBrightness(255); // full light!!!
        LaunchApplication(new BatteryApplication());
    } else {
        uint8_t userBright = NVS.getInt("lBright"); // restore user light
        if ( userBright != 0 ) { ttgo->setBrightness(userBright); } // reset the user brightness
    }
}

static void AnyEventSystem(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    if (0 != strcmp(SYSTEM_EVENTS, base)) {
        lEvLog("WARNING! unknown base: '%s' args: %p id: %d data: %p\n", base, handler_args, id, event_data);
        return;
    }
    if (SYSTEM_EVENT_TICK == id) { return; }
    // tasks for any activity?
    lEvLog("@TODO Unhandheld SystemEvent: args: %p base: '%s' id: %d data: %p\n",handler_args,base,id,event_data);
}

static void BMAEventDoubleTap(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    lEvLog("BMA423: Event: Double tap\n");
    if (false == ttgo->bl->isOn()) {
        lEvLog("BMA423: Event: Double tap: Bring up system\n");
        ScreenWake();
        esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_WAKE, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        LaunchApplication(new WatchfaceAlwaysOn());
        //DoSleep();
    }
}

static void BMAEventTilt(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    lEvLog("BMA423: Event: Tilt\n");
    if (false == ttgo->bl->isOn()) { DoSleep(); }
}


static void BMAEventNoActivity(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    lEvLog("BMA423: Event: No actity\n");
    if (false == ttgo->bl->isOn()) { DoSleep(); }
}

static void BMAEventStepCounter(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    // Get step data from register
    uint32_t nowSteps = ttgo->bma->getCounter();
    if (nowSteps != (stepCount - lastBootStepCount))
    {
        stepCount = nowSteps + lastBootStepCount;
        lEvLog("BMA423: Event: Steps: %d\n", stepCount);
    }
    if (false == ttgo->bl->isOn()) { DoSleep(); }
}

extern bool provisioned;
extern void StopDatabase();

void SaveDataBeforeShutdown() {
    NVS.setInt("provisioned", provisioned, false);

    NVS.setInt("accXMax", accXMax, false);
    NVS.setInt("accXMin", accXMin, false);
    NVS.setInt("accYMax", accYMax, false);
    NVS.setInt("accYMin", accYMin, false);
    NVS.setInt("accZMax", accZMax, false);
    NVS.setInt("accZMin", accZMin, false);

    NVS.setInt("sBMAASta", stepsBMAActivityStationary, false);
    NVS.setInt("tBMAASta", (int64_t)timeBMAActivityStationary, false);
    NVS.setInt("sBMAAWalk", stepsBMAActivityWalking, false);
    NVS.setInt("tBMAAWalk", (int64_t)timeBMAActivityWalking, false);
    NVS.setInt("sBMAARun", stepsBMAActivityRunning, false);
    NVS.setInt("tBMAARun", (int64_t)timeBMAActivityRunning, false);
    NVS.setInt("sBMAAInvalid", stepsBMAActivityInvalid, false);
    NVS.setInt("tBMAAInvalid", (int64_t)timeBMAActivityInvalid, false);
    NVS.setInt("sBMAANone", stepsBMAActivityNone, false);
    NVS.setInt("tBMAANone", (int64_t)timeBMAActivityNone, false);
    // lLog("DEBUG stepCount: %d\n",stepCount);
    NVS.setInt("stepCount", stepCount, false);
    //delay(20);
    bool saved = NVS.commit();
    if (false == saved) {
        lLog("NVS: Unable to commit!! (data lost!?)\n");
    }
    char * usageStatics=(char *)ps_malloc(200);
    if ( nullptr != usageStatics ) {
        sprintf(usageStatics,"Usage stats: Sleep: %u secs, in use: %lu secs (usage ratio: %.2f%%%%)",
                deviceSleepMSecs/1000,deviceUsageMSecs/1000,deviceUsageRatio);
        SqlLog(usageStatics);
        delay(200);
    }
    //StopDatabase();
    //delay(100);

    //StopBLE();
    //delay(50);
    //delay(50);
    LoT().SPIFFSReady=false; // mark as unused
    //IsSPIFFSEnabled()
    SPIFFS.end();
    lEvLog("SPIFFS: Closed\n");

    NVS.close();
    lEvLog("NVS: Closed\n");
}

static void AXPEventPEKLong(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    if ( ttgo->bl->isOn() ) {
        LaunchApplication(new ShutdownApplication());
    }
    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_STOP, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
}
void _SendEventWakeTask(void *data) {
    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_WAKE, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    vTaskDelete(NULL);
}
static void AXPEventPEKShort(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {

    /*
    static unsigned long lastPEK = 0;
    if ( millis() > lastPEK ) {
        */
    if (ttgo->bl->isOn()) {
        lEvLog("Event: user wants to put device to sleep\n");
        FreeSpace();
        ScreenSleep();
        esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_STOP, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        DoSleep();
    }
    else {
        lEvLog("Event: user wants to get a screen\n");
        esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_WAKE, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        FreeSpace();
        ScreenWake();
    }
    /*
    lastPEK = millis()+50;
} else {
    Serial.println("Event: discarded PEK by: Too fast rule");
}
*/
}


void TakeBMPSample() {
    
    BaseType_t done = xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
    if (pdTRUE != done) { return; }
    
    Accel acc;
    // Get acceleration data
    bool res = ttgo->bma->getAccel(acc);
    float nowBMATemp = ttgo->bma->temperature();
    // Obtain the BMA423 direction,
    // so that the screen orientation is consistent with the sensor
    uint8_t nowRotation = ttgo->bma->direction();

    xSemaphoreGive(I2cMutex);
    
    if (res != false) {
        // @TODO simplify the values to round (send event only if change is relevant)
        if ((accX != acc.x) || (accY != acc.y) || (accZ != acc.z)) {
            accX = acc.x;
            accY = acc.y;
            accZ = acc.z;
            // set min/max values (to get percentage)
            if (accXMax < acc.x) { accXMax = acc.x; }
            else if (accXMin > acc.x) { accXMin = acc.x; }
            if (accYMax < acc.y) { accYMax = acc.y; }
            else if (accYMin > acc.y) { accYMin = acc.y; }
            if (accZMax < acc.z) { accZMax = acc.z; }
            else if (accZMin > acc.z) { accZMin = acc.z; }
            if (0 != (accXMax - accXMin)) {
                lpcX = pcX;
                pcX = ((acc.x - accXMin) * 100) / (accXMax - accXMin);
                degX = float(((acc.x - accXMin) * 360.0) / (accXMax - accXMin));
            }
            if (0 != (accYMax - accYMin))
            {
                lpcY = pcY;
                pcY = ((acc.y - accYMin) * 100) / (accYMax - accYMin);
                degY = float(((acc.y - accYMin) * 360.0) / (accYMax - accYMin));
            }
            if (0 != (accZMax - accZMin))
            {
                lpcZ = pcZ;
                pcZ = ((acc.z - accZMin) * 100) / (accZMax - accZMin);
                degZ = float(((acc.z - accZMin) * 360.0) / (accZMax - accZMin));
            }

            // Serial.printf("BMA423: Acceleromether: MaxX: %d MaxY: %d MaxZ: %d\n", accXMax, accYMax, accZMax);
            // Serial.printf("BMA423: Acceleromether: MinX: %d MinY: %d MinZ: %d\n", accXMin, accYMin, accZMin);
            //  Serial.printf("BMA423: Acceleromether: Xpc: %d%% Ypc: %d%% Zpc: %d%%\n",pcX,pcY,pcZ);

            // Serial.printf("BMA423: Acceleromether: X: %dº Y: %dº Z: %dº\n",degX,degY,degZ);

            // lEvLog("BMA423: Acceleromether: X: %d Y: %d Z: %d\n", acc.x, acc.y, acc.z);
            // esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMP_EVENT_MOVE,nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);

            #ifdef LUNOKIOT_SERIAL_GYRO_CSV
            // RAW
            Serial.printf("CSV:%f,%f,%f\n", degX, degY, degZ);

            // smoothed 2x
            static float ldegX = degX;
            static float ldegY = degY;
            static float ldegZ = degZ;
            // Serial.printf("CSV:%f,%f,%f\n",(degX+ldegX)/2.0,(degY+ldegY)/2.0,(degZ+ldegZ)/2.0);
            ldegX = degX;
            ldegY = degY;
            ldegZ = degZ;
            #endif
        }
    }

    if (round(nowBMATemp) != round(bmaTemp)) {
        // Serial.printf("BMA423: Temperature: %.1fºC\n", bmaTemp);
        if (UIRunning) {
            esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_TEMP, nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
        }
    }
    bmaTemp = nowBMATemp; // set value

    if (nowRotation != bmaRotation) {
        // Serial.printf("BMA423: Direction: Current: %u Obtained: %u Last: %u\n", ttgo->tft->getRotation(), rotation, prevRotation);
        bmaRotation = nowRotation;
        if (UIRunning) {
            esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_DIRECTION, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
        }
    }
}

unsigned long nextSlowSensorsTick = 0;
unsigned long nextSensorsTick = 0;
static void SystemEventTick(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    if (millis() > nextSlowSensorsTick) { // poll some sensors ( slow pooling )
        TakeAllSamples();
        nextSlowSensorsTick = millis() + 2666;
    } else if (millis() > nextSensorsTick) { // poll some other sensors ( normal pooling )
        TakeBMPSample();
        nextSensorsTick = millis() + (1000 / 8);
    }
}

static void SystemEventStop(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    SqlLog("stop");
    lSysLog("System event: Stop\n");
}
unsigned long lastLowMemTimestamp_ms=0; // time limit for the next LowMemory()
static void SystemEventLowMem(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    // dont trigger too fast
    if ( lastLowMemTimestamp_ms > millis() ) { return; }
    lastLowMemTimestamp_ms=millis()+2000;
    // dump mem status
    const char lineTxt[]="------------------------\n";
    lSysLog(lineTxt);
    lSysLog("System event: Low memory\n");
    lSysLog(lineTxt);
    FreeSpace();
    lSysLog(lineTxt);
    // try to warn foreground app about the problem
    if( xSemaphoreTake( UISemaphore, portMAX_DELAY) == pdTRUE )  {
    if ( nullptr != currentApplication ) {
            currentApplication->LowMemory();
        }
        if ( nullptr == currentApplication->canvas ) {
            lSysLog("System event: Application don't have enough memory to run, returning to user selected Watchface\n");
            xSemaphoreGive( UISemaphore ); // free
            //@TODO this is an excelent place to launch the BSOD screen :P
            lLog("@TODO show BSOD here!\n");
            LaunchWatchface();
            return;
        }
        //lSysLog("System event: Application memory constrained\n");
    }
    xSemaphoreGive( UISemaphore ); // free
}
void SystemBMARestitution() {
    const char fmtStr[]="INSERT INTO rawlog VALUES (NULL,'%02u-%02u-%04u %02u:%02u:%02u','%s');";
    if (BMAActivitesOffset > 0 ) {
        for(int c=0;c<BMAActivitesOffset;c++) {
            uint8_t currAct=BMAActivitesBuffer[c];
            RTC_Date r=BMAActivitesTimes[c];
            lEvLog("BMA: Pending activity (%d): %02u:%02u:%02u date: %02u-%02u-%04u ",c,r.hour,r.minute,r.second,r.day,r.month,r.year); 
            lLog("%u '%s' dump to SQLite\n",c,BMAMessages[currAct]);
            size_t totalsz = strlen(fmtStr)+40;
            char * query=(char*)ps_malloc(totalsz);
            if ( nullptr == query ) {
                lEvLog("BMA: ERROR: Pending activity (%d) unable to build SQL query!!!\n",c);
                continue;
            }
            sprintf(query,fmtStr,r.day,r.month,r.year,r.hour,r.minute,r.second,BMAMessages[currAct]);
            if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
                int  rc = db_exec(lIoTsystemDatabase,query);
                if (rc != SQLITE_OK) {
                    lSysLog("SQL: ERROR: Unable exec: '%s'\n", query);
                }
                xSemaphoreGive( SqlLogSemaphore ); // free
            }
            free(query);

//BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_QUERY_STACK_SIZE,  query, uxTaskPriorityGet(NULL), NULL,1);
//if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }

        }
        BMAActivitesOffset=0;
    }
}

static void SystemEventWake(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    lSysLog("System event: Wake\n");
    SqlLog("wake");

    TakeAllSamples(); // get current pose
    
    //lSysLog("GESTURE? %f,%f,%f\n", degX, degY, degZ);

    // enable lamp? @TODO THIS MUST BE A SETTING
    bool launchLamp=false;
    //lLog("@TODO DISABLED LAMP LAUNCH <=========================\n");
    if ( degY > 340.0 ) { launchLamp = true; }
    else if ( degY < 20.0 ) { launchLamp = true; }

    if ( launchLamp ) { // is the correct pose to get light?
        LaunchApplication(new LampApplication());
        return;
    } else if ( nullptr == currentApplication ) {
        LaunchWatchface(false);
    } else { // already have an app on currentApplication
        if ( false == currentApplication->isWatchface() ) { // Isn't a watchface, run it now!
            LaunchWatchface(false);
        }
    }
    // In case of already running a watchface, do nothing

    // check the BMA ACTIVITIES during sleep
    SystemBMARestitution();
}
// called when system is up
static void SystemEventReady(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    lSysLog("System event: Up and running!\n");
    FreeSpace();
    if ( ( NVS.getInt("WifiEnabled") ) && ( false == provisioned ) ) {
        // wifi is enabled but no provisioned? Launch the prov to suggest the user to do it
        LaunchApplication(new Provisioning2Application());
        return;
    }
    if ( nullptr == currentApplication ) { LaunchWatchface(false); }
}

static void FreeRTOSEventReceived(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    // https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-guides/event-handling.html#event-ids-and-corresponding-data-structures
    bool identified = false;
    if (WIFI_PROV_EVENT == base) {
        if (WIFI_PROV_INIT == id) {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Initialized\n", base);
            identified = true;
        } else if (WIFI_PROV_START == id) {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Started\n", base);
            identified = true;
        } else if (WIFI_PROV_CRED_RECV == id) {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Received Wi-Fi credentials SSID: '%s' PASSWORD: '%s'\n", base, (const char *)wifi_sta_cfg->ssid, (const char *)wifi_sta_cfg->password);
            identified = true;
        } else if (WIFI_PROV_CRED_FAIL == id) {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Bad credentials (WPA password)\n", base);
            identified = true;
        } else if (WIFI_PROV_CRED_SUCCESS == id) {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Credentials success\n", base);
            identified = true;
        } else if (WIFI_PROV_END == id) {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: End\n", base);
            identified = true;
        } else if (WIFI_PROV_DEINIT == id) {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Deinit\n", base);
            identified = true;
        }
    }
    else if (WIFI_EVENT == base) {
        if (WIFI_EVENT_SCAN_DONE == id) {
            wifi_event_sta_scan_done_t *scanData = (wifi_event_sta_scan_done_t *)event_data;
            const char *scanDoneStr = (scanData->status ? "failed" : "done");
            lNetLog("FreeRTOS event:'%s' WiFi Scan %d %s (found: %d)\n", base, scanData->scan_id, scanDoneStr, scanData->number);
            identified = true;
        } else if (WIFI_EVENT_STA_START == id) {
            lNetLog("FreeRTOS event:'%s' WiFi STA: Start\n", base);
            identified = true;
        } else if (WIFI_EVENT_STA_STOP == id) {
            lNetLog("FreeRTOS event:'%s' WiFi STA: Stop\n", base);
            identified = true;
        } else if (WIFI_EVENT_STA_CONNECTED == id) {
            wifi_event_sta_connected_t *connectData = (wifi_event_sta_connected_t *)event_data;
            char ssidstr[34];
            snprintf(ssidstr,connectData->ssid_len+1,"%s",connectData->ssid);
            const char * AuthMode = "UNKNOWN";
            if ( WIFI_AUTH_OPEN == connectData->authmode ) { AuthMode = "Open"; }
            else if ( WIFI_AUTH_WEP == connectData->authmode ) { AuthMode = "WEP"; }
            else if ( WIFI_AUTH_WPA_PSK == connectData->authmode ) { AuthMode = "WPA PSK"; }
            else if ( WIFI_AUTH_WPA2_PSK == connectData->authmode ) { AuthMode = "WPA2 PSK"; }
            else if ( WIFI_AUTH_WPA_WPA2_PSK == connectData->authmode ) { AuthMode = "WPA WPA2 PSK"; }
            else if ( WIFI_AUTH_WPA2_ENTERPRISE == connectData->authmode ) { AuthMode = "WPA2 enterprise"; }
            else if ( WIFI_AUTH_WPA3_PSK == connectData->authmode ) { AuthMode = "WPA3 PSK"; }
            else if ( WIFI_AUTH_WPA2_WPA3_PSK == connectData->authmode ) { AuthMode = "WPA2 WPA3 PSK"; }

            lNetLog("FreeRTOS event:'%s' WiFi STA: Connected SSID: '%s' Channel: %d Auth: '%s'\n", base,ssidstr,connectData->channel,AuthMode);
            identified = true;
        } else if (WIFI_EVENT_STA_DISCONNECTED == id) {
            wifi_event_sta_disconnected_t *disconnectData = (wifi_event_sta_disconnected_t *)event_data;
            const char * ReasonText = "UNKNOWN";
            if ( WIFI_REASON_UNSPECIFIED == disconnectData->reason ) {
                ReasonText = "Unspecified";
            } else if ( WIFI_REASON_AUTH_EXPIRE == disconnectData->reason ) {
                ReasonText = "Auth expire";
            } else if ( WIFI_REASON_AUTH_LEAVE == disconnectData->reason ) {
                ReasonText = "Auth leave";
            } else if ( WIFI_REASON_ASSOC_EXPIRE == disconnectData->reason ) {
                ReasonText = "Assoc expire";
                WiFi.disconnect();
                delay(100);
                WiFi.begin();
            } else if ( WIFI_REASON_ASSOC_TOOMANY == disconnectData->reason ) {
                ReasonText = "Assoc too many";
            } else if ( WIFI_REASON_NOT_AUTHED == disconnectData->reason ) {
                ReasonText = "Not authed";
            } else if ( WIFI_REASON_NOT_ASSOCED == disconnectData->reason ) {
                ReasonText = "Not assoced";
            } else if ( WIFI_REASON_ASSOC_LEAVE == disconnectData->reason ) {
                ReasonText = "Assoc leave";
            } else if ( WIFI_REASON_ASSOC_NOT_AUTHED == disconnectData->reason ) {
                ReasonText = "Assoc not authed";
            } else if ( WIFI_REASON_DISASSOC_PWRCAP_BAD == disconnectData->reason ) {
                ReasonText = "Disassoc pwrcap bad";
            } else if ( WIFI_REASON_DISASSOC_SUPCHAN_BAD == disconnectData->reason ) {
                ReasonText = "Disassoc subchan bad";
            } else if ( WIFI_REASON_IE_INVALID == disconnectData->reason ) {
                ReasonText = "IE invalid";
            } else if ( WIFI_REASON_MIC_FAILURE == disconnectData->reason ) {
                ReasonText = "Mic failure";
            } else if ( WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT == disconnectData->reason ) {
                ReasonText = "4Way handshake timeout";
                WiFi.disconnect();
                delay(100);
                WiFi.begin();
            } else if ( WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT == disconnectData->reason ) {
                ReasonText = "Group key update timeout";
            } else if ( WIFI_REASON_IE_IN_4WAY_DIFFERS == disconnectData->reason ) {
                ReasonText = "IE in 4way differs";
            } else if ( WIFI_REASON_GROUP_CIPHER_INVALID == disconnectData->reason ) {
                ReasonText = "Group cipher invalid";
            } else if ( WIFI_REASON_PAIRWISE_CIPHER_INVALID == disconnectData->reason ) {
                ReasonText = "Group pairwise cipher invalid";
            } else if ( WIFI_REASON_AKMP_INVALID == disconnectData->reason ) {
                ReasonText = "AKMP invalid";
            } else if ( WIFI_REASON_UNSUPP_RSN_IE_VERSION == disconnectData->reason ) {
                ReasonText = "Unsupported RSN IE version";
            } else if ( WIFI_REASON_INVALID_RSN_IE_CAP == disconnectData->reason ) {
                ReasonText = "Invalid RSN IE cap";
            } else if ( WIFI_REASON_802_1X_AUTH_FAILED == disconnectData->reason ) {
                ReasonText = "802_1x auth failed";
            } else if ( WIFI_REASON_CIPHER_SUITE_REJECTED == disconnectData->reason ) {
                ReasonText = "Cipher suite rejected";
            } else if ( WIFI_REASON_INVALID_PMKID == disconnectData->reason ) {
                ReasonText = "Invalid PMKID";
            } else if ( WIFI_REASON_BEACON_TIMEOUT == disconnectData->reason ) {
                ReasonText = "Beacon timeout";
            } else if ( WIFI_REASON_NO_AP_FOUND == disconnectData->reason ) {
                ReasonText = "No AP found";
            } else if ( WIFI_REASON_AUTH_FAIL == disconnectData->reason ) {
                ReasonText = "Auth fail";
            } else if ( WIFI_REASON_ASSOC_FAIL == disconnectData->reason ) {
                ReasonText = "Assoc fail";
            } else if ( WIFI_REASON_HANDSHAKE_TIMEOUT == disconnectData->reason ) {
                ReasonText = "Handshake timeout";
                WiFi.disconnect();
                delay(100);
                WiFi.begin();
            } else if ( WIFI_REASON_CONNECTION_FAIL == disconnectData->reason ) {
                ReasonText = "Connection fail";
            } else if ( WIFI_REASON_AP_TSF_RESET == disconnectData->reason ) {
                ReasonText = "AP TSF reset";
            }
            lNetLog("FreeRTOS event:'%s' WiFi STA: Disconnected Reason: '%s'\n", base,ReasonText);

            //lSysLog("FreeRTOS event: Forced poweroff WiFi after disconnect\n");
            //WiFi.mode(WIFI_OFF); 
            identified = true;
        } else if (WIFI_EVENT_AP_START == id) {
            lNetLog("FreeRTOS event:'%s' WiFi AP: Start\n", base);
            identified = true;
        } else if (WIFI_EVENT_AP_STOP == id) {
            lNetLog("FreeRTOS event:'%s' WiFi AP: Stop\n", base);
            identified = true;
        } else if (WIFI_EVENT_AP_STACONNECTED == id) {
            wifi_event_ap_staconnected_t *apStaData = (wifi_event_ap_staconnected_t *)event_data;
            lNetLog("FreeRTOS event:'%s' WiFi AP: Client %02x:%02x:%02x:%02x:%02x:%02x connected aid: %d\n", base, apStaData->mac[0], apStaData->mac[1], apStaData->mac[2], apStaData->mac[3], apStaData->mac[4], apStaData->mac[5],apStaData->aid);
            identified = true;
        } else if (WIFI_EVENT_AP_STADISCONNECTED == id) {
            wifi_event_ap_stadisconnected_t *apStaData = (wifi_event_ap_stadisconnected_t *)event_data;
            lNetLog("FreeRTOS event:'%s' WiFi AP: Client %02x:%02x:%02x:%02x:%02x:%02x disconnected\n", base, apStaData->mac[0], apStaData->mac[1], apStaData->mac[2], apStaData->mac[3], apStaData->mac[4], apStaData->mac[5]);
            identified = true;
        }
    } else if (IP_EVENT == base) {
        if (IP_EVENT_STA_GOT_IP == id) {
            lNetLog("FreeRTOS event:'%s' Network: IP Obtained\n", base);
            identified = true;
        } else if (IP_EVENT_STA_LOST_IP == id) {
            lNetLog("FreeRTOS event:'%s' Network: IP Lost\n", base);
            identified = true;
        } else if (IP_EVENT_AP_STAIPASSIGNED == id) {
            ip_event_ap_staipassigned_t *assignedIP = (ip_event_ap_staipassigned_t *)event_data;
            esp_ip4_addr_t newIP = assignedIP->ip;
            unsigned char octet[4] = {0, 0, 0, 0};
            for (int i = 0; i < 4; i++) {
                octet[i] = (newIP.addr >> (i * 8)) & 0xFF;
            }
            lNetLog("FreeRTOS event:'%s' Network: STA IP Assigned: %d.%d.%d.%d\n", base, octet[0], octet[1], octet[2], octet[3]);
            identified = true;
        }
    }

    if (false == identified) {
        lLog("@TODO lunokIoT: unamanged FreeRTOS event:'%s' id: '%d' \n", base, id);
    }
}

TaskHandle_t AXPInterruptControllerHandle = NULL;
static void AXPInterruptController(void *args) {
    lSysLog("AXP interrupt handler \n");
    // ADC monitoring must be enabled to use the AXP202 monitoring function
    ttgo->power->adc1Enable(
        AXP202_VBUS_VOL_ADC1 |
            AXP202_VBUS_CUR_ADC1 |
            AXP202_BATT_CUR_ADC1 |
            AXP202_BATT_VOL_ADC1,
        true);
    ttgo->powerAttachInterrupt([](){ irqAxp = true; });

    ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_PEK_LONGPRESS_IRQ |
                               AXP202_BATT_LOW_TEMP_IRQ | AXP202_BATT_OVER_TEMP_IRQ |
                               AXP202_BATT_REMOVED_IRQ | AXP202_BATT_CONNECT_IRQ |
                               AXP202_BATT_ACTIVATE_IRQ | AXP202_BATT_EXIT_ACTIVATE_IRQ |
                               AXP202_CHARGING_IRQ | AXP202_CHARGING_FINISHED_IRQ,
                           true);
    ttgo->power->clearIRQ();


    TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
    while(true) {   
        BaseType_t isDelayed = xTaskDelayUntil( &nextCheck, LUNOKIOT_SYSTEM_INTERRUPTS_TIME ); // wait a ittle bit
        BaseType_t done = xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_FAST_TIME_TICKS);
        if (pdTRUE != done) { continue; }

        if (irqAxp) {
            lEvLog("AXP202: INT received\n");
            int readed = ttgo->power->readIRQ();
            if ( AXP_PASS != readed ) { xSemaphoreGive(I2cMutex); continue; }
            if (ttgo->power->isChargingIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery charging\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_CHARGING, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->power->isChargingDoneIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery fully charged\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_FULL, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->power->isBattEnterActivateIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery active\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_ACTIVE, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->power->isBattExitActivateIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery free\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_FREE, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->power->isBattPlugInIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery present\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_PRESENT, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->power->isBattRemoveIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery removed\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_REMOVED, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->power->isBattTempLowIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery temperature low\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_TEMP_LOW, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
                continue;
            } else if (ttgo->power->isBattTempHighIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery temperature high\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_TEMP_HIGH, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
                continue;
            } else if (ttgo->power->isVbusPlugInIRQ()) {
                ttgo->power->clearIRQ();
                vbusPresent = true;
                lEvLog("AXP202: Power source\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_POWER, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->power->isVbusRemoveIRQ()) {
                ttgo->power->clearIRQ();
                vbusPresent = false;
                lEvLog("AXP202: No power\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_NOPOWER, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->power->isPEKShortPressIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Event PEK Button short press\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_SHORT, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
                continue;
            } else if (ttgo->power->isPEKLongtPressIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Event PEK Button long press\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_LONG, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
                continue;
            } else if (ttgo->power->isTimerTimeoutIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Event Timer timeout\n");
                irqAxp = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_TIMER_TIMEOUT, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
                continue;
            }
            ttgo->power->clearIRQ(); // <= if this is enabled, the AXP turns dizzy until next event
            lSysLog("@TODO unknown unprocessed interrupt call from AXP202!\n");
        }
        irqAxp = false;
        xSemaphoreGive(I2cMutex);
    }
    lEvLog("AXPInterruptController is dead!\n");
    vTaskDelete(NULL);
}

TaskHandle_t BMAInterruptControllerHandle = NULL;
static void BMAInterruptController(void *args) {
    lSysLog("BMA interrupt handler\n"); // BMA423

    lEvLog("NVS: Loading last session values...\n");

    int32_t tempStepCount = NVS.getInt("stepCount");

    int16_t tempAccXMax = NVS.getInt("accXMax");
    int16_t tempAccXMin = NVS.getInt("accXMin");
    int16_t tempAccYMax = NVS.getInt("accYMax");
    int16_t tempAccYMin = NVS.getInt("accYMin");
    int16_t tempAccZMax = NVS.getInt("accZMax");
    int16_t tempAccZMin = NVS.getInt("accZMin");

    uint32_t tempStepsBMAActivityStationary = NVS.getInt("sBMAASta");
    unsigned long tempTimeBMAActivityStationary = NVS.getInt("tBMAASta");
    if (false != tempStepsBMAActivityStationary) { stepsBMAActivityStationary = tempStepsBMAActivityStationary; }
    if (false != tempTimeBMAActivityStationary) { timeBMAActivityStationary = tempTimeBMAActivityStationary; }
    // lLog("BMA423: NVS: Last session stepsBMAActivityStationary = %d\n",stepsBMAActivityStationary);
    // lLog("BMA423: NVS: Last session timeBMAActivityStationary = %lu\n",timeBMAActivityStationary);
    uint32_t tempStepsBMAActivityWalking = NVS.getInt("sBMAAWalk");
    unsigned long tempTimeBMAActivityWalking = NVS.getInt("tBMAAWalk");
    if (false != tempStepsBMAActivityWalking) { stepsBMAActivityWalking = tempStepsBMAActivityWalking; }
    if (false != tempTimeBMAActivityWalking) { timeBMAActivityWalking = tempTimeBMAActivityWalking; }
    // lLog("BMA423: NVS: Last session stepsBMAActivityWalking = %d\n",stepsBMAActivityWalking);
    // lLog("BMA423: NVS: Last session timeBMAActivityWalking = %lu\n",timeBMAActivityWalking);
    uint32_t tempStepsBMAActivityRunning = NVS.getInt("sBMAARun");
    unsigned long tempTimeBMAActivityRunning = NVS.getInt("tBMAARun");
    if (false != tempStepsBMAActivityRunning) { stepsBMAActivityRunning = tempStepsBMAActivityRunning; }
    if (false != tempTimeBMAActivityRunning) { timeBMAActivityRunning = tempTimeBMAActivityRunning; }
    // lLog("BMA423: NVS: Last session stepsBMAActivityRunning = %d\n",stepsBMAActivityRunning);
    // lLog("BMA423: NVS: Last session timeBMAActivityRunning = %lu\n",timeBMAActivityRunning);
    uint32_t tempStepsBMAActivityNone = NVS.getInt("sBMAANone");
    unsigned long tempTimeBMAActivityNone = NVS.getInt("tBMAANone");
    if (false != tempStepsBMAActivityNone) { stepsBMAActivityNone = tempStepsBMAActivityNone; }
    if (false != tempTimeBMAActivityNone) { timeBMAActivityNone = tempTimeBMAActivityNone; }
    // lLog("BMA423: NVS: Last session stepsBMAActivityNone = %d\n",stepsBMAActivityNone);
    // lLog("BMA423: NVS: Last session timeBMAActivityNone = %lu\n",timeBMAActivityNone);

    // lLog("BMA423: NVS: Last session tempStepCount: %d\n",tempStepCount);

    if (0 != tempStepCount) {
        lastBootStepCount = tempStepCount;
        stepCount = lastBootStepCount;
        lEvLog("BMA423: NVS: Last session stepCount: %d\n", stepCount);
    }

    if (false != tempAccXMax) { accXMax = tempAccXMax; }
    if (false != tempAccXMin) { accXMin = tempAccXMin; }
    if (false != tempAccYMax) { accYMax = tempAccYMax; }
    if (false != tempAccYMin) { accYMin = tempAccYMin; }
    if (false != tempAccZMax) { accZMax = tempAccZMax; }
    if (false != tempAccZMin) { accZMin = tempAccZMin; }

    // lLog("BMA423: NVS: Last session stepCount = %d\n",tempStepCount);

    // lLog("BMA423: NVS: X(%d/%d) ",accXMax,accXMin);
    // lLog("Y(%d/%d) ",accYMax,accYMin);
    // lLog("Z(%d/%d)\n",accZMax,accZMin);
    //  Accel parameter structure
    Acfg cfg;
    /*!
        Output data rate in Hz, Optional parameters:
            - BMA4_OUTPUT_DATA_RATE_0_78HZ
            - BMA4_OUTPUT_DATA_RATE_1_56HZ
            - BMA4_OUTPUT_DATA_RATE_3_12HZ
            - BMA4_OUTPUT_DATA_RATE_6_25HZ
            - BMA4_OUTPUT_DATA_RATE_12_5HZ
            - BMA4_OUTPUT_DATA_RATE_25HZ
            - BMA4_OUTPUT_DATA_RATE_50HZ
            - BMA4_OUTPUT_DATA_RATE_100HZ
            - BMA4_OUTPUT_DATA_RATE_200HZ
            - BMA4_OUTPUT_DATA_RATE_400HZ
            - BMA4_OUTPUT_DATA_RATE_800HZ
            - BMA4_OUTPUT_DATA_RATE_1600HZ
    */
    // cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    //cfg.odr = BMA4_OUTPUT_DATA_RATE_50HZ;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_25HZ;
    /*!
        G-range, Optional parameters:
            - BMA4_ACCEL_RANGE_2G
            - BMA4_ACCEL_RANGE_4G
            - BMA4_ACCEL_RANGE_8G
            - BMA4_ACCEL_RANGE_16G
    */
    // cfg.range = BMA4_ACCEL_RANGE_8G;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    /*!
        Bandwidth parameter, determines filter configuration, Optional parameters:
            - BMA4_ACCEL_OSR4_AVG1
            - BMA4_ACCEL_OSR2_AVG2
            - BMA4_ACCEL_NORMAL_AVG4
            - BMA4_ACCEL_CIC_AVG8
            - BMA4_ACCEL_RES_AVG16
            - BMA4_ACCEL_RES_AVG32
            - BMA4_ACCEL_RES_AVG64
            - BMA4_ACCEL_RES_AVG128
    */
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    // cfg.bandwidth = BMA4_ACCEL_RES_AVG128;

    /*! Filter performance mode , Optional parameters:
        - BMA4_CIC_AVG_MODE
        - BMA4_CONTINUOUS_MODE
    */
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;

    BaseType_t done = xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    if (pdTRUE != done) {
        lSysLog("BMA: ERROR: Unable to get i2c mutex!\n");
        return;
    }

    // Configure the BMA423 accelerometer
    bool isConfigured = ttgo->bma->accelConfig(cfg);
    if ( true != isConfigured ) {
        lSysLog("ERROR: UNABLE TO SET BMA CONFIG\n");
        xSemaphoreGive(I2cMutex);
        return;
    }
    // Enable BMA423 accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    bool isAccelEnable = ttgo->bma->enableAccel();
    if ( true != isAccelEnable ) {
        lSysLog("ERROR: UNABLE TO ENABLE BMA ACCELERATION\n");
        xSemaphoreGive(I2cMutex);
        return;
    }
    ttgo->bma423AttachInterrupt([](){ irqBMA = true; });

    bool featureOK = ttgo->bma->enableFeature(BMA423_STEP_CNTR, true);
    if ( false == featureOK ) { lEvLog("BMA: Enable feature 'Step counter' failed\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }
    featureOK = ttgo->bma->enableFeature(BMA423_ANY_MOTION, true);
    if ( false == featureOK ) { lEvLog("BMA: Enable feature 'any motion' failed\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }
    featureOK = ttgo->bma->enableFeature(BMA423_NO_MOTION, true);
    if ( false == featureOK ) { lEvLog("BMA: Enable feature 'no motion' failed\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }
    featureOK = ttgo->bma->enableFeature(BMA423_ACTIVITY, true);
    if ( false == featureOK ) { lEvLog("BMA: Enable feature 'activity' failed\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }
    //featureOK = ttgo->bma->enableFeature(BMA423_TILT, true);
    //if ( false == featureOK ) { lEvLog("BMA: Enable feature 'tilt' failed\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }
    featureOK = ttgo->bma->enableFeature(BMA423_WAKEUP, true);
    if ( false == featureOK ) { lEvLog("BMA: Enable feature 'double tap' failed\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }
    // Reset steps
    featureOK = ttgo->bma->resetStepCounter();
    if ( false == featureOK ) { lEvLog("BMA: Unable to reset stepCounter\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }
    // Isnt needed, the stepcount ocurrs internally on BMA
    //featureOK = ttgo->bma->enableStepCountInterrupt();
    //if ( false == featureOK ) { lEvLog("BMA: Unable to Enable interrupt 'step count'\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }

    featureOK = ttgo->bma->enableActivityInterrupt();
    if ( false == featureOK ) { lEvLog("BMA: Unable to Enable interrupt 'activity'\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }
    featureOK = ttgo->bma->enableAnyNoMotionInterrupt();
    if ( false == featureOK ) { lEvLog("BMA: Unable to Enable interrupt 'no motion'\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }
    featureOK = ttgo->bma->enableTiltInterrupt();
    if ( false == featureOK ) { lEvLog("BMA: Unable to Enable interrupt 'tilt'\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }
    featureOK = ttgo->bma->enableWakeupInterrupt();
    if ( false == featureOK ) { lEvLog("BMA: Unable to Enable interrupt 'double tap'\n"); xSemaphoreGive(I2cMutex); vTaskDelete(NULL); }

    xSemaphoreGive(I2cMutex);

    TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
    while(true) {
        //delay(LUNOKIOT_SYSTEM_INTERRUPTS_TIME);
        BaseType_t isDelayed = xTaskDelayUntil( &nextCheck, LUNOKIOT_SYSTEM_INTERRUPTS_TIME ); // wait a ittle bit
        // check the BMA int's
        BaseType_t done = xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_FAST_TIME_TICKS);
        if (pdTRUE != done) { continue; }

        if (irqBMA) {
            lEvLog("BMA423: INT received\n");
            int readed = ttgo->bma->readInterrupt();
            if ( false == readed ) { xSemaphoreGive(I2cMutex); continue; } // wait a bit
           /*
            bool rlst;
            do {
                esp_task_wdt_reset();
                delay(3);
                // Read the BMA423 interrupt status,
                // need to wait for it to return to true before continuing
                rlst = ttgo->bma->readInterrupt();
            } while (!rlst);
            */
            //delay(10);
            // Check if it is a step interrupt
            if (ttgo->bma->isStepCounter()) {
                irqBMA = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_STEPCOUNTER, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->bma->isTilt()) {
                irqBMA = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_TILT, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->bma->isActivity()) {
                irqBMA = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_ACTIVITY, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->bma->isAnyNoMotion()) {
                irqBMA = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_NOMOTION, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            } else if (ttgo->bma->isDoubleClick()) {
                irqBMA = false;
                xSemaphoreGive(I2cMutex);
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_DOUBLE_TAP, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                continue;
            }
            
            irqBMA = false;
            lLog("@TODO unknown unprocessed interrupt call from BMA423!\n");
        }
        xSemaphoreGive(I2cMutex);
    }
    //lEvLog("BMAInterruptController is dead!\n");
    vTaskDelete(NULL);
}






TaskHandle_t RTCInterruptControllerHandle = NULL;
static void RTCInterruptController(void *args) {
    lSysLog("RTC interrupt handler \n");
    xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    ttgo->rtcAttachInterrupt([](){irqRTC = true;});
    
    if ( ttgo->rtc->alarmActive() ) { 
        //ttgo->rtc->disableAlarm();
        lEvLog("RTC: Alarm is active\n");
    }
    if ( ttgo->rtc->isTimerEnable() ) {
        //ttgo->rtc->disableTimer();
        lEvLog("RTC: Timer is active\n");
    }
    
    //ttgo->rtc->resetAlarm();
    ttgo->rtc->disableAlarm();

    ttgo->rtc->setAlarmByMinutes(1);
    ttgo->rtc->enableAlarm();
    lLog("PCF8563: TEST @@@@@@@@@@@@@@@@@@@@@@@@@@DEBUG alarm set by 1 minute\n");

/*
    ttgo->rtc->disableTimer();
    ttgo->rtc->setTimer(5,20,true);
    ttgo->rtc->enableTimer();
    //lLog("PCF8563: @DEBUG timer set by 5 times\n");
    */
    xSemaphoreGive(I2cMutex);
    TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
    while(true) {   
        BaseType_t isDelayed = xTaskDelayUntil( &nextCheck, LUNOKIOT_SYSTEM_INTERRUPTS_TIME ); // wait a ittle bit

        BaseType_t done = xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_FAST_TIME_TICKS);
        if (pdTRUE != done) { continue; }
        // check the RTC int
        if (irqRTC) {
            irqRTC = false;
            lEvLog("PCF8563: INT received\n");
            if ( ttgo->rtc->alarmActive() ) { 
                //ttgo->rtc->resetAlarm();
                ttgo->rtc->disableAlarm();
                lEvLog("RTC: Alarm is active\n");
            }
            if ( ttgo->rtc->isTimerEnable() ) {
                lEvLog("RTC: Timer is active\n");
                ttgo->rtc->disableTimer();
            }


            /**
            ttgo->rtc->setAlarmByMinutes(1);
            ttgo->rtc->enableAlarm();
            lLog("PCF8563: TEST AGAIN ---------------------> alarm set by 1 minute\n");
            */
            xSemaphoreGive(I2cMutex);
            continue;

            //detachInterrupt(RTC_INT_PIN);
            //detachInterrupt(RTC_INT_PIN);
            bool isAlarm = ttgo->rtc->alarmActive();
            lEvLog("PCF8563: Event: Alarm active: %s\n",(isAlarm?"true":"false"));
            if ( true == isAlarm ) {
                ttgo->rtc->resetAlarm();
                //ttgo->rtc->resetAlarm(); */
                lEvLog("PCF8563: Event: Alarm\n");
                //ttgo->rtc->disableAlarm();
                //ttgo->rtc->setDateTime(2019, 8, 12, 15, 0, 53);
                //ttgo->rtc->setAlarmByMinutes(3);
                //ttgo->rtc->enableAlarm();
                //lLog("@TODO @DEBUG RE-ENABLE ALARM... wakeup BUG pending...");
                ttgo->rtc->disableAlarm();
                //ttgo->rtc->setAlarmByMinutes(1);
                //ttgo->rtc->enableAlarm();
                //lLog("@@@@@@@@@@@@@@@ AGAIN alarm set by 1 minute\n");
            }
            bool isTimer = ttgo->rtc->isTimerActive();
            bool isTimerEnabled = ttgo->rtc->isTimerEnable();
            lEvLog("PCF8563: Event: Timer active: %s enabled: %s\n",(isTimer?"true":"false"),(isTimerEnabled?"true":"false"));
            if ( true == isTimer ) {
                ttgo->rtc->clearTimer();
                lEvLog("PCF8563: Event: Timer\n");
                ttgo->rtc->disableTimer();
                ttgo->rtc->setTimer(10,5,true);
                ttgo->rtc->enableTimer();
                lLog("@@@@@@@@@@@@@@@ AGAIN set timer\n");
            }

        }
        xSemaphoreGive(I2cMutex);        
    }
    lEvLog("RTCInterruptController is dead!\n");
    vTaskDelete(NULL);
}



/*
#include <esp_expression_with_stack.h>
void external_stack_function(void)
{
    printf("Executing this printf from external stack! \n");
}
*/
/**
 * @brief callback called when a allocation operation fails, if registered
 * @param size in bytes of failed allocation
 * @param caps capabillites requested of failed allocation
 * @param function_name function which generated the failure
 */
void SystemAllocFailed(size_t size, uint32_t caps, const char * function_name) {
    //lLog("\n");
    //lLog(" ====== /!\\/!\\/!\\ ======\n");
    lLog("\n");
    lLog("ESP32: ERROR: Unable to allocate %d Bytes requested by: '%s' (flags: 0x%x)\n",size,function_name,caps);
    lLog("\n");
    /*
    //Allocate a stack buffer, from heap or as a static form:
    portSTACK_TYPE *shared_stack = (portSTACK_TYPE *)ps_malloc(LUNOKIOT_TASK_STACK_SIZE * sizeof(portSTACK_TYPE));
    if ( NULL == shared_stack ) {
        lSysLog("Unable to allocate space to call LowMemory()\n");
        return;
    }

    //Allocate a mutex to protect its usage:
    SemaphoreHandle_t printf_lock = xSemaphoreCreateMutex();
    assert(printf_lock != NULL);

    //Call the desired function using the macro helper:
    esp_execute_shared_stack_function(printf_lock,
                                    shared_stack,
                                    8192,
                                    external_stack_function);

    vSemaphoreDelete(printf_lock);
    free(shared_stack);
    */
   
    //lLog("@TODO show BSOD here 2!\n");

   //@TODO this is an excelent place to launch the BSOD screen :P
    esp_err_t sended = esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_LOWMEMORY, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    if ( ESP_OK != sended ) {
        lLog("@TODO show BSOD here 2!\n");
    }
}

void SystemShutdownHandler() {
    lLog("======>> lunokIoT: See you soon! :) <<======\n");
    uart_wait_tx_idle_polling(UART_NUM_0);
}

void SystemEventsStart() {
    systemStatsBootCounter++; // increment boot count
    esp_err_t allocatorRegistered = heap_caps_register_failed_alloc_callback(&SystemAllocFailed);
    if ( ESP_OK != allocatorRegistered ) {
        lSysLog("WARNING: Unable to register memory allocator\n");
    }
    esp_err_t shutdownRegistered = esp_register_shutdown_handler(&SystemShutdownHandler); 
    if ( ESP_OK != shutdownRegistered ) {
        lSysLog("WARNING: Unable to register shutdown handler\n");
    }
    //delay(50);

    // Create my own event loop
    lSysLog("System event loop\n");
    // configure system event loop (send and receive messages from other parts of code)
    esp_event_loop_args_t lunokIoTSystemEventloopConfig = {
        .queue_size = 8,                             // maybe so big?
        .task_name = "lEvTask",                      // lunokIoT Event Task
        .task_priority = tskIDLE_PRIORITY+8,         // a little bit faster?
        .task_stack_size = LUNOKIOT_TASK_STACK_SIZE, // don't need so much
        .task_core_id = 1                            // to core 1
    };                                               // details: https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-reference/system/esp_event.html#_CPPv421esp_event_loop_args_t
    //delay(50);

    esp_err_t loopCreated = esp_event_loop_create(&lunokIoTSystemEventloopConfig, &systemEventloopHandler);
    if ( ESP_FAIL == loopCreated ) {
        lSysLog("SystemEventsStart: ERROR: Unable to create LunokIoT Event Queue (fail)\n");
    } else if ( ESP_ERR_NO_MEM == loopCreated ) {
        lSysLog("SystemEventsStart: ERROR: Unable to create LunokIoT Event Queue (no mem)\n");
    } else if ( ESP_ERR_INVALID_ARG == loopCreated ) {
        lSysLog("SystemEventsStart: ERROR: Unable to create LunokIoT Event Queue (invalid args)\n");
    }
    //delay(50);

    // get the freeRTOS event loop
    lSysLog("freeRTOS event loop\n");
    esp_err_t espLoopCreated = esp_event_loop_create_default();
    if ( ESP_FAIL == espLoopCreated ) {
        lSysLog("SystemEventsStart: ERROR: Unable to create ESP32 Event Queue (fail)\n");
    } else if ( ESP_ERR_NO_MEM == espLoopCreated ) {
        lSysLog("SystemEventsStart: ERROR: Unable to create ESP32 Event Queue (no mem)\n");
    } else if ( ESP_ERR_INVALID_ARG == espLoopCreated ) {
        lSysLog("SystemEventsStart: ERROR: Unable to create ESP32 Event Queue (invalid args)\n");
    }
    // register all crap to event default
    esp_err_t registered = esp_event_handler_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, FreeRTOSEventReceived, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on ESP32 queue ANY_BASE ANY_ID\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_LOWMEMORY, SystemEventLowMem, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: LOW MEMORY\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_WAKE, SystemEventWake, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: WAKE\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_STOP, SystemEventStop, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: STOP\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_READY, SystemEventReady, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: READY\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_TICK, SystemEventTick, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: TICK\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_SHORT, AXPEventPEKShort, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: PEK short\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_LONG, AXPEventPEKLong, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: PEK long\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_STEPCOUNTER, BMAEventStepCounter, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: Step counter\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_ACTIVITY, BMAEventActivity, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: Activity\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_NOMOTION, BMAEventNoActivity, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: No motion\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_TIMER, SystemEventTimer, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: TIMER\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_POWER, SystemEventPMUPower, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: POWER\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_NOPOWER, SystemEventPMUPower, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: NO POWER\n");
    }

    // tilt and doubletap
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_TILT, BMAEventTilt, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: Tilt\n");
    }
    registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_DOUBLE_TAP, BMAEventDoubleTap, nullptr, NULL);
    if ( ESP_OK != registered ) {
        lSysLog("SystemEventsStart: ERROR: Unable register on lunokIoT queue Event: Double tap\n");
    }


    lSysLog("PMU interrupts\n");
    // Start the AXP interrupt controller loop
    BaseType_t intTaskOk = xTaskCreatePinnedToCore(AXPInterruptController, "intAXP", LUNOKIOT_TINY_STACK_SIZE, nullptr, tskIDLE_PRIORITY+5, &AXPInterruptControllerHandle,0);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch AXP int handler!\n"); }
    //delay(150);

    lSysLog("IMU interrupts\n");
    // Start the BMA interrupt controller loop
    intTaskOk = xTaskCreatePinnedToCore(BMAInterruptController, "intBMA", LUNOKIOT_TINY_STACK_SIZE, nullptr, tskIDLE_PRIORITY+5, &BMAInterruptControllerHandle,0);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch BMA int handler!\n"); }

    lSysLog("RTC interrupts @DEBUG @TODO DISABLED\n");
    // Start the RTC interrupt controller loop
    //intTaskOk = xTaskCreatePinnedToCore(RTCInterruptController, "intRTC", LUNOKIOT_TINY_STACK_SIZE, nullptr, tskIDLE_PRIORITY+5, &RTCInterruptControllerHandle,0);
    //if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch RTC int handler!\n"); }

    LunokIoTSystemTickerStart();
    //delay(50);
}

/*
 * Notification of boot end (only one time per boot)
 */
void SystemEventBootEnd() {
    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_READY, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
}

void TakeAXPSample() {
    BaseType_t done = xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
    if (pdTRUE != done) { return; }
    // begin tick report
    float nowAXPTemp = ttgo->power->getTemp();
    vbusPresent = ttgo->power->isVBUSPlug();
    bool battConnected = ttgo->power->isBatteryConnect();
    int nowBatteryPercent = ttgo->power->getBattPercentage();

    xSemaphoreGive(I2cMutex);

    if (nowAXPTemp != axpTemp) {
        axpTemp = nowAXPTemp;
        // lEvLog("AXP202: Temperature: %.1fºC\n", axpTemp);
        esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_TEMPERATURE, nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
    }

    // basic Battery info
    if (battConnected) {
        if (nowBatteryPercent != batteryPercent) {
            batteryPercent = nowBatteryPercent;
            esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_PC, nullptr, 0, LUNOKIOT_EVENT_FAST_TIME_TICKS);
        }
    } else {
        if (-1 != batteryPercent) {
            batteryPercent = -1;
            esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_NOTFOUND, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
        }
        batteryPercent = -1;
    }
}

void TakeESPSamples() {
    hallData = hallRead();
    float nowCpuTemp = (temprature_sens_read() - 32) / 1.8;

    if (nowCpuTemp != cpuTemp) {
        cpuTemp = nowCpuTemp;
        esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_TEMP, nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
    }
}
/*
 * Get all samples possible
 */
unsigned long nextReportTimestamp = 0;
void TakeAllSamples() {

    TakeESPSamples();
    TakeBMPSample();
    TakeAXPSample();

    // periodical report
    if( millis() > nextReportTimestamp ) {
        char logMsg[255] = { 0 }; 
        sprintf(logMsg,"Temperatures: CPU: %.2f C, PMU: %.1f C, IMU: %g C", cpuTemp,axpTemp,bmaTemp);
        SqlLog(logMsg);
        sprintf(logMsg,"Hall: %d", hallData);
        SqlLog(logMsg);
        sprintf(logMsg,"Acceleromether: X: %d, Y: %d, Z: %d", accX,accY,accZ);
        SqlLog(logMsg);
        sprintf(logMsg,"Battery: %d, USB: %s", batteryPercent, (vbusPresent?"yes":"no"));
        SqlLog(logMsg);
        nextReportTimestamp=millis()+(10*60*1000);
    }

}
