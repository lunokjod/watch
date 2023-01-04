#ifndef __LUNOKIOT__BATTERY_APP__
#define __LUNOKIOT__BATTERY_APP__
/*
 * This app explains the KVO funcionality (callback when bus event received)
 */

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"

#include "../system/Datasources/kvo.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"
#include "../system/Application.hpp"
#include "../static/img_back_32.xbm" // back buton image
#include "../system/SystemEvents.hpp"   // the event bus

class BatteryApplication: public TemplateApplication {
    private:
        bool dirtyFrame = true;
        unsigned long nextRedraw=0;
        unsigned long pollTempRedraw=0;
        unsigned long scaleTempRedraw=0;
        bool normalRange=true;
        const int SCALE=4;
        uint16_t chargingAnimOffset=0;
        bool charging=false;
        const int16_t battHeight = 133;
        GraphWidget *batteryTempMonitor=nullptr;
    public:
        const char *AppName() override { return "Battery"; };
        // PMU_EVENT_BATT_PC
        EventKVO * BattPCEvent = nullptr;
        EventKVO * BattChargeEvent = nullptr;
        EventKVO * BattFullEvent = nullptr;

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

        BatteryApplication();
        ~BatteryApplication();
        bool Tick();
};

#endif
