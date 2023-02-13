#include "lunokiot_config.hpp"
#include <Arduino.h>
#include <LilyGoWatch.h>
#include <Ticker.h>
#include "AXP202.hpp"
#include "../SystemEvents.hpp"
#include "../../app/LogView.hpp"
//#include "../Datasources/kvo.hpp"
extern SemaphoreHandle_t I2cMutex;

bool PMUCounterEnabled=false;
Ticker PMUCounter;
extern TTGOClass *ttgo;
float PMUBattDischarge=0;
float batteryRemainingTimeHours=0.0;
float batteryTotalTimeHours = 0.0;

static void PMUMonitorEventCallback(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // http://www.x-powers.com/en.php/Info/product_detail/article_id/30

    // -1 is no battery
    if ( batteryPercent < 1 ) { return; } // cannot calculate without battery or full discharge
    BaseType_t done = xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
    if (pdTRUE != done) { return; }
    PMUBattDischarge = ttgo->power->getBattDischargeCurrent();
    xSemaphoreGive(I2cMutex);

    //int chargeCurrent = ttgo->power->getChargeControlCur();
    float battValue0to1 = float(batteryPercent/100.0);
    batteryTotalTimeHours = float(batteryCapacitymAh/PMUBattDischarge);
    if ( PMUBattDischarge > 0.0 ) {
        batteryRemainingTimeHours = float(batteryTotalTimeHours*battValue0to1);
        //lEvLog("PMU: Battery(%dmAh) Total time: %.2f Remaining time: %.2f\n", batteryCapacitymAh, batteryTotalTimeHours, batteryRemainingTimeHours);
    } else {
        batteryRemainingTimeHours=0.0;
    }
}

void PMUMonitorCallback() { // freeRTOS discourages process on callback due priority and recomends a queue :)
    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_REPORT, nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
}

static void PMUMonitorON(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    /*
    ttgo->power->EnableCoulombcounter();
    ttgo->power->ClearCoulombcounter();
    */
    if ( false == PMUCounterEnabled ) {
        lEvLog("PMU Monitor enabled\n");
        PMUCounter.attach(LUNOKIOT_PMU_MONITOR,PMUMonitorCallback);
        PMUCounterEnabled=true;
    }
}
static void PMUMonitorOFF(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    if ( PMUCounterEnabled ) {
        lEvLog("PMU Monitor disabled\n");
        PMUCounter.detach();
        PMUCounterEnabled=false;
    }
    //ttgo->power->DisableCoulombcounter();
    //ttgo->power->ClearCoulombcounter();
}

void StartPMUMonitor() {
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_READY, PMUMonitorON, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_WAKE, PMUMonitorON, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_LIGHTSLEEP, PMUMonitorOFF, nullptr, NULL);
    
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_REPORT, PMUMonitorEventCallback, nullptr,NULL);
}