#include <Arduino.h>
#include <LilyGoWatch.h>
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


#include "UI/UI.hpp"

#include "../app/Shutdown.hpp"
#include "../app/Provisioning2.hpp"

#include "../app/Watchface.hpp"

bool systemSleep = false;
// Event loops
esp_event_loop_handle_t systemEventloopHandler;
ESP_EVENT_DEFINE_BASE(SYSTEM_EVENTS);

bool provisioned=false;
int hallData = 0;               // hall sensor data
bool vbusPresent =false;        // USB connected?
int batteryPercent;             // -1 if no batt
uint8_t bmaRotation;            // bma ground?
float axpTemp;
float bmaTemp;
const char * currentActivity="None";
uint32_t stepCount=0;
uint32_t lastBootStepCount = 0;

int16_t accXMax=-5000;
int16_t accXMin=5000;
int16_t accYMax=-5000;
int16_t accYMin=5000;
int16_t accZMax=-5000;
int16_t accZMin=5000;

int16_t pcX = 0;        // percent
int16_t pcY = 0;
int16_t pcZ = 0;
int16_t lpcX = 0;       // last percent
int16_t lpcY = 0;
int16_t lpcZ = 0;
float degX = 0.0;       // degrees
float degY = 0.0;
float degZ = 0.0;
int16_t accX = 0;       // accelerator value
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
void BMPIntHandler() {
    Serial.println("BMA423: NVS: Loading last session values...");

    int32_t tempStepCount = NVS.getInt("stepCount");

    int16_t tempAccXMax = NVS.getInt("accXMax");
    int16_t tempAccXMin = NVS.getInt("accXMin");
    int16_t tempAccYMax = NVS.getInt("accYMax");
    int16_t tempAccYMin = NVS.getInt("accYMin");
    int16_t tempAccZMax = NVS.getInt("accZMax");
    int16_t tempAccZMin = NVS.getInt("accZMin");

    uint32_t tempStepsBMAActivityStationary = NVS.getInt("sBMAASta");
    unsigned long tempTimeBMAActivityStationary = NVS.getInt("tBMAASta");
    if ( false != tempStepsBMAActivityStationary ) { stepsBMAActivityStationary=tempStepsBMAActivityStationary; }
    if ( false != tempTimeBMAActivityStationary ) { timeBMAActivityStationary=tempTimeBMAActivityStationary; }
    Serial.printf("BMA423: NVS: Last session stepsBMAActivityStationary = %d\n",stepsBMAActivityStationary);
    Serial.printf("BMA423: NVS: Last session timeBMAActivityStationary = %lu\n",timeBMAActivityStationary);
    uint32_t tempStepsBMAActivityWalking = NVS.getInt("sBMAAWalk");
    unsigned long tempTimeBMAActivityWalking = NVS.getInt("tBMAAWalk");
    if ( false != tempStepsBMAActivityWalking ) { stepsBMAActivityWalking=tempStepsBMAActivityWalking; }
    if ( false != tempTimeBMAActivityWalking ) { timeBMAActivityWalking=tempTimeBMAActivityWalking; }
    Serial.printf("BMA423: NVS: Last session stepsBMAActivityWalking = %d\n",stepsBMAActivityWalking);
    Serial.printf("BMA423: NVS: Last session timeBMAActivityWalking = %lu\n",timeBMAActivityWalking);
    uint32_t tempStepsBMAActivityRunning = NVS.getInt("sBMAARun");
    unsigned long tempTimeBMAActivityRunning = NVS.getInt("tBMAARun");
    if ( false != tempStepsBMAActivityRunning ) { stepsBMAActivityRunning=tempStepsBMAActivityRunning; }
    if ( false != tempTimeBMAActivityRunning ) { timeBMAActivityRunning=tempTimeBMAActivityRunning; }
    Serial.printf("BMA423: NVS: Last session stepsBMAActivityRunning = %d\n",stepsBMAActivityRunning);
    Serial.printf("BMA423: NVS: Last session timeBMAActivityRunning = %lu\n",timeBMAActivityRunning);
    uint32_t tempStepsBMAActivityNone = NVS.getInt("sBMAANone");
    unsigned long tempTimeBMAActivityNone = NVS.getInt("tBMAANone");
    if ( false != tempStepsBMAActivityNone ) { stepsBMAActivityNone=tempStepsBMAActivityNone; }
    if ( false != tempTimeBMAActivityNone ) { timeBMAActivityNone=tempTimeBMAActivityNone; }
    Serial.printf("BMA423: NVS: Last session stepsBMAActivityNone = %d\n",stepsBMAActivityNone);
    Serial.printf("BMA423: NVS: Last session timeBMAActivityNone = %lu\n",timeBMAActivityNone);

    if ( false != tempStepCount ) {
        lastBootStepCount = tempStepCount;
        stepCount = lastBootStepCount;
    }

    if ( false != tempAccXMax ) { accXMax=tempAccXMax; }
    if ( false != tempAccXMin ) { accXMin=tempAccXMin; }
    if ( false != tempAccYMax ) { accYMax=tempAccYMax; }
    if ( false != tempAccYMin ) { accYMin=tempAccYMin; }
    if ( false != tempAccZMax ) { accZMax=tempAccZMax; }
    if ( false != tempAccZMin ) { accZMin=tempAccZMin; }

    Serial.printf("BMA423: NVS: Last session stepCount = %d\n",tempStepCount);

    Serial.printf("BMA423: NVS: X(%d/%d) ",accXMax,accXMin);
    Serial.printf("Y(%d/%d) ",accYMax,accYMin);
    Serial.printf("Z(%d/%d)\n",accZMax,accZMin);
    // Accel parameter structure
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
    //cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_50HZ;
    /*!
        G-range, Optional parameters:
            - BMA4_ACCEL_RANGE_2G
            - BMA4_ACCEL_RANGE_4G
            - BMA4_ACCEL_RANGE_8G
            - BMA4_ACCEL_RANGE_16G
    */
    //cfg.range = BMA4_ACCEL_RANGE_8G;
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
    //cfg.bandwidth = BMA4_ACCEL_RES_AVG128;

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
    attachInterrupt(BMA423_INT1, [] {
        // Set interrupt to set irq value to 1
        irqBMA = true;
    }, RISING); //It must be a rising edge

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

void AXPIntHandler() {

    // ADC monitoring must be enabled to use the AXP202 monitoring function
    ttgo->power->adc1Enable(
        AXP202_VBUS_VOL_ADC1 |
        AXP202_VBUS_CUR_ADC1 |
        AXP202_BATT_CUR_ADC1 |
        AXP202_BATT_VOL_ADC1,
        true);

    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(AXP202_INT, [] {
        irqAxp = true;
    }, FALLING);

    ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_PEK_LONGPRESS_IRQ |
    AXP202_BATT_LOW_TEMP_IRQ | AXP202_BATT_OVER_TEMP_IRQ | 
    AXP202_BATT_REMOVED_IRQ | AXP202_BATT_CONNECT_IRQ | 
    AXP202_BATT_ACTIVATE_IRQ | AXP202_BATT_EXIT_ACTIVATE_IRQ | 
    AXP202_CHARGING_IRQ | AXP202_CHARGING_FINISHED_IRQ, true);
    ttgo->power->clearIRQ();    
}

