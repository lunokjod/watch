#include "lunokiot_config.hpp"
#include <Arduino.h>
#include <LilyGoWatch.h>
#include <Ticker.h>
#include "AXP202.hpp"
#include "../SystemEvents.hpp"
#include "../../app/LogView.hpp"
//#include "../Datasources/kvo.hpp"

bool PMUCounterEnabled=false;
Ticker PMUCounter;
extern TTGOClass *ttgo;
float PMUBattDischarge=0;

static void PMUMonitorEventCallback(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // http://www.x-powers.com/en.php/Info/product_detail/article_id/30
    PMUBattDischarge = ttgo->power->getBattDischargeCurrent();
    //lEvLog("PMU: %f mAh from last %d seconds\n", PMUBattDischarge,LUNOKIOT_PMU_MONITOR);
}

void PMUMonitorCallback() { // freeRTOS discourages process on callback due priority and recomends a queue :)
    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, PMU_EVENT_REPORT, nullptr, 0, LUNOKIOT_EVENT_FAST_TIME_TICKS);
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