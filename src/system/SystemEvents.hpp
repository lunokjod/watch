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
    SYSTEM_EVENT_READY,       // boot end
    SYSTEM_EVENT_TICK,        // timed, meanwhile the screen is on
    SYSTEM_EVENT_STOP,        // when the system is power down
    SYSTEM_EVENT_WAKE,        //  received when the system is wake up
    SYSTEM_EVENT_LIGHTSLEEP,  // received when the system is begin sleep

    PMU_EVENT_BATT_CHARGING,  // notify when the battery is charging
    PMU_EVENT_BATT_FULL,      // the batt is full
    PMU_EVENT_BATT_ACTIVE,    // the battery becomes active
    PMU_EVENT_BATT_FREE,      // battery freed
    PMU_EVENT_BATT_PRESENT,   // have battery? 
    PMU_EVENT_BATT_REMOVED,   // ups, battery removed!
    PMU_EVENT_BATT_TEMP_LOW,  // Help! battery can be damaged!
    PMU_EVENT_BATT_TEMP_HIGH, // o_Ou
    PMU_EVENT_BATT_NOTFOUND,  // no battery
    PMU_EVENT_BATT_PC,        // battery load percent

    PMU_EVENT_POWER,          // usb connected
    PMU_EVENT_NOPOWER,        // no usb
    PMU_EVENT_PEK_SHORT,      // reserved by system to sleep/wake
    PMU_EVENT_PEK_LONG,       // reserved by system to launch ordered powerdown
    PMU_EVENT_TEMPERATURE,    // battery temperature

    BMP_EVENT_MOVE,             // movement detected
    BMA_EVENT_STEPCOUNTER,      // step detected
    BMA_EVENT_ACTIVITY,         // activity
    BMA_EVENT_NOMOTION,         // @TODO why interrupt never received? :(
    BMA_EVENT_TEMP,             // device temperature
    BMA_EVENT_DIRECTION         // pose/orientation
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
        std::function<bool ()> callback = nullptr; // lambda power
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
void DoSleep();
void SaveDataBeforeShutdown();

#endif
