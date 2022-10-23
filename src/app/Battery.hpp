#ifndef __LUNOKIOT__BATTERY_APP__
#define __LUNOKIOT__BATTERY_APP__
/*
 * This app explains the KVO funcionality (callback when bus event received)
 */

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Datasources/kvo.hpp"

#include "../system/Application.hpp"
#include "../static/img_back_32.xbm" // back buton image
#include "Watchface.hpp"             // needed on LaunchApplication
#include "../system/SystemEvents.hpp"   // the event bus

class BatteryApplication: public LunokIoTApplication {
    private:
        bool dirtyFrame = true;
        unsigned long nextRedraw=0;
    public:
        // PMU_EVENT_BATT_PC
        EventKVO * BattPCEvent = nullptr;

        /* @TODO monitor other events
        PMU_EVENT_BATT_CHARGING,
        PMU_EVENT_BATT_FULL,
        PMU_EVENT_BATT_ACTIVE,
        PMU_EVENT_BATT_FREE,
        PMU_EVENT_BATT_PRESENT,
        PMU_EVENT_BATT_REMOVED,
        PMU_EVENT_BATT_TEMP_LOW,
        PMU_EVENT_BATT_TEMP_HIGH,
        PMU_EVENT_BATT_NOTFOUND,
        */

        ButtonImageXBMWidget * btnBack = nullptr;
        BatteryApplication();
        ~BatteryApplication();
        bool Tick();
};

#endif
