#include <Arduino.h>
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo library shit ;)
#include <ArduinoNvs.h>

#include "SystemEvents.hpp"

#include "lunokiot_config.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_event.h>
#include <esp_event_base.h>
#include <WiFi.h>

#include <esp_wifi.h>
#include <wifi_provisioning/manager.h>

#include <esp_task_wdt.h>

#include "UI/UI.hpp"

#include "../app/Shutdown.hpp"
#include "../app/Provisioning2.hpp"

#include "../app/Watchface.hpp"

#include <HTTPClient.h>

#include "../app/LogView.hpp"

#include <cmath>
extern bool UIRunning;
bool systemSleep = false;
// Event loops
esp_event_loop_handle_t systemEventloopHandler;
ESP_EVENT_DEFINE_BASE(SYSTEM_EVENTS);

bool provisioned = false;
int hallData = 0;         // hall sensor data
bool vbusPresent = false; // USB connected?
int batteryPercent;       // -1 if no batt
uint8_t bmaRotation;      // bma ground?
float axpTemp;
float bmaTemp;
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

bool irqAxp = false;
bool irqBMA = false;
bool irqRTC = false;

extern NimBLECharacteristic *battCharacteristic;
/*
void RTCIntHandler() {
    pinMode(RTC_INT_PIN, INPUT_PULLDOWN);
    attachInterrupt(RTC_INT_PIN, [] {
        irqRTC = true;
    }, FALLING);

    bool rtcValid = ttgo->rtc->isValid();
    Serial.printf("PCF8563: Valid: %s\n",(rtcValid?"true":"false"));

    ttgo->rtc->disableAlarm();
    Serial.println("PCF8563: @TODO incomplete code");
    return;
    ttgo->rtc->setDateTime(2019, 8, 12, 15, 0, 53);
    ttgo->rtc->setAlarmByMinutes(1);
    ttgo->rtc->enableAlarm();
    Serial.println("PCF8563: @DEBUG alarm set by 1 minute");

}
*/
void BMPIntHandler()
{
    lEvLog("BMA423: NVS: Loading last session values...\n");

    int32_t tempStepCount = NVS.getInt("stepCount");

    int16_t tempAccXMax = NVS.getInt("accXMax");
    int16_t tempAccXMin = NVS.getInt("accXMin");
    int16_t tempAccYMax = NVS.getInt("accYMax");
    int16_t tempAccYMin = NVS.getInt("accYMin");
    int16_t tempAccZMax = NVS.getInt("accZMax");
    int16_t tempAccZMin = NVS.getInt("accZMin");

    uint32_t tempStepsBMAActivityStationary = NVS.getInt("sBMAASta");
    unsigned long tempTimeBMAActivityStationary = NVS.getInt("tBMAASta");
    if (false != tempStepsBMAActivityStationary)
    {
        stepsBMAActivityStationary = tempStepsBMAActivityStationary;
    }
    if (false != tempTimeBMAActivityStationary)
    {
        timeBMAActivityStationary = tempTimeBMAActivityStationary;
    }
    // lLog("BMA423: NVS: Last session stepsBMAActivityStationary = %d\n",stepsBMAActivityStationary);
    // lLog("BMA423: NVS: Last session timeBMAActivityStationary = %lu\n",timeBMAActivityStationary);
    uint32_t tempStepsBMAActivityWalking = NVS.getInt("sBMAAWalk");
    unsigned long tempTimeBMAActivityWalking = NVS.getInt("tBMAAWalk");
    if (false != tempStepsBMAActivityWalking)
    {
        stepsBMAActivityWalking = tempStepsBMAActivityWalking;
    }
    if (false != tempTimeBMAActivityWalking)
    {
        timeBMAActivityWalking = tempTimeBMAActivityWalking;
    }
    // lLog("BMA423: NVS: Last session stepsBMAActivityWalking = %d\n",stepsBMAActivityWalking);
    // lLog("BMA423: NVS: Last session timeBMAActivityWalking = %lu\n",timeBMAActivityWalking);
    uint32_t tempStepsBMAActivityRunning = NVS.getInt("sBMAARun");
    unsigned long tempTimeBMAActivityRunning = NVS.getInt("tBMAARun");
    if (false != tempStepsBMAActivityRunning)
    {
        stepsBMAActivityRunning = tempStepsBMAActivityRunning;
    }
    if (false != tempTimeBMAActivityRunning)
    {
        timeBMAActivityRunning = tempTimeBMAActivityRunning;
    }
    // lLog("BMA423: NVS: Last session stepsBMAActivityRunning = %d\n",stepsBMAActivityRunning);
    // lLog("BMA423: NVS: Last session timeBMAActivityRunning = %lu\n",timeBMAActivityRunning);
    uint32_t tempStepsBMAActivityNone = NVS.getInt("sBMAANone");
    unsigned long tempTimeBMAActivityNone = NVS.getInt("tBMAANone");
    if (false != tempStepsBMAActivityNone)
    {
        stepsBMAActivityNone = tempStepsBMAActivityNone;
    }
    if (false != tempTimeBMAActivityNone)
    {
        timeBMAActivityNone = tempTimeBMAActivityNone;
    }
    // lLog("BMA423: NVS: Last session stepsBMAActivityNone = %d\n",stepsBMAActivityNone);
    // lLog("BMA423: NVS: Last session timeBMAActivityNone = %lu\n",timeBMAActivityNone);

    // lLog("BMA423: NVS: Last session tempStepCount: %d\n",tempStepCount);

    if (0 != tempStepCount)
    {
        lastBootStepCount = tempStepCount;
        stepCount = lastBootStepCount;
        lEvLog("BMA423: NVS: Last session stepCount: %d\n", stepCount);
    }

    if (false != tempAccXMax)
    {
        accXMax = tempAccXMax;
    }
    if (false != tempAccXMin)
    {
        accXMin = tempAccXMin;
    }
    if (false != tempAccYMax)
    {
        accYMax = tempAccYMax;
    }
    if (false != tempAccYMin)
    {
        accYMin = tempAccYMin;
    }
    if (false != tempAccZMax)
    {
        accZMax = tempAccZMax;
    }
    if (false != tempAccZMin)
    {
        accZMin = tempAccZMin;
    }

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
    cfg.odr = BMA4_OUTPUT_DATA_RATE_50HZ;
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

    // Configure the BMA423 accelerometer
    ttgo->bma->accelConfig(cfg);
    // Enable BMA423 accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    ttgo->bma->enableAccel();

    pinMode(BMA423_INT1, INPUT);
    attachInterrupt(
        BMA423_INT1, []
        {
        // Set interrupt to set irq value to 1
        irqBMA = true; },
        RISING); // It must be a rising edge

    ttgo->bma->enableFeature(BMA423_STEP_CNTR, true);
    ttgo->bma->enableFeature(BMA423_ANY_MOTION, true);
    ttgo->bma->enableFeature(BMA423_NO_MOTION, true);
    ttgo->bma->enableFeature(BMA423_ACTIVITY, true);

    // Reset steps
    ttgo->bma->resetStepCounter();

    ttgo->bma->enableStepCountInterrupt();
    ttgo->bma->enableActivityInterrupt();
    ttgo->bma->enableAnyNoMotionInterrupt();
}

void AXPIntHandler()
{

    // ADC monitoring must be enabled to use the AXP202 monitoring function
    ttgo->power->adc1Enable(
        AXP202_VBUS_VOL_ADC1 |
            AXP202_VBUS_CUR_ADC1 |
            AXP202_BATT_CUR_ADC1 |
            AXP202_BATT_VOL_ADC1,
        true);

    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(
        AXP202_INT, []
        { irqAxp = true; },
        FALLING);

    ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_PEK_LONGPRESS_IRQ |
                               AXP202_BATT_LOW_TEMP_IRQ | AXP202_BATT_OVER_TEMP_IRQ |
                               AXP202_BATT_REMOVED_IRQ | AXP202_BATT_CONNECT_IRQ |
                               AXP202_BATT_ACTIVATE_IRQ | AXP202_BATT_EXIT_ACTIVATE_IRQ |
                               AXP202_CHARGING_IRQ | AXP202_CHARGING_FINISHED_IRQ,
                           true);
    ttgo->power->clearIRQ();
}

void WakeUpReason()
{ // this function decides the system must wake or sleep

// if call return, the system remains active but with the screen off (must call DoSleep in other place)
//
// at this time only 3 interrupts returns:
//
// esp timer int
// AXP202 int
// BMA432 int

    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();
    TakeSamples();
    lEvLog("ESP32 Wake up from: ");
    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        lEvLog("Interrupt triggered on ext0 <-- (PMU) AXP202\n");
        return;
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
    {
        lEvLog("Interrupt triggered on ext1 <-- ");
        uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
        if (GPIO_SEL_37 == GPIO_reason)
        {
            lEvLog("(RTC) PCF8563\n");
        }
        else if (GPIO_SEL_38 == GPIO_reason)
        {
            lEvLog("(TOUCH) FocalTech\n");
        }
        else if (GPIO_SEL_39 == GPIO_reason)
        {
            lEvLog("(BMA) BMA423\n");
            //return;
        }
        else
        {
            // Serial.printf("unexpected (?) GPIO %d reason: %u\n", impliedGPIO, GPIO_reason);
            lLog("WARNING: Unexpected: GPIO\n");
            // lLog((log(GPIO_reason))/log(2), 0);
        }
        /*
        uint64_t impliedGPIO = (log(GPIO_reason))/log(2);
        if ( 37 == impliedGPIO) { Serial.println("(RTC) PCF8563"); }
        else if ( 38 == impliedGPIO) { Serial.println("(TOUCH) FocalTech"); }
        else if ( 39 == impliedGPIO) { Serial.println("(ACCEL) BMA423"); }
        else {
            Serial.printf("unexpected (?) GPIO %d reason: %u\n", impliedGPIO, GPIO_reason);
            Serial.print("GPIO that triggered the wake up: GPIO ");
            Serial.println((log(GPIO_reason))/log(2), 0);
        }
        */
    }
    break;
    case ESP_SLEEP_WAKEUP_TIMER:
        lEvLog("Wakeup caused by timer\n");
        esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_TIMER, nullptr, 0, LUNOKIOT_EVENT_FAST_TIME_TICKS);
        return;
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        lEvLog("Wakeup caused by touchpad\n");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        lEvLog("Wakeup caused by ULP program\n");
        break;
    default:
        lEvLog("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
    DoSleep();
}

uint16_t doSleepThreads = 0;
SemaphoreHandle_t DoSleepTaskSemaphore = xSemaphoreCreateMutex();

static void DoSleepTask(void *args)
{
    /*
    if ( networkActivity ) {
        Serial.println("ESP32: DoSleep DISCARDED due network activity");
        vTaskDelete(NULL);
    }*/
    lEvLog("ESP32: DoSleep Trying to obtain the lock...\n");
    bool done = xSemaphoreTake(DoSleepTaskSemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
    if (false == done)
    {
        lEvLog("ESP32: DoSleep DISCARDED: already in progress...\n");
        vTaskDelete(NULL);
    }

    doSleepThreads++;
    lLog("ESP32: Free heap: %d KB\n", ESP.getFreeHeap() / 1024);
    lLog("ESP32: Free PSRAM: %d KB\n", ESP.getFreePsram() / 1024);

    lEvLog("ESP32: DoSleep(%d) began!\n", doSleepThreads);

    ScreenSleep();

    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_LIGHTSLEEP, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);

    // Serial.printf("WIFI STATUS: %d\n", WiFi.status());
    const size_t MAXRETRIES = 3;
    size_t retries = MAXRETRIES;
    // Serial.printf("WIFISTATUS: %d\n",WiFi.status());
    // while ( ( WL_NO_SHIELD != WiFi.status())&&( WL_IDLE_STATUS != WiFi.status() ) ) {
    while (WL_CONNECTED == WiFi.status())
    {
        lNetLog("ESP32: DoSleep WAITING (%d): WiFi in use, waiting for system radio powerdown....\n", retries);
        delay(2000);
        retries--;
        if (0 == retries + 1)
        {
            lNetLog("ESP32: System don't end the connection. Forcing WiFI off due timeout\n");
            WiFi.disconnect(true);
            delay(200); // get driver time to archieve ordered disconnect (say AP goodbye etc...)
            break;
        }
    }
    WiFi.mode(WIFI_OFF);
    if (retries < MAXRETRIES)
    {
        lEvLog("ESP32: WiFi released, DoSleep continue...\n");
    }
    // AXP202 interrupt gpio_35
    // RTC interrupt gpio_37
    // touch panel interrupt gpio_38
    // bma interrupt gpio_39
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * LUNOKIOT_WAKE_TIME_S);
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_39, ESP_EXT1_WAKEUP_ANY_HIGH);
    lEvLog("ESP32: -- ZZz --\n");
    Serial.flush();
    delay(100);
    esp_light_sleep_start();            // device sleeps now
    lEvLog("ESP32: -- Wake -- o_O'\n"); // good morning!!
    systemSleep = false;

    lEvLog("ESP32: DoSleep(%d) dies here!\n", doSleepThreads);
    xSemaphoreGive(DoSleepTaskSemaphore);
    WakeUpReason();
    vTaskDelete(NULL);
}

