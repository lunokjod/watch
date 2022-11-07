#ifndef __LUNOKIOT__ESP32_NETWORK__SUPPPORT___
#define __LUNOKIOT__ESP32_NETWORK__SUPPPORT___

#include <functional>
#include <list>
#include <NimBLEDevice.h>

// notify to DoSleep they cannot poweroff the ESP
extern bool networkActivity;
/*
 * Network timed callbacks
 */
class NetworkTaskDescriptor {
    public:
        bool enabled=true;
        char *name = nullptr; // task description
        unsigned long everyTimeMS = -1; // lapse in millis
        unsigned long _nextTrigger = -1; // -1 means first launch in next time window or 0 for inmediate
        unsigned long _lastCheck = -1;  // reserved
        void * payload = nullptr;   // @TODO this may be useless
        std::function<bool ()> callback = nullptr; // lambda power
};

// represent BLE remote devices
class lBLEDevice {
    public:
        NimBLEAddress addr;
        char * devName = nullptr;
        unsigned long firstSeen = 0; // @TODO use time_t instead
        unsigned long lastSeen = 0;
        size_t seenCount = 0;
        int rssi = 0;
        ~lBLEDevice();
};

extern std::list <lBLEDevice*>BLEKnowDevices;

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