void TakeSamples() {
    TakeBMPSample();
    //Serial.printf("BMA423: Acceleromether: X: %d Y: %d Z: %d\n", accX, accY, accZ);
}
void WakeUpReason() {

  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  Serial.printf("ESP32 Wake up from: ");
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Interrupt triggered on ext0 <-- (PMU) AXP202"); break;
    case ESP_SLEEP_WAKEUP_EXT1 :
        {
            Serial.printf("Interrupt triggered on ext1 <-- ");
            uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
            if ( GPIO_SEL_37 == GPIO_reason) { Serial.println("(RTC) PCF8563"); }
            else if ( GPIO_SEL_38 == GPIO_reason) { Serial.println("(TOUCH) FocalTech"); }
            else if ( GPIO_SEL_39 == GPIO_reason) { Serial.println("(ACCEL) BMA423"); }
            else {
                //Serial.printf("unexpected (?) GPIO %d reason: %u\n", impliedGPIO, GPIO_reason);
                Serial.print("WARNING: Unexpected: GPIO ");
                Serial.println((log(GPIO_reason))/log(2), 0);
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
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer");
        Serial.printf("Event: Forced sample due no user activity for a while (%ds timeout)\n", LUNOKIOT_WAKE_TIME_S);
        TakeSamples();
        DoSleep();
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
uint16_t doSleepThreads=0;
SemaphoreHandle_t DoSleepTaskSemaphore = xSemaphoreCreateMutex();

static void DoSleepTask(void* args) {

    bool done = xSemaphoreTake( DoSleepTaskSemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS );
    if ( false == done ) {
        Serial.println("ESP32: DoSleep DISCARDED: already in progress...");
        vTaskDelete(NULL);
    }

    doSleepThreads++;
    Serial.printf("ESP32: Free heap: %d KB\n", ESP.getFreeHeap()/1024);
    Serial.printf("ESP32: Free PSRAM: %d KB\n", ESP.getFreePsram()/1024);

    Serial.printf("ESP32: DoSleep(%d) began!\n", doSleepThreads);
    //Serial.printf("WIFI STATUS: %d\n", WiFi.status());
    size_t retries = 3;
    Serial.printf("WIFISTATUS: %d\n",WiFi.status());
    //while ( ( WL_NO_SHIELD != WiFi.status())&&( WL_IDLE_STATUS != WiFi.status() ) ) {
    while ( WL_CONNECTED == WiFi.status()) {
        Serial.printf("ESP32: DoSleep WAITING (%d): WiFi in use, waiting for radio powerdown....\n", retries);
        delay(2000);
        retries--;
        if ( 0 == retries ) {
            Serial.println("ESP32: Forcing WiFI off due timeout");
            WiFi.disconnect(true);
            delay(100);
            break;
        }
    }
    WiFi.mode(WIFI_OFF);
    delay(200);
    Serial.println("ESP32: WiFi released, DoSleep continue...");
    // AXP202 interrupt gpio_35
    // RTC interrupt gpio_37
    // touch panel interrupt gpio_38
    // bma interrupt gpio_39
    ScreenSleep();
    esp_sleep_enable_ext0_wakeup( GPIO_NUM_35, 0);
    esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR*LUNOKIOT_WAKE_TIME_S);
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_39, ESP_EXT1_WAKEUP_ANY_HIGH);
    Serial.println("ESP32: -- ZZz --");
    Serial.flush();
    delay(100);
    esp_light_sleep_start(); // device sleeps now
    Serial.println("ESP32: -- Wake -- o_O'"); // good morning!!
    systemSleep = false;
    xSemaphoreGive( DoSleepTaskSemaphore );

    WakeUpReason();

    Serial.printf("ESP32: DoSleep(%d) dies here!\n", doSleepThreads);


    vTaskDelete(NULL);
}

void DoSleep() {
    systemSleep = true;
    xTaskCreate(DoSleepTask, "SLEEP", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);
}

static void BMAEventActivity(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    const char * nowActivity = ttgo->bma->getActivity();
    if ( 0 != strcmp(currentActivity,nowActivity) ) {
        unsigned long endBMAActivity = millis();
        unsigned long totalBMAActivity = endBMAActivity-beginBMAActivity;
        uint32_t endStepsBMAActivity = stepCount;
        uint32_t totalStepsBMAActivity = endStepsBMAActivity-beginStepsBMAActivity;
        //time counter attribution
        if ( 0 == strcmp("BMA423_USER_STATIONARY",currentActivity)) {
            stepsBMAActivityStationary+=totalStepsBMAActivity;
            timeBMAActivityStationary += totalBMAActivity;
        } else if ( 0 == strcmp("BMA423_USER_WALKING",currentActivity)) {
            stepsBMAActivityWalking+=totalStepsBMAActivity;
            timeBMAActivityWalking += totalBMAActivity;
        } else if ( 0 == strcmp("BMA423_USER_RUNNING",currentActivity)) {
            stepsBMAActivityRunning+=totalStepsBMAActivity;
            timeBMAActivityRunning += totalBMAActivity;
        } else if ( 0 == strcmp("BMA423_STATE_INVALID",currentActivity)) {
            stepsBMAActivityInvalid+=totalStepsBMAActivity;
            timeBMAActivityInvalid += totalBMAActivity;
        } else if ( 0 == strcmp("None",currentActivity)) {
            stepsBMAActivityNone+=totalStepsBMAActivity;
            timeBMAActivityNone += totalBMAActivity;
        }
        Serial.printf("BMA423: Event: Last actity: %s\n",currentActivity);
        Serial.printf("BMA423: Event: Current actity: %s\n",nowActivity);
        Serial.printf("BMA423: Timers:\n");
        Serial.printf("BMA423:        Stationary: %lu secs (%u steps)\n",timeBMAActivityStationary/1000, stepsBMAActivityStationary );
        Serial.printf("BMA423:           Walking: %lu secs (%u steps)\n",timeBMAActivityWalking/1000, stepsBMAActivityWalking );
        Serial.printf("BMA423:           Running: %lu secs (%u steps)\n",timeBMAActivityRunning/1000, stepsBMAActivityRunning );
        Serial.printf("BMA423:           Invalid: %lu secs (%u steps)\n",timeBMAActivityInvalid/1000, stepsBMAActivityInvalid );
        Serial.printf("BMA423:              None: %lu secs (%u steps)\n",timeBMAActivityNone/1000, stepsBMAActivityNone );

        beginBMAActivity = millis();
        beginStepsBMAActivity = stepCount;
        currentActivity=nowActivity;
    }
    if ( false == ttgo->bl->isOn() ) {
        DoSleep();
    }
}
static void BMAEventNoActivity(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    Serial.println("BMA423: Event: No actity");
    if ( false == ttgo->bl->isOn() ) {
        DoSleep();
    }
}

static void BMAEventStepCounter(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // Get step data from register
    uint32_t nowSteps = ttgo->bma->getCounter();
    if (nowSteps != (stepCount - lastBootStepCount) ) {
        stepCount = nowSteps + lastBootStepCount;
        Serial.printf("BMA423: Event: Steps: %d\n",stepCount);
    }
    if ( false == ttgo->bl->isOn() ) {
        DoSleep();
    }
}

void SaveDataBeforeShutdown() {
    NVS.setInt("provisioned", provisioned, false);

    NVS.setInt("accXMax",accXMax,false); 
    NVS.setInt("accXMin",accXMin,false); 
    NVS.setInt("accYMax",accYMax,false); 
    NVS.setInt("accYMin",accYMin,false); 
    NVS.setInt("accZMax",accZMax,false);
    NVS.setInt("accZMin",accZMin,false);

    NVS.setInt("sBMAASta",stepsBMAActivityStationary,false);
    NVS.setInt("tBMAASta",(int64_t)timeBMAActivityStationary,false);
    NVS.setInt("sBMAAWalk",stepsBMAActivityWalking,false);
    NVS.setInt("tBMAAWalk",(int64_t)timeBMAActivityWalking,false);
    NVS.setInt("sBMAARun",stepsBMAActivityRunning,false);
    NVS.setInt("tBMAARun",(int64_t)timeBMAActivityRunning,false);
    NVS.setInt("sBMAAInvalid",stepsBMAActivityInvalid,false);
    NVS.setInt("tBMAAInvalid",(int64_t)timeBMAActivityInvalid,false);
    NVS.setInt("sBMAANone",stepsBMAActivityNone,false);
    NVS.setInt("tBMAANone",(int64_t)timeBMAActivityNone,false);

    NVS.setInt("stepCount",stepCount,false);
    NVS.commit();
    NVS.close();
}

static void AXPEventPEKLong(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    LaunchApplication(new ShutdownApplication());
    SaveDataBeforeShutdown();
}  

static void AXPEventPEKShort(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {

    /*
    static unsigned long lastPEK = 0;
    if ( millis() > lastPEK ) {
        */
        if ( ttgo->bl->isOn() ) {
            Serial.println("Event: user wants to put device to sleep");
            ScreenSleep();
            LaunchApplication(nullptr);
            delay(100);
            DoSleep();
        } else {
            Serial.println("Event: user wants to get a screen");
            TakeSamples();
            //delay(50);
            LaunchApplication(new WatchfaceApplication());
            ScreenWake();
            //delay(100);
        }
        /*
        lastPEK = millis()+50;
    } else {
        Serial.println("Event: discarded PEK by: Too fast rule");
    }
    */
}

void ScreenWake() {
    if ( false == ttgo->bl->isOn() ) {
        //setCpuFrequencyMhz(240);
        //delay(50);
        //ttgo->rtc->syncToSystem();
        ttgo->displayWakeup();
        UINextTimeout = millis()+UITimeout;
        ttgo->bl->on();
        ttgo->touchWakup();
        if ( ttgo->power->isVBUSPlug() ) {
            ttgo->setBrightness(255);
        } //else { ttgo->setBrightness(30); }
        FPS = MAXFPS;
        UINextTimeout = millis()+UITimeout;
    }
}

void ScreenSleep() {
    if ( true == ttgo->bl->isOn() ) {
        ttgo->bl->off();
        Serial.println("UI: Put screen to sleep now");
        ttgo->displaySleep();
        delay(50);
        ttgo->touchToSleep();
        Serial.flush();
        delay(50);
        //setCpuFrequencyMhz(80);
        //ttgo->rtc->syncToSystem();
        //delay(50);
        //@TODO send hint for suspend main CPU via system user-defined event loop like LunokIoTEventloopTask
    }
}

void TakeBMPSample() {
    Accel acc;
    // Get acceleration data
    bool res = ttgo->bma->getAccel(acc);

    if (res != false) {
        if ( ( accX != acc.x ) || ( accY != acc.y ) || ( accZ != acc.z ) ) {
            accX = acc.x;
            accY = acc.y;
            accZ = acc.z;
            // set min/max values (to get percentage)
            if ( accXMax < acc.x ) { accXMax = acc.x; }
            else if ( accXMin > acc.x ) { accXMin = acc.x; }
            if ( accYMax < acc.y ) { accYMax = acc.y; }
            else if ( accYMin > acc.y ) { accYMin = acc.y; }
            if ( accZMax < acc.z ) { accZMax = acc.z;  }
            else if ( accZMin > acc.z ) { accZMin = acc.z; }
            if ( 0 != (accXMax - accXMin)) {
                lpcX = pcX;
                pcX = ((acc.x - accXMin) * 100) / (accXMax - accXMin);
                degX = float(((acc.x - accXMin) * 360.0) / (accXMax - accXMin));
            }
            if ( 0 != (accYMax - accYMin)) {
                lpcY = pcY;
                pcY = ((acc.y - accYMin) * 100) / (accYMax - accYMin);
                degY = float(((acc.y - accYMin) * 360.0) / (accYMax - accYMin));
            }
            if ( 0 != (accZMax - accZMin)) {
                lpcZ = pcZ;
                pcZ = ((acc.z - accZMin) * 100) / (accZMax - accZMin);
                degZ = float(((acc.z - accZMin) * 360.0) / (accZMax - accZMin));
            }
            
            //Serial.printf("BMA423: Acceleromether: MaxX: %d MaxY: %d MaxZ: %d\n", accXMax, accYMax, accZMax);
            //Serial.printf("BMA423: Acceleromether: MinX: %d MinY: %d MinZ: %d\n", accXMin, accYMin, accZMin);
            // Serial.printf("BMA423: Acceleromether: Xpc: %d%% Ypc: %d%% Zpc: %d%%\n",pcX,pcY,pcZ);
            
            //Serial.printf("BMA423: Acceleromether: X: %dº Y: %dº Z: %dº\n",degX,degY,degZ);
    
            //Serial.printf("BMA423: Acceleromether: X: %d Y: %d Z: %d\n", acc.x, acc.y, acc.z);

#ifdef LUNOKIOT_SERIAL_CSV
            // RAW
            Serial.printf("CSV:%f,%f,%f\n",degX,degY,degZ);
            
            // smoothed 2x
            static float ldegX = degX;
            static float ldegY = degY;
            static float ldegZ = degZ;
            //Serial.printf("CSV:%f,%f,%f\n",(degX+ldegX)/2.0,(degY+ldegY)/2.0,(degZ+ldegZ)/2.0);
            ldegX = degX;
            ldegY = degY;
            ldegZ = degZ; 
#endif           
            esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMP_EVENT_MOVE,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
        }
    }
    float nowBMATemp = ttgo->bma->temperature();
    if ( nowBMATemp != bmaTemp ) {
        //Serial.printf("BMA423: Temperature: %.1fºC\n", bmaTemp);
        bmaTemp = nowBMATemp;
        esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_TEMP,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
    }
    // Obtain the BMA423 direction,
    // so that the screen orientation is consistent with the sensor
    uint8_t nowRotation = ttgo->bma->direction();
    if (nowRotation != bmaRotation) {
        //Serial.printf("BMA423: Direction: Current: %u Obtained: %u Last: %u\n", ttgo->tft->getRotation(), rotation, prevRotation);
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
        esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_DIRECTION,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);

    }
}

unsigned long nextSlowSensorsTick = 0;
unsigned long nextSensorsTick = 0;
static void SystemEventTick(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {

    if ( millis() > nextSlowSensorsTick ) { // poll some sensors ( slow pooling )
        hallData = hallRead();
        
        // @TODO obtain more AXP202 info
        vbusPresent = ttgo->power->isVBUSPlug();
        // basic Battery info
        if (ttgo->power->isBatteryConnect()) {
            int nowBatteryPercent = ttgo->power->getBattPercentage();
            if ( nowBatteryPercent != batteryPercent ) {
                batteryPercent = nowBatteryPercent;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_PC,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            }
            batteryPercent = nowBatteryPercent;
            
        } else {
            if ( -1 != batteryPercent ) {
                batteryPercent = -1;
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_NOTFOUND,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
            }
            batteryPercent = -1;
        }

        // begin tick report
        float nowAXPTemp = ttgo->power->getTemp();
        if ( nowAXPTemp != axpTemp ) {
            //Serial.printf("AXP202: Temperature: %.1fºC\n", axpTemp);
            axpTemp = nowAXPTemp;
            esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_TEMPERATURE,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
        }

        nextSlowSensorsTick = millis()+(1000/1);
    }

    if ( millis() > nextSensorsTick ) { // poll some other sensors ( normal pooling )
        TakeSamples();
        nextSensorsTick = millis()+(1000/8);
    }
    
    /*
    static bool lastBLStatus = ttgo->bl->isOn();
    bool thisSample = ttgo->bl->isOn();
    if ( ( false == thisSample ) && ( true == lastBLStatus )) {
        Serial.println("Screen is off, the ESP32 must be going to sleep accordly...");
        lastBLStatus = thisSample;
        DoSleep();
        return;
    }
    lastBLStatus = thisSample;
    */
}

static void SystemEventReady(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    
    Serial.println("lunokIoT: System event: up and running...");

    if ( provisioned ) {
        Serial.println("lunokIoT: Device provisioned: Launching 'WatchfaceApplication'...");
        LaunchApplication(new WatchfaceApplication());
        return;
    }
    Serial.println("lunokIoT: Device not provisioned: Launching 'Provisioning2Application'...");
    LaunchApplication(new Provisioning2Application());
}

static void FreeRTOSEventReceived(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-guides/event-handling.html#event-ids-and-corresponding-data-structures
    bool identified=false;
    if ( WIFI_PROV_EVENT == base) {
        if ( WIFI_PROV_INIT == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi provisioning: Initialized\n",base);
            identified=true;
        } else if ( WIFI_PROV_START == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi provisioning: Started\n",base);
            identified=true;
        } else if ( WIFI_PROV_CRED_RECV == id ) {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            Serial.printf("FreeRTOS event:'%s' WiFi provisioning: Received Wi-Fi credentials SSID: '%s' PASSWORD: '%s'\n",base,(const char *) wifi_sta_cfg->ssid,(const char *) wifi_sta_cfg->password);
            identified=true;
        } else if ( WIFI_PROV_CRED_FAIL == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi provisioning: Bad credentials (WPA password)\n",base);
            identified=true;
        } else if ( WIFI_PROV_CRED_SUCCESS == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi provisioning: Credentials success\n",base);
            identified=true;
        } else if ( WIFI_PROV_END == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi provisioning: End\n",base);
            identified=true;
        } else if ( WIFI_PROV_DEINIT == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi provisioning: Deinit\n",base);
            identified=true;
        }
    } else if ( WIFI_EVENT == base) {
        if ( WIFI_EVENT_SCAN_DONE == id ) {
            wifi_event_sta_scan_done_t *scanData = (wifi_event_sta_scan_done_t *)event_data;
            const char * scanDoneStr = (scanData->status?"failed":"done");
            Serial.printf("FreeRTOS event:'%s' WiFi Scan %d %s (found: %d)\n",base,scanData->scan_id,scanDoneStr,scanData->number);
            identified=true;
        } else if ( WIFI_EVENT_STA_START == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi STA: Start\n",base);
            identified=true;
        } else if ( WIFI_EVENT_STA_STOP == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi STA: Stop\n",base);
            identified=true;
        } else if ( WIFI_EVENT_STA_CONNECTED == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi STA: Connected\n",base);
            identified=true;
        } else if ( WIFI_EVENT_STA_DISCONNECTED == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi STA: Disconnected\n",base);
            identified=true;
        } else if ( WIFI_EVENT_AP_START == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi AP: Start\n",base);
            identified=true;
        } else if ( WIFI_EVENT_AP_STOP == id ) {
            Serial.printf("FreeRTOS event:'%s' WiFi AP: Stop\n",base);
            identified=true;
        } else if ( WIFI_EVENT_AP_STACONNECTED == id ) {
            wifi_event_ap_staconnected_t *apStaData = (wifi_event_ap_staconnected_t *)event_data;
            Serial.printf("FreeRTOS event:'%s' WiFi AP: Client %02x:%02x:%02x:%02x:%02x:%02x connected\n",base,apStaData->mac[0],apStaData->mac[1],apStaData->mac[2],apStaData->mac[3],apStaData->mac[4],apStaData->mac[5]);
            identified=true;
        } else if ( WIFI_EVENT_AP_STADISCONNECTED == id ) {
            wifi_event_ap_stadisconnected_t *apStaData = (wifi_event_ap_stadisconnected_t *)event_data;
            Serial.printf("FreeRTOS event:'%s' WiFi AP: Client %02x:%02x:%02x:%02x:%02x:%02x disconnected\n",base,apStaData->mac[0],apStaData->mac[1],apStaData->mac[2],apStaData->mac[3],apStaData->mac[4],apStaData->mac[5]);
            identified=true;
        }
    } else if ( IP_EVENT == base) {
        if ( IP_EVENT_STA_GOT_IP == id ) {
            Serial.printf("FreeRTOS event:'%s' Network: IP Obtained\n",base);
            identified=true;
        } else if ( IP_EVENT_STA_LOST_IP == id ) {
            Serial.printf("FreeRTOS event:'%s' Network: IP Lost\n",base);
            identified=true;
        } else if ( IP_EVENT_AP_STAIPASSIGNED == id ) {
            ip_event_ap_staipassigned_t *assignedIP = (ip_event_ap_staipassigned_t *)event_data;
            esp_ip4_addr_t newIP = assignedIP->ip;
            unsigned char octet[4]  = {0,0,0,0};
            for (int i=0; i<4; i++) { octet[i] = ( newIP.addr >> (i*8) ) & 0xFF; }
            Serial.printf("FreeRTOS event:'%s' Network: STA IP Assigned: %d.%d.%d.%d\n",base,octet[0],octet[1],octet[2],octet[3]);
            identified=true;
        }
        
    }

    if (false == identified ) {
        Serial.printf("@TODO lunokIoT: unamanged FreeRTOS event:'%s' id: '%d' \n",base, id);
    }
}

static void SystemLoopTask(void* args) {
    unsigned long nextSySTick = 0; // for rest of system (including UI)
    unsigned long nextIntTick = 0; // inquiry interrupts
    while(true) {
        delay(1);
        if ( millis() > nextIntTick ) {
            // check for AXP int's
            if (irqAxp) {
                Serial.println("CHECK AXP");
                ttgo->power->readIRQ();
                irqAxp = false;

                if (ttgo->power->isChargingIRQ()) {
                    ttgo->power->clearIRQ();
                    Serial.println("AXP202: Battery charging");
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_CHARGING,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->power->isChargingDoneIRQ()){
                    ttgo->power->clearIRQ();
                    Serial.println("AXP202: Battery fully charged");
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_FULL,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->power->isBattEnterActivateIRQ()){
                    ttgo->power->clearIRQ();
                    Serial.println("AXP202: Battery active");
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_ACTIVE,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->power->isBattExitActivateIRQ()) {
                    ttgo->power->clearIRQ();
                    Serial.println("AXP202: Battery free");
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_FREE,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->power->isBattPlugInIRQ()) {
                    ttgo->power->clearIRQ();
                    Serial.println("AXP202: Battery present");
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_PRESENT,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->power->isBattRemoveIRQ()) {
                    ttgo->power->clearIRQ();
                    Serial.println("AXP202: Battery removed");
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_REMOVED,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->power->isBattTempLowIRQ()) {
                    ttgo->power->clearIRQ();
                    Serial.println("AXP202: Battery temperature low");
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_TEMP_LOW,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->power->isBattTempHighIRQ()) {
                    ttgo->power->clearIRQ();
                    Serial.println("AXP202: Battery temperature high");
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_BATT_TEMP_HIGH,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->power->isVbusPlugInIRQ()) {
                    //setCpuFrequencyMhz(240);
                    ttgo->power->clearIRQ();
                    vbusPresent = true;
                    if ( ttgo->bl->isOn() ) { ttgo->setBrightness(255); }
                    Serial.println("AXP202: Power source");
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_POWER,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->power->isVbusRemoveIRQ()) {
                    //setCpuFrequencyMhz(80);
                    ttgo->power->clearIRQ();
                    vbusPresent = false;
                    //if ( ttgo->bl->isOn() ) { ttgo->setBrightness(30); }
                    Serial.println("AXP202: No power");
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_NOPOWER,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->power->isPEKShortPressIRQ()) {
                    ttgo->power->clearIRQ();
                    Serial.println("AXP202: Event PEK Button short press");
                    Serial.flush();
                    delay(50); // forced delay for debounce
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_SHORT,nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
                    delay(50); // forced delay for debounce
                } else if (ttgo->power->isPEKLongtPressIRQ()) {
                    ttgo->power->clearIRQ();
                    Serial.println("AXP202: Event PEK Button long press");
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_LONG,nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
                } else {
                    ttgo->power->clearIRQ();
                    Serial.println("@TODO unknown interrupt call from AXP202 ?...");
                }
                continue; // this makes go to begin of loop (and check for other interrupts inmediatly)
            }
            // check the BMA int's
            if (irqBMA) {
                Serial.println("CHECK BMA");
                irqBMA = false;
                bool  rlst;
                do {
                    delay(0);
                    // Read the BMA423 interrupt status,
                    // need to wait for it to return to true before continuing
                    rlst =  ttgo->bma->readInterrupt();
                } while (!rlst);
                // Check if it is a step interrupt
                if (ttgo->bma->isStepCounter()) {
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_STEPCOUNTER,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->bma->isActivity() ) {
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_ACTIVITY,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else if (ttgo->bma->isAnyNoMotion()) {
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_NOMOTION,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
                } else {
                    Serial.println("@TODO unknown interrupt call from BMA423 ?...");
                }
                continue; // this makes go to begin of loop (and check for other interrupts inmediatly)
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


            nextIntTick = millis()+(1000/6);
        }


        //if ( systemSleep ) { continue; }
        // system tick
        if ( millis() > nextSySTick ) {
            if ( true == ttgo->bl->isOn() ) {
                esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_TICK, nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
            }
            nextSySTick = millis()+(1000/24);
        }
        //delay(1000/24);
    }
}
void SystemEventsStart() {

    // create the system event loop
    esp_event_loop_args_t lunokIoTSystemEventloopConfig = {
        .queue_size = 10,
        .task_name = "lEvTask", // task will be created
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = LUNOKIOT_TASK_STACK_SIZE,
        .task_core_id = tskNO_AFFINITY
    };
    // Create the event loops
    ESP_ERROR_CHECK(esp_event_loop_create(&lunokIoTSystemEventloopConfig, &systemEventloopHandler));

    // Register the handler for task iteration event. Notice that the same handler is used for handling event on different loops.
    // The loop handle is provided as an argument in order for this example to display the loop the handler is being run on.
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_READY, SystemEventReady, nullptr, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_TICK, SystemEventTick, nullptr, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_SHORT, AXPEventPEKShort,nullptr, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_PEK_LONG, AXPEventPEKLong,nullptr, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_STEPCOUNTER, BMAEventStepCounter,nullptr, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_ACTIVITY, BMAEventActivity,nullptr, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, BMA_EVENT_NOMOTION, BMAEventNoActivity,nullptr, NULL));
    // Create the event source task with the same priority as the current task
    xTaskCreate(SystemLoopTask, "STask", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);
    Serial.println("lunokIoT: User event loop running");


    // get the freeRTOS event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, FreeRTOSEventReceived,NULL));

    Serial.println("lunokIoT: System event loop running");
}

void SystemEventBootEnd() {
    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_READY,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
}

#include <WiFi.h>
std::list<NetworkTaskDescriptor *> networkPendingTasks = {};

bool RemoveNetworkTask(NetworkTaskDescriptor *oldTsk) {
    Serial.printf("RemoveNetworkTask: Task %p '%s' removed\n", oldTsk,oldTsk->name);
    networkPendingTasks.remove(oldTsk);
    return true;
}

bool AddNetworkTask(NetworkTaskDescriptor *nuTsk) {
    size_t offset = 0;
    for (auto const& tsk : networkPendingTasks) {
        if ( tsk == nuTsk ) {
            Serial.printf("AddNetworkTask: Task %p '%s' already on the list (offset: %d)\n", tsk,tsk->name, offset);
            return false;
        }
        offset++;
    }
    networkPendingTasks.push_back(nuTsk);
    Serial.printf("AddNetworkTask: Task %p '%s' added (offset: %d)\n", nuTsk,nuTsk->name, offset);
    return true;
}

static void NetworkHandlerTask(void* args) {
    delay(5000); // arbitrary wait before begin first connection
    unsigned long nextConnectMS = 0;
    unsigned long beginConnected = -1;
    unsigned long beginIdle = -1;
    while(true) {
        delay(1000);
        if ( systemSleep ) { continue; }
        if ( false == provisioned ) { continue; } // nothing to do without provisioning :(
        
        if ( millis() > nextConnectMS ) { // begin connection?
            //check the pending tasks... if no one, don't connect
            Serial.println("Network: Timed WiFi connection procedure begin");
            bool mustStart = false;

            for (auto const& tsk : networkPendingTasks) {
                //CHECK THE TIMEOUTS
                if ( -1 == tsk->_nextTrigger ) {
                    tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                } else {
                    if ( millis() > tsk->_nextTrigger ) {
                        Serial.printf("NetworkTask: Pending task '%s'\n", tsk->name);
                        mustStart = true;
                    }
                }
                tsk->_lastCheck = millis();
            }
            if ( mustStart ) {
                WiFi.begin();
                WiFi.setAutoReconnect(false);
            } else {
                Serial.println("Network: No tasks pending for this period... don't launch WiFi");
                for (auto const& tsk : networkPendingTasks) {
                    Serial.printf("NetworkTask: Task '%s' In: %d secs\n", tsk->name, (tsk->_nextTrigger-millis())/1000);
                }
            }
            nextConnectMS = millis()+ReconnectPeriodMs;
            continue;
        }

        wl_status_t currStat = WiFi.status();
        /*
        if ( WL_IDLE_STATUS == currStat) {
            if ( -1 == beginIdle ) {
                beginIdle = millis();
            } else {
                unsigned long idleMS = millis()-beginIdle;
                if ( idleMS > NetworkTimeout ) {
                    Serial.printf("Network: WiFi TIMEOUT (idle) at %d sec\n", idleMS/1000);
                    WiFi.disconnect();
                    WiFi.mode(WIFI_OFF);
                    beginIdle=-1;
                    continue;
                }
                Serial.printf("Network: WiFi idle: %d sec\n", idleMS/1000);
            }
            continue;
        } else */
        if ( WL_CONNECTED == currStat) {
            if ( -1 == beginConnected ) {
                beginConnected = millis();
            } else {
                unsigned long connectedMS = millis()-beginConnected;
                if ( connectedMS > NetworkTimeout ) {
                    Serial.printf("Network: WiFi TIMEOUT (connected) at %d sec of use\n", connectedMS/1000);
                    WiFi.disconnect(true);
                    delay(100);
                    WiFi.mode(WIFI_OFF);
                    beginConnected=-1;
                    continue;
                }
                //CHECK THE tasks
                for (auto const& tsk : networkPendingTasks) {
                    if ( -1 == tsk->_nextTrigger ) {
                        tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                    } else {
                        if ( millis() > tsk->_nextTrigger ) {
                            Serial.printf("NetworkTask: Running task '%s'...\n", tsk->name);
                            tsk->callback();
                            tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                            tsk->_lastCheck = millis();
                            continue;
                        }
                    }
                    tsk->_lastCheck = millis();
                }
                connectedMS = millis()-beginConnected;
                Serial.printf("Network: WiFi connection: %d sec\n", connectedMS/1000);
            }
            continue;
        }
        // empty loop, use it to reset all counters
        beginConnected = -1;
        beginIdle = -1;
    }
    vTaskDelete(NULL); // never return
}

bool NetworkHandler() {
    provisioned = (bool)NVS.getInt("provisioned"); // initial value load

    xTaskCreate(NetworkHandlerTask, "nEt", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);

    if ( provisioned ) { return true; }
    return false;
}
