#ifndef __LUNOKIOT__SYSTEM_EVENTS__
#define __LUNOKIOT__SYSTEM_EVENTS__

#include <Arduino.h>
#include <esp_event_base.h>
#include <list>
#include <functional>

/*
 * Event loop for the system
 */
ESP_EVENT_DECLARE_BASE(SYSTEM_EVENTS);

// Events for system loop
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

/*
 * Network timed callbacks
 */
class NetworkTaskDescriptor {
    public:
        char *name = nullptr; // task description
        unsigned long everyTimeMS = -1; // lapse in millis
        unsigned long _nextTrigger = -1; // -1 means first launch in next time window or 0 for inmediate
        unsigned long _lastCheck = -1;  // reserved
        void * payload = nullptr;   // @TODO this may be useless
        std::function<void ()> callback = nullptr; // lambda power
};
// Install the network loop task scheduler
bool NetworkHandler();
// Remove desired task from network pooling
bool RemoveNetworkTask(NetworkTaskDescriptor *oldTsk);
// Add task to the network loop
bool AddNetworkTask(NetworkTaskDescriptor *nuTsk);

/*
 * Interrupt handler installer
 */
void AXPIntHandler();
void BMPIntHandler();
void RTCIntHandler();

// Force get samples from sensors
void TakeSamples();

// Build system loop
void SystemEventsStart();
// Announce the end of boot
void SystemEventBootEnd();

/*
 * Low power related functions
 */
void ScreenSleep();
void ScreenWake();
void DoSleep();
void SaveDataBeforeShutdown();

#endif
