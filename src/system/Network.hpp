#ifndef __LUNOKIOT__ESP32_NETWORK__SUPPPORT___
#define __LUNOKIOT__ESP32_NETWORK__SUPPPORT___

#include <functional>
#include <list>
#include <NimBLEDevice.h>
#include "../lunokiot_config.hpp"

// notify to DoSleep they cannot poweroff the ESP
extern bool networkActivity;
/*
 * Network timed callbacks
 */
class NetworkTaskDescriptor {
    public:
        bool enabled=true;
        const char *name = nullptr; // task description
        unsigned long everyTimeMS = -1; // lapse in millis
        unsigned long _nextTrigger = -1; // -1 means first launch in next time window or 0 for inmediate
        unsigned long _lastCheck = -1;  // reserved
        uint32_t desiredStack = LUNOKIOT_TASK_STACK_SIZE;
        void * payload = nullptr;   // @TODO this may be useless
        std::function<bool ()> callback = nullptr; // lambda power
};

// Install the network loop task scheduler
bool NetworkHandler();
// Remove desired task from network pooling
bool RemoveNetworkTask(NetworkTaskDescriptor *oldTsk);
// Add task to the network loop
bool AddNetworkTask(NetworkTaskDescriptor *nuTsk);

#endif
