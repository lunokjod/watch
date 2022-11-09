#ifndef __LUNOKIOT__ESP32_NETWORK__SUPPPORT___
#define __LUNOKIOT__ESP32_NETWORK__SUPPPORT___

#include <functional>
#include <list>
#include <NimBLEDevice.h>


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define BLE_CHARACTERISTIC_LUNOKIOT_VERSION   "ab84d0b6-4cd2-4f4e-ae59-9a81203205f7"
#define BLE_SERVICE_BATTERY           "180F"
#define BLE_CHARACTERISTIC_BATTERY    "2A19"
#define BLE_DESCRIPTOR_BATTERY        "2901"

#define BLE_SERVICE_LUNOKIOT   "ab84d0b6-4cd2-4f4e-ae59-9a81203205f7"
#define SERVICE_UART_UUID      "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

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
        int8_t txPower = 0;
        double distance = -1;
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
