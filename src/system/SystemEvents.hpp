#ifndef __LUNOKIOT__SYSTEM_EVENTS__
#define __LUNOKIOT__SYSTEM_EVENTS__


#include <Arduino.h>
#include <esp_event_base.h>
#include <list>
#include <functional>

ESP_EVENT_DECLARE_BASE(SYSTEM_EVENTS);

enum {
    SYSTEM_EVENT_READY,
    SYSTEM_EVENT_TICK,
    SYSTEM_EVENT_STOP,
    SYSTEM_EVENT_LIGHTSLEEP,
    SYSTEM_EVENT_SLEEP,

    PMU_EVENT_BATT_CHARGING,
    PMU_EVENT_BATT_FULL,
    PMU_EVENT_BATT_ACTIVE,
    PMU_EVENT_BATT_FREE,
    PMU_EVENT_BATT_PRESENT,
    PMU_EVENT_BATT_REMOVED,
    PMU_EVENT_BATT_TEMP_LOW,
    PMU_EVENT_BATT_TEMP_HIGH,
    PMU_EVENT_BATT_NOTFOUND,
    PMU_EVENT_BATT_PC,

    PMU_EVENT_POWER,
    PMU_EVENT_NOPOWER,
    PMU_EVENT_PEK_SHORT,
    PMU_EVENT_PEK_LONG,
    PMU_EVENT_TEMPERATURE,

    BMP_EVENT_MOVE,
    BMA_EVENT_STEPCOUNTER,
    BMA_EVENT_ACTIVITY,
    BMA_EVENT_NOMOTION,
    BMA_EVENT_TEMP,
    BMA_EVENT_DIRECTION
};


class NetworkTaskDescriptor {
    public:
        char *name = nullptr; // task description
        unsigned long everyTimeMS = -1; // lapse in millis
        unsigned long _nextTrigger = -1; // first launch in next time window or 0 for inmediate
        unsigned long _lastCheck = -1;  // internal record
        void * payload = nullptr;
        std::function<void ()> callback = nullptr;
};
bool RemoveNetworkTask(NetworkTaskDescriptor *oldTsk);
bool AddNetworkTask(NetworkTaskDescriptor *nuTsk);

void AXPIntHandler();
void BMPIntHandler();
void RTCIntHandler();
void SystemEventsStart();
void SystemEventBootEnd();
void ScreenSleep();
void ScreenWake();
void DoSleep();
void TakeBMPSample();
bool NetworkHandler();
void SaveDataBeforeShutdown();
#endif
