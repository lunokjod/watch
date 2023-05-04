//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//
// LunokWatch is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later 
// version.
//
// LunokWatch is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
// details.
//
// You should have received a copy of the GNU General Public License along with 
// LunokWatch. If not, see <https://www.gnu.org/licenses/>. 
//

#ifndef ___LUNOKIOT__BLE__HANDLING___
#define ___LUNOKIOT__BLE__HANDLING___

// https://computingforgeeks.com/connect-to-bluetooth-device-from-linux-terminal/
#include <NimBLEDevice.h>
#include "../Datasources/kvo.hpp"
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

const uint8_t BLECORE = CONFIG_BT_NIMBLE_PINNED_TO_CORE;

enum BLEZoneLocations {
    UNKNOWN=0,
    // general sites (add do you want)
    HOME,
    WORK,
    REST,
    MARK
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
        bool dbSync=false;
        int locationGroup=BLEZoneLocations::UNKNOWN;
        ~lBLEDevice();
};

extern std::list <lBLEDevice*>BLEKnowDevices;

extern const char * BLEZoneLocationsHumanReadable[];

extern BLEZoneLocations BLELocationZone;

extern SemaphoreHandle_t BLEKnowDevicesSemaphore;
extern std::list <lBLEDevice*>BLEKnowDevices;

//extern volatile bool bleWaitStop; // internal flag to wait BLEStop task
//extern bool bleEnabled; // is the service on?
//extern volatile bool bleBeingUsed;   // downloading something
//extern volatile bool bleAdvertising;

class LBLEUARTCallbacks;
class LBLEServerCallbacks;

class LoTBLE {
    friend LBLEUARTCallbacks;
    friend LBLEServerCallbacks;
    private:
        const float GraceTimeSeconds = 9;
        SemaphoreHandle_t taskLock = xSemaphoreCreateMutex();
        void _BLELoopTask();
        TaskHandle_t BLELoopTaskHandler=NULL;
        const unsigned long UserSettingsCheckMS=10*1000;
        const unsigned long LocationCheckMS=80*1000;

        void _TryLaunchTask();
        void _TryStopTask();
        EventKVO * BLEWStartEvent = nullptr; 
        EventKVO * BLEWStopEvent = nullptr; 
        bool enabled=false;
        bool running=false;
        BLEService *pServiceUART = nullptr;
        NimBLECharacteristic * pRxCharacteristic = nullptr;
        NimBLECharacteristic * pTxCharacteristic = nullptr;

        EventKVO * battPercentPublish = nullptr; 
        BLEService *pServiceBattery = nullptr;
        NimBLECharacteristic * BatteryCharacteristic = nullptr;
        // gadgetbridge integration
        // used to nofity when new message is ready for parsing
        bool BLEGadgetbridgeCommandPending = false;
        bool BLEBangleJSCommandPending = false;
        char *gadgetBridgeBuffer=nullptr;
        size_t gadgetBridgeBufferOffset=0;
        SemaphoreHandle_t BLEGadgetbridge = xSemaphoreCreateMutex(); // locked during UP/DOWN BLE service
    public:
        char BTName[15] = { 0 }; // buffer for build the name like: "lunokIoT_69fa"
        LoTBLE();
        ~LoTBLE();
        const float GraceTime() { return GraceTimeSeconds; }
        void Enable();
        void Disable();
        bool IsEnabled();
        bool IsDisabled() { return (!IsEnabled()); }
        bool InUse();
        bool IsScanning();
        void StartAdvertising();
        void StopAdvertising();
        bool IsAdvertising();
        size_t Clients();
        bool BLESendUART(const char * data);
};

#endif
