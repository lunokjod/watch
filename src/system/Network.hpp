#ifndef __LUNOKIOT__ESP32_NETWORK__SUPPPORT___
#define __LUNOKIOT__ESP32_NETWORK__SUPPPORT___

#include <functional>
#include <list>
#include <NimBLEDevice.h>
#include "../lunokiot_config.hpp"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define BLE_CHARACTERISTIC_LUNOKIOT_VERSION        "95f0565b-b06c-4587-acc2-07f258b0f6f5"
#define BLE_CHARACTERISTIC_LUNOKIOT_BATTERY_TEMP   "4b3cce41-1d16-49f8-a8e3-04adfe41c745"
#define BLE_CHARACTERISTIC_LUNOKIOT_BMA_TEMP       "2c35cafc-6220-438c-82fc-ae131d532d12"

#define BLE_SERVICE_BATTERY           "180F"
#define BLE_CHARACTERISTIC_BATTERY    "2A19"
#define BLE_DESCRIPTOR_HUMAN_DESC     "2901"
#define BLE_DESCRIPTOR_TYPE           "2904"

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
        const char *name = nullptr; // task description
        unsigned long everyTimeMS = -1; // lapse in millis
        unsigned long _nextTrigger = -1; // -1 means first launch in next time window or 0 for inmediate
        unsigned long _lastCheck = -1;  // reserved
        uint32_t desiredStack = LUNOKIOT_TASK_STACK_SIZE;
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
