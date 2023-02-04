#ifndef ___LUNOKIOT__BLE__HANDLING___
#define ___LUNOKIOT__BLE__HANDLING___

#include <NimBLEDevice.h>

/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/

/** NimBLE differences highlighted in comment blocks **/

/*******original********
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
***********************/

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

extern volatile bool bleServiceRunning;

void StartBLE();
void StopBLE();

void BLESetupHooks();
//void BLEKickAllPeers();


//extern volatile bool bleServiceRunning;
#endif
