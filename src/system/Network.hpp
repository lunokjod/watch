#ifndef __LUNOKIOT__ESP32_NETWORK__SUPPPORT___
#define __LUNOKIOT__ESP32_NETWORK__SUPPPORT___

#include <functional>

// notify to DoSleep they cannot poweroff the ESP
extern bool networkActivity;
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

void BLESetupHooks();
void StartBLE();
void StopBLE();
void BLEKickAllPeers();

#endif