void DoSleep()
{
    if (false == systemSleep)
    {
        systemSleep = true;
        xTaskCreate(DoSleepTask, "SLEEP", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);
    }
}

static void BMAEventActivity(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    const char *nowActivity = ttgo->bma->getActivity();
    if (0 != strcmp(currentActivity, nowActivity))
    {
        unsigned long endBMAActivity = millis();
        unsigned long totalBMAActivity = endBMAActivity - beginBMAActivity;
        uint32_t endStepsBMAActivity = stepCount;
        uint32_t totalStepsBMAActivity = endStepsBMAActivity - beginStepsBMAActivity;
        // time counter attribution
        if (0 == strcmp("BMA423_USER_STATIONARY", currentActivity))
        {
            stepsBMAActivityStationary += totalStepsBMAActivity;
            timeBMAActivityStationary += totalBMAActivity;
        }
        else if (0 == strcmp("BMA423_USER_WALKING", currentActivity))
        {
            stepsBMAActivityWalking += totalStepsBMAActivity;
            timeBMAActivityWalking += totalBMAActivity;
        }
        else if (0 == strcmp("BMA423_USER_RUNNING", currentActivity))
        {
            stepsBMAActivityRunning += totalStepsBMAActivity;
            timeBMAActivityRunning += totalBMAActivity;
        }
        else if (0 == strcmp("BMA423_STATE_INVALID", currentActivity))
        {
            stepsBMAActivityInvalid += totalStepsBMAActivity;
            timeBMAActivityInvalid += totalBMAActivity;
        }
        else if (0 == strcmp("None", currentActivity))
        {
            stepsBMAActivityNone += totalStepsBMAActivity;
            timeBMAActivityNone += totalBMAActivity;
        }
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
static void SystemEventTimer(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    TakeSamples();
    if (false == ttgo->bl->isOn()) { DoSleep(); }
}

static void SystemEventPMUPower(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    if ( false == ttgo->bl->isOn()) { return; } // no screen?

    if ( vbusPresent ) { // is plugged?
        ttgo->setBrightness(255); // full light!!!
    } else {
        uint8_t userBright = NVS.getInt("lBright"); // restore user light
        if ( userBright != 0 ) { ttgo->setBrightness(userBright); } // reset the user brightness
    }
}

static void AnyEventSystem(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    if (0 != strcmp(SYSTEM_EVENTS, base))
    {
        lLog("WARNING! unkown base: '%s' args: %p id: %d data: %p\n", base, handler_args, id, event_data);
        return;
    }
    if (SYSTEM_EVENT_TICK == id)
    {
        return;
    }
    // tasks for any activity?
    //lEvLog("@TODO Unhandheld SystemEvent: args: %p base: '%s' id: %d data: %p\n",handler_args,base,id,event_data);
}

static void BMAEventNoActivity(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    lEvLog("BMA423: Event: No actity\n");
    if (false == ttgo->bl->isOn()) { DoSleep(); }
}

static void BMAEventStepCounter(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    // Get step data from register
    uint32_t nowSteps = ttgo->bma->getCounter();
    if (nowSteps != (stepCount - lastBootStepCount))
    {
        stepCount = nowSteps + lastBootStepCount;
        lEvLog("BMA423: Event: Steps: %d\n", stepCount);
    }
    if (false == ttgo->bl->isOn()) { DoSleep(); }
}

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
    delay(50);
    bool saved = NVS.commit();
    if (false == saved)
    {
        lLog("NVS: Unable to commit!! (data lost!?)\n");
    }
    NVS.close();
    lEvLog("NVS: Closed\n");
    Serial.flush();
}

static void AXPEventPEKLong(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    LaunchApplication(new ShutdownApplication());
    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_STOP, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
}
void _SendEventWakeTask(void *data)
{
    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_WAKE, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    vTaskDelete(NULL);
}
static void AXPEventPEKShort(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{

    /*
    static unsigned long lastPEK = 0;
    if ( millis() > lastPEK ) {
        */
    if (ttgo->bl->isOn())
    {
        lEvLog("Event: user wants to put device to sleep\n");
        ScreenSleep();
        LaunchApplication(nullptr);
        DoSleep();
    }
    else
    {
        lEvLog("Event: user wants to get a screen\n");
        ScreenWake();
        xTaskCreate(_SendEventWakeTask, "", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);
    }
    /*
    lastPEK = millis()+50;
} else {
    Serial.println("Event: discarded PEK by: Too fast rule");
}
*/
}

void TakeBMPSample()
{
    Accel acc;
    // Get acceleration data
    bool res = ttgo->bma->getAccel(acc);

    if (res != false)
    {
        // @TODO simplify the values to round (send event only if change is relevant)
        if ((accX != acc.x) || (accY != acc.y) || (accZ != acc.z))
        {
            accX = acc.x;
            accY = acc.y;
            accZ = acc.z;
            // set min/max values (to get percentage)
            if (accXMax < acc.x)
            {
                accXMax = acc.x;
            }
            else if (accXMin > acc.x)
            {
                accXMin = acc.x;
            }
            if (accYMax < acc.y)
            {
                accYMax = acc.y;
            }
            else if (accYMin > acc.y)
            {
                accYMin = acc.y;
            }
            if (accZMax < acc.z)
            {
                accZMax = acc.z;
            }
            else if (accZMin > acc.z)
            {
                accZMin = acc.z;
            }
            if (0 != (accXMax - accXMin))
            {
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
    float nowBMATemp = ttgo->bma->temperature();
    if (round(nowBMATemp) != round(bmaTemp))
    {
        // Serial.printf("BMA423: Temperature: %.1fºC\n", bmaTemp);
        if (UIRunning) {
            esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_TEMP, nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
        }
    }
    bmaTemp = nowBMATemp; // set value

    // Obtain the BMA423 direction,
    // so that the screen orientation is consistent with the sensor
    uint8_t nowRotation = ttgo->bma->direction();
    if (nowRotation != bmaRotation)
    {
        // Serial.printf("BMA423: Direction: Current: %u Obtained: %u Last: %u\n", ttgo->tft->getRotation(), rotation, prevRotation);
        bmaRotation = nowRotation;
        /*
        switch (rotation) {
        case DIRECTION_DISP_DOWN:
            //No use
            break;
        case DIRECTION_DISP_UP:
            //No use
            break;
        case DIRECTION_BOTTOM_EDGE:
            Serial.printf(" set %u\n", WATCH_SCREEN_BOTTOM_EDGE);
            break;
        case DIRECTION_TOP_EDGE:
            Serial.printf(" set %u\n", WATCH_SCREEN_TOP_EDGE);
            break;
        case DIRECTION_RIGHT_EDGE:
            Serial.printf(" set %u\n", WATCH_SCREEN_RIGHT_EDGE);
            break;
        case DIRECTION_LEFT_EDGE:
            Serial.printf(" set %u\n", WATCH_SCREEN_LEFT_EDGE);
            break;
        default:
            break;
        }
        */
        if (UIRunning) {
            esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_DIRECTION, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
        }
    }
}

unsigned long nextSlowSensorsTick = 0;
unsigned long nextSensorsTick = 0;
static void SystemEventTick(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{

    if (millis() > nextSlowSensorsTick)
    { // poll some sensors ( slow pooling )
        hallData = hallRead();

        // @TODO obtain more AXP202 info
        vbusPresent = ttgo->power->isVBUSPlug();
        // basic Battery info
        if (ttgo->power->isBatteryConnect())
        {
            int nowBatteryPercent = ttgo->power->getBattPercentage();
            if (nowBatteryPercent != batteryPercent)
            {
                batteryPercent = nowBatteryPercent;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_PC, nullptr, 0, LUNOKIOT_EVENT_FAST_TIME_TICKS);
            }
        }
        else
        {
            if (-1 != batteryPercent)
            {
                batteryPercent = -1;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_NOTFOUND, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            }
            batteryPercent = -1;
        }

        // begin tick report
        float nowAXPTemp = ttgo->power->getTemp();
        if (nowAXPTemp != axpTemp)
        {
            axpTemp = nowAXPTemp;
            // lEvLog("AXP202: Temperature: %.1fºC\n", axpTemp);
            esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_TEMPERATURE, nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
        }
        nextSlowSensorsTick = millis() + 3000;
    }

    if (millis() > nextSensorsTick)
    { // poll some other sensors ( normal pooling )
        TakeSamples();
        nextSensorsTick = millis() + (1000 / 8);
    }
}

static void SystemEventReady(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{

    lEvLog("lunokIoT: System event: up and running...\n");
    /*
    #ifdef LUNOKIOT_WIFI_ENABLED
        if ( provisioned ) {
            Serial.println("lunokIoT: Device provisioned: Launching 'WatchfaceApplication'...");
            LaunchApplication(new WatchfaceApplication());
            return;
        }
        Serial.println("lunokIoT: Device not provisioned: Launching 'Provisioning2Application'...");
        LaunchApplication(new Provisioning2Application());
    #else
    */
    // lLog("lunokIoT: Launching 'WatchfaceApplication'...\n");
    LaunchApplication(new WatchfaceApplication(), false);
    //#endif
}

static void FreeRTOSEventReceived(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    // https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-guides/event-handling.html#event-ids-and-corresponding-data-structures
    bool identified = false;
    if (WIFI_PROV_EVENT == base)
    {
        if (WIFI_PROV_INIT == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Initialized\n", base);
            identified = true;
        }
        else if (WIFI_PROV_START == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Started\n", base);
            identified = true;
        }
        else if (WIFI_PROV_CRED_RECV == id)
        {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Received Wi-Fi credentials SSID: '%s' PASSWORD: '%s'\n", base, (const char *)wifi_sta_cfg->ssid, (const char *)wifi_sta_cfg->password);
            identified = true;
        }
        else if (WIFI_PROV_CRED_FAIL == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Bad credentials (WPA password)\n", base);
            identified = true;
        }
        else if (WIFI_PROV_CRED_SUCCESS == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Credentials success\n", base);
            identified = true;
        }
        else if (WIFI_PROV_END == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: End\n", base);
            identified = true;
        }
        else if (WIFI_PROV_DEINIT == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi provisioning: Deinit\n", base);
            identified = true;
        }
    }
    else if (WIFI_EVENT == base)
    {
        if (WIFI_EVENT_SCAN_DONE == id)
        {
            wifi_event_sta_scan_done_t *scanData = (wifi_event_sta_scan_done_t *)event_data;
            const char *scanDoneStr = (scanData->status ? "failed" : "done");
            lNetLog("FreeRTOS event:'%s' WiFi Scan %d %s (found: %d)\n", base, scanData->scan_id, scanDoneStr, scanData->number);
            identified = true;
        }
        else if (WIFI_EVENT_STA_START == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi STA: Start\n", base);
            identified = true;
        }
        else if (WIFI_EVENT_STA_STOP == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi STA: Stop\n", base);
            identified = true;
        }
        else if (WIFI_EVENT_STA_CONNECTED == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi STA: Connected\n", base);
            identified = true;
        }
        else if (WIFI_EVENT_STA_DISCONNECTED == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi STA: Disconnected\n", base);
            identified = true;
        }
        else if (WIFI_EVENT_AP_START == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi AP: Start\n", base);
            identified = true;
        }
        else if (WIFI_EVENT_AP_STOP == id)
        {
            lNetLog("FreeRTOS event:'%s' WiFi AP: Stop\n", base);
            identified = true;
        }
        else if (WIFI_EVENT_AP_STACONNECTED == id)
        {
            wifi_event_ap_staconnected_t *apStaData = (wifi_event_ap_staconnected_t *)event_data;
            lNetLog("FreeRTOS event:'%s' WiFi AP: Client %02x:%02x:%02x:%02x:%02x:%02x connected\n", base, apStaData->mac[0], apStaData->mac[1], apStaData->mac[2], apStaData->mac[3], apStaData->mac[4], apStaData->mac[5]);
            identified = true;
        }
        else if (WIFI_EVENT_AP_STADISCONNECTED == id)
        {
            wifi_event_ap_stadisconnected_t *apStaData = (wifi_event_ap_stadisconnected_t *)event_data;
            lNetLog("FreeRTOS event:'%s' WiFi AP: Client %02x:%02x:%02x:%02x:%02x:%02x disconnected\n", base, apStaData->mac[0], apStaData->mac[1], apStaData->mac[2], apStaData->mac[3], apStaData->mac[4], apStaData->mac[5]);
            identified = true;
        }
    }
    else if (IP_EVENT == base)
    {
        if (IP_EVENT_STA_GOT_IP == id)
        {
            lNetLog("FreeRTOS event:'%s' Network: IP Obtained\n", base);
            identified = true;
        }
        else if (IP_EVENT_STA_LOST_IP == id)
        {
            lNetLog("FreeRTOS event:'%s' Network: IP Lost\n", base);
            identified = true;
        }
        else if (IP_EVENT_AP_STAIPASSIGNED == id)
        {
            ip_event_ap_staipassigned_t *assignedIP = (ip_event_ap_staipassigned_t *)event_data;
            esp_ip4_addr_t newIP = assignedIP->ip;
            unsigned char octet[4] = {0, 0, 0, 0};
            for (int i = 0; i < 4; i++)
            {
                octet[i] = (newIP.addr >> (i * 8)) & 0xFF;
            }
            lNetLog("FreeRTOS event:'%s' Network: STA IP Assigned: %d.%d.%d.%d\n", base, octet[0], octet[1], octet[2], octet[3]);
            identified = true;
        }
    }

    if (false == identified)
    {
        lLog("@TODO lunokIoT: unamanged FreeRTOS event:'%s' id: '%d' \n", base, id);
    }
}

// this loop is the HEART of system

static void SystemLoopTask(void *args) {
    unsigned long nextSySTick = 0; // heartbeat of system
    unsigned long nextIntTick = 0; // inquiry interrupts
    unsigned long nextLogMark = 0; // do a log mark
    size_t markCount = 0;
    while (true)
    {
        esp_task_wdt_reset();
        delay(10);
        // check for AXP int's
        if (irqAxp) {
            ttgo->power->readIRQ();
            if (ttgo->power->isChargingIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery charging\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_CHARGING, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            }
            else if (ttgo->power->isChargingDoneIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery fully charged\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_FULL, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            }
            else if (ttgo->power->isBattEnterActivateIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery active\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_ACTIVE, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            }
            else if (ttgo->power->isBattExitActivateIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery free\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_FREE, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            }
            else if (ttgo->power->isBattPlugInIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery present\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_PRESENT, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            }
            else if (ttgo->power->isBattRemoveIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery removed\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_REMOVED, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            }
            else if (ttgo->power->isBattTempLowIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery temperature low\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_TEMP_LOW, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
            }
            else if (ttgo->power->isBattTempHighIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Battery temperature high\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_TEMP_HIGH, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
            }
            else if (ttgo->power->isVbusPlugInIRQ()) {
                ttgo->power->clearIRQ();
                vbusPresent = true;
                lEvLog("AXP202: Power source\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_POWER, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            }
            else if (ttgo->power->isVbusRemoveIRQ()) {
                ttgo->power->clearIRQ();
                vbusPresent = false;
                lEvLog("AXP202: No power\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_NOPOWER, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            }
            else if (ttgo->power->isPEKShortPressIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Event PEK Button short press\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_SHORT, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
            }
            else if (ttgo->power->isPEKLongtPressIRQ()) {
                ttgo->power->clearIRQ();
                lEvLog("AXP202: Event PEK Button long press\n");
                irqAxp = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_LONG, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
            } else {
                //ttgo->power->clearIRQ();
                lLog("@TODO unknown interrupt call from AXP202 ?...\n");
            }
        }
        // check the BMA int's
        if (irqBMA) {
            bool rlst;
            do {
                esp_task_wdt_reset();
                delay(1);
                // Read the BMA423 interrupt status,
                // need to wait for it to return to true before continuing
                rlst = ttgo->bma->readInterrupt();
            } while (!rlst);
            // Check if it is a step interrupt
            if (ttgo->bma->isStepCounter()) {
                irqBMA = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_STEPCOUNTER, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            } else if (ttgo->bma->isActivity()) {
                irqBMA = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_ACTIVITY, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            } else if (ttgo->bma->isAnyNoMotion()) {
                irqBMA = false;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_NOMOTION, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            } else {
                lLog("@TODO unknown interrupt call from BMA423 ?...\n");
                // this intentionally creates a infinite loop and WATCHDOG interrupt
            }
        }
        /*
        // check the RTC int
        if (irqRTC) {
            irqRTC = false;
            bool isAlarm = ttgo->rtc->alarmActive();
            if ( true == isAlarm ) {
                Serial.println("PCF8563: Event: Alarm");
                ttgo->rtc->disableAlarm();
                ttgo->rtc->setDateTime(2019, 8, 12, 15, 0, 53);
                ttgo->rtc->setAlarmByMinutes(1);
                ttgo->rtc->enableAlarm();
                Serial.println("@TODO @DEBUG RE-ENABLE ALARM... wakeup BUG pending...");
            }
            bool isTimer = ttgo->rtc->isTimerActive();
            if ( true == isTimer ) {
                Serial.println("PCF8563: Event: Timer");
                ttgo->rtc->disableTimer();
            }
            delay(10);
        }*/

        // log mark
        if (millis() > nextLogMark) {
            lEvLog("-- MARK (%d) --\n", markCount);
            markCount++;
            nextLogMark = millis() + 120000;
        }

        //  system tick
        if (millis() > nextSySTick) {
            if (true == ttgo->bl->isOn()) {
                esp_task_wdt_reset();
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_TICK, nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
            }
            nextSySTick = millis() + (1000 / 12);
        }
    }
}
void SystemEventsStart()
{
    // configure system event loop (send and receive messages from other parts of code)
    esp_event_loop_args_t lunokIoTSystemEventloopConfig = {
        .queue_size = 10,                           // maybe so big
        .task_name = "lEvTask",                     // lunokIoT Event Task
        .task_priority = uxTaskPriorityGet(NULL),   // when the freeRTOS wants
        .task_stack_size = LUNOKIOT_APP_STACK_SIZE, // don't need so much
        .task_core_id = tskNO_AFFINITY              // bah, dontcare
    };                                              // details: https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-reference/system/esp_event.html#_CPPv421esp_event_loop_args_t

    // Create my own event loop
    ESP_ERROR_CHECK(esp_event_loop_create(&lunokIoTSystemEventloopConfig, &systemEventloopHandler));

    // Register the handler for task iteration event. Notice that the same handler is used for handling event on different loops.
    // The loop handle is provided as an argument in order for this example to display the loop the handler is being run on.
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_READY, SystemEventReady, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_TICK, SystemEventTick, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_SHORT, AXPEventPEKShort, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_LONG, AXPEventPEKLong, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_STEPCOUNTER, BMAEventStepCounter, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_ACTIVITY, BMAEventActivity, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_NOMOTION, BMAEventNoActivity, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_TIMER, SystemEventTimer, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_POWER, SystemEventPMUPower, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_NOPOWER, SystemEventPMUPower, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, AnyEventSystem, nullptr, NULL);
    // Create the event source task with the same priority as the current task
    xTaskCreate(SystemLoopTask, "STask", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);
    lEvLog("lunokIoT: User event loop running\n");

    // get the freeRTOS event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, FreeRTOSEventReceived, NULL));
    lEvLog("lunokIoT: System event loop running\n");
}

/*
 * Eyecandy notification of boot end
 */
void SystemEventBootEnd()
{
    provisioned = NVS.getInt("provisioned"); // update the value
    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_READY, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
}

/*
 * Get all samples possible
 */
void TakeSamples()
{
    //@TODO AXP, hall, and others must be collected
    TakeBMPSample();
    // Serial.printf("BMA423: Acceleromether: X: %d Y: %d Z: %d\n", accX, accY, accZ);
}
