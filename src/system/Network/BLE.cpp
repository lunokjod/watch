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

#include <functional>
#include <list>
#include <esp_task_wdt.h>
#include <Arduino.h>
#include <ArduinoNvs.h>

#include "../../lunokiot_config.hpp"

#include "BLE.hpp"
#include "../SystemEvents.hpp"
#include "../Datasources/kvo.hpp"

#include "../../app/Bluetooth.hpp"
#include "../Datasources/database.hpp"
#include "Gagetbridge.hpp"
#include <cstring>
#include <LilyGoWatch.h>
#include "../../lunokIoT.hpp"

extern TTGOClass *ttgo;

const char * BLEZoneLocationsHumanReadable[] = {
    "Unknown",
    "Home",
    "Work",
    "Rest",
    "Mark"
};

BLEZoneLocations BLELocationZone=BLEZoneLocations::UNKNOWN; // unknown by default

const esp_power_level_t defaultBLEPowerLevel = ESP_PWR_LVL_N3;
// monitors the wake and sleep of BLE
//EventKVO * BLEWStartEvent = nullptr; 
//EventKVO * BLEWLightSleepEvent = nullptr;
//EventKVO * BLEWStopEvent = nullptr;
// monitor the sensors

//EventKVO * battTempPublish = nullptr; 
//EventKVO * envTempPublish = nullptr; 
// service start/stop flag
//StaticTask_t BLETaskHandler;
//StackType_t BLEStack[LUNOKIOT_TASK_STACK_SIZE];
//SemaphoreHandle_t BLEUpDownStep = xSemaphoreCreateMutex(); // locked during UP/DOWN BLE service


//TaskHandle_t BLEStartHandler;
// monitor devices to publish on GATT
//NimBLECharacteristic * battCharacteristic = nullptr;
//NimBLECharacteristic * lBattTempCharacteristic = nullptr;
//NimBLECharacteristic * lBMATempCharacteristic = nullptr;
// nimble things
//BLEService *pLunokIoTService = nullptr;
//BLEService *pBattService = nullptr;

// list of know devices
SemaphoreHandle_t BLEKnowDevicesSemaphore = xSemaphoreCreateMutex();
std::list <lBLEDevice*>BLEKnowDevices;
uint32_t bleLocationScanCounter=0;

// http://www.espruino.com/Gadgetbridge
const size_t gadgetBridgeBufferSize=8*1024;

// Server callbacks
class LBLEServerCallbacks: public NimBLEServerCallbacks {
    private:
        LoTBLE *bleHandler=nullptr;
    public:
        LBLEServerCallbacks(LoTBLE *handler) : bleHandler(handler) {
            lNetLog("BLE: %p LBLEServerCallbacks %p\n",bleHandler,this);
        }

        void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
            lNetLog("BLE: %p LBLEServerCallbacks: Client connect \n",bleHandler,this);
            bleHandler->StopAdvertising();
            if ( nullptr != currentApplication ) {
                if ( 0 != strcmp(currentApplication->AppName(),"BLE pairing")) {
                    if ( ttgo->bl->isOn()) {
                        LaunchApplication(new BluetoothApplication());
                    }
                }
            }
        }
        void onDisconnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
            lNetLog("BLE: %p LBLEServerCallbacks: Client disconnect \n",bleHandler,this);
            //deviceConnected = false;
            bleHandler->StartAdvertising();
        }
        void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
            lNetLog("BLE: %p LBLEServerCallbacks: MTU changed to: %u\n",bleHandler,this,MTU);
        }
        uint32_t onPassKeyRequest() {
            uint32_t pass = BLEDevice::getSecurityPasskey();
            lNetLog("BLE: %p LBLEServerCallbacks: Password request: %04u\n",bleHandler,this,pass);
            return BLEDevice::getSecurityPasskey();
        }
        void onAuthenticationComplete(ble_gap_conn_desc* desc) {
            lNetLog("BLE: %p LBLEServerCallbacks: Authentication complete\n",bleHandler,this);
            if ( nullptr != currentApplication ) {
                if ( 0 == strcmp(currentApplication->AppName(),"BLE pairing")) {
                    if ( ttgo->bl->isOn()) {
                        LaunchWatchface();
                    }
                }
            }
        }
        bool onConfirmPIN(uint32_t pin) {
            uint32_t pass = BLEDevice::getSecurityPasskey();
            lNetLog("BLE: %p LBLEServerCallbacks: PIN confirmation: %04u ",bleHandler,this,pass);
            if ( pin == BLEDevice::getSecurityPasskey() ) {
                lLog("OK\n");
                return true;
            }
            lLog("FAIL\n");
            return false;
        }
};

// advertiser callbacks
class LBLEAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice* advertisedDevice) {
        bool alreadyKnown = false;
        int FinalZone=-1;
        if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  {
            for (auto const& dev : BLEKnowDevices) {
                if ( dev->addr == advertisedDevice->getAddress() ) {
                    char *finalName = (char*)"";
                    if ( nullptr != dev->devName ) { finalName = dev->devName; }
                    lNetLog("BLE: Know dev: '%s'(%s) (from: %d secs) last seen: %d secs (%d times) zone: %d\n",
                                            finalName,dev->addr.toString().c_str(),
                                            ((millis()-dev->firstSeen)/1000),
                                            ((millis()-dev->lastSeen)/1000),
                                            dev->seenCount,dev->locationGroup);
                    dev->lastSeen = millis();
                    dev->seenCount++;
                    if ( advertisedDevice->haveRSSI() ) {
                        dev->rssi = advertisedDevice->getRSSI();
                    }
                    if ( advertisedDevice->haveTXPower() ) {
                        dev->txPower = advertisedDevice->getTXPower();
                    }
                    // https://stackoverflow.com/questions/20416218/understanding-ibeacon-distancing/20434019#20434019
                    if (( advertisedDevice->haveRSSI()) && (advertisedDevice->haveTXPower())) {
                        // Distance = 10 ^ ((Measured Power -RSSI)/(10 * N))
                        dev->distance = 10 ^((dev->txPower-dev->rssi)/(10* 2));
                        //lNetLog("BLE: DEBUG Device DISTANCE: %f\n",dev->distance);
                    }
                    if ( BLEZoneLocations::UNKNOWN != dev->locationGroup ) { FinalZone=dev->locationGroup; }
                    alreadyKnown=true;
                    SqlUpdateBluetoothDevice(dev->addr.toString().c_str(),dev->distance,dev->locationGroup);
                    break;
                }
            }
            xSemaphoreGive( BLEKnowDevicesSemaphore );
        }
        if ( alreadyKnown ) {
            if ( -1 == FinalZone ) { // not found
                //lNetLog("BLE zone not found\n");
                BLELocationZone = BLEZoneLocations::UNKNOWN;
            } else { // found zone
                //lNetLog("BLE zone found: %d '%s'\n",FinalZone,BLEZoneLocationsHumanReadable[FinalZone]);
                BLELocationZone = (BLEZoneLocations)FinalZone;
            }
            return;
        } // nu device
        lBLEDevice * newDev = new lBLEDevice();
        newDev->addr =  advertisedDevice->getAddress();
        newDev->devName = (char *)ps_malloc(advertisedDevice->getName().length()+1);
        sprintf(newDev->devName,"%s",advertisedDevice->getName().c_str());
        lNetLog("BLE: New dev: '%s'(%s)\n",newDev->devName, newDev->addr.toString().c_str());
        if ( advertisedDevice->haveRSSI() ) {
            newDev->rssi = advertisedDevice->getRSSI();
        }
        if ( advertisedDevice->haveTXPower() ) {
            newDev->txPower = advertisedDevice->getTXPower();
        }
        // https://stackoverflow.com/questions/20416218/understanding-ibeacon-distancing/20434019#20434019
        if (( advertisedDevice->haveRSSI()) && (advertisedDevice->haveTXPower())) {
            // Distance = 10 ^ ((Measured Power -RSSI)/(10 * N))
            newDev->distance = 10 ^((newDev->txPower-newDev->rssi)/(10* 2));
            //lNetLog("BLE: DEBUG Device DISTANCE: %f\n",newDev->distance);
        }
        newDev->locationGroup=BLEZoneLocations::UNKNOWN;
        newDev->firstSeen = millis();
        newDev->lastSeen = newDev->firstSeen;
        if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  {
            BLEKnowDevices.push_back(newDev);
            //DONT NEEDED (see BLEDev destroy) SqlAddBluetoothDevice(newDev->addr.toString().c_str(), newDev->distance, newDev->locationGroup);
            xSemaphoreGive( BLEKnowDevicesSemaphore );
        }
        //lNetLog("BLE: seen devices: %d\n",BLEKnowDevices.size());
    }
};

class LBLEUARTCallbacks: public NimBLECharacteristicCallbacks {
    
    bool gadgetbridgeCommandInProgress=false;
    bool bangleCommandInProgress=false;

    //void onRead(NimBLECharacteristic* pCharacteristic) {
    //    lNetLog("BLE: UART: onREAD\n");
    //}
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        const char * receivedData = rxValue.c_str();
        // check for GADGETBRIDGE commands
        const char GadgetbridgeCommandEND[] = ")\n";
        const char GadgetbridgeCommandBEGIN[] = "\x10GB(";
        LoTBLE * myHost = LoT().GetBLE();

        if ( 0 == strncmp(receivedData,GadgetbridgeCommandBEGIN,strlen(GadgetbridgeCommandBEGIN))) {
            lNetLog("BLE: UART: Begin Gadgetbridge GB command\n");
            //bleBeingUsed=true;
            gadgetbridgeCommandInProgress=true;
            if ( nullptr != myHost->gadgetBridgeBuffer ) { free(myHost->gadgetBridgeBuffer); myHost->gadgetBridgeBuffer=nullptr; }
            myHost->gadgetBridgeBuffer = (char*)ps_malloc(gadgetBridgeBufferSize);
            sprintf(myHost->gadgetBridgeBuffer,"%s",receivedData+strlen(GadgetbridgeCommandBEGIN)); // ignore cmd string
            myHost->gadgetBridgeBufferOffset=strlen(myHost->gadgetBridgeBuffer);
            return;
        } else if ( gadgetbridgeCommandInProgress ) {
            if (nullptr == myHost->gadgetBridgeBuffer) {
                gadgetbridgeCommandInProgress=false;
                lNetLog("BLE: UART: memory ERROR! (no buffer space for gadgetbridge)\n");
                myHost->gadgetBridgeBufferOffset=0;
                return;
            }
            lNetLog("BLE: UART: Continue Gadgetbridge GB command\n");
            // copy to buffer
            for (int i = 0; i < rxValue.length(); i++) {
                myHost->gadgetBridgeBuffer[myHost->gadgetBridgeBufferOffset]=rxValue[i];
                myHost->gadgetBridgeBufferOffset++;

            }
            // check if is the end
            const char *toLast=receivedData;
            toLast+=strlen(receivedData)-strlen(GadgetbridgeCommandEND);
            if ( 0 == strcmp(GadgetbridgeCommandEND,toLast)) {
                myHost->gadgetBridgeBuffer[myHost->gadgetBridgeBufferOffset-2]=0; // correct eol
                gadgetbridgeCommandInProgress=false;
                lNetLog("BLE: UART: End Gadgetbridge GB command\n");
                // Parse outside
                if( xSemaphoreTake( myHost->BLEGadgetbridge, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) == pdTRUE )  {
                    myHost->BLEGadgetbridgeCommandPending=true;
                    myHost->gadgetBridgeBufferOffset=0;
                    // data avaliable on gadgetBridgeBuffer
                    xSemaphoreGive( myHost->BLEGadgetbridge );
                }
                //bleBeingUsed=false;
            }
            return;
        }
        // check for BANGLEJS extra commands
        // setTime
        const char BangleJSCommandBEGIN[] = "\x10setTime(";
        const char BangleJSCommandEND[] = "\n";

        if ( 0 == strncmp(receivedData,BangleJSCommandBEGIN,strlen(BangleJSCommandBEGIN))) {
            lNetLog("BLE: UART: Begin BangleJS command\n");
            //bleBeingUsed=true;
            bangleCommandInProgress=true;
            if ( nullptr != myHost->gadgetBridgeBuffer ) { free(myHost->gadgetBridgeBuffer); myHost->gadgetBridgeBuffer=nullptr; }
            myHost->gadgetBridgeBuffer = (char*)ps_malloc(gadgetBridgeBufferSize);
            sprintf(myHost->gadgetBridgeBuffer,"%s",receivedData+1); // bypass \x10
            myHost->gadgetBridgeBufferOffset=strlen(myHost->gadgetBridgeBuffer);
            return;
        } else if ( bangleCommandInProgress ) {
            if (nullptr == myHost->gadgetBridgeBuffer) {
                bangleCommandInProgress=false;
                lNetLog("BLE: UART: memory ERROR! (no buffer size for bangle command)\n");
                myHost->gadgetBridgeBufferOffset=0;
                //bleBeingUsed=false;
                return;
            }
            lNetLog("BLE: UART: Continue BangleJS command\n");
            // copy to buffer
            for (int i = 0; i < rxValue.length(); i++) {
                myHost->gadgetBridgeBuffer[myHost->gadgetBridgeBufferOffset]=rxValue[i];
                myHost->gadgetBridgeBufferOffset++;
            }
            // check if is the end
            const char *toLast=receivedData;
            toLast+=strlen(receivedData)-strlen(BangleJSCommandEND);
            if ( 0 == strcmp(BangleJSCommandEND,toLast)) {
                myHost->gadgetBridgeBuffer[myHost->gadgetBridgeBufferOffset-1]=0; // correct eol
                bangleCommandInProgress=false;
                //bleBeingUsed=false;
                lNetLog("BLE: UART: End BangleJS command\n");

                // Parse outside
                LoTBLE * myHost = LoT().GetBLE();
                if( xSemaphoreTake( myHost->BLEGadgetbridge, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) == pdTRUE )  {
                    myHost->BLEBangleJSCommandPending=true;
                    myHost->gadgetBridgeBufferOffset=0;
                    // data avaliable on gadgetBridgeBuffer
                    xSemaphoreGive( myHost->BLEGadgetbridge );
                }
            }
            return;
        }

        // no know howto parse... show as debug
        lNetLog("BLE: UART Received Value: ");
        for (int i = 0; i < rxValue.length(); i++) { //ignore \0
            lLog("%c(0x%02x) ",rxValue[i],rxValue[i]);
        }
        lLog("\n");
    }
};

bool LoTBLE::IsScanning() {
    NimBLEScan * scan = NimBLEDevice::getScan();
    if ( nullptr == scan ) { return false; }
    return scan->isScanning();
}

bool LoTBLE::IsAdvertising() {
    NimBLEAdvertising * adv = NimBLEDevice::getAdvertising();
    if ( nullptr == adv ) { return false; }
    return adv->isAdvertising();
}

void LoTBLE::StartAdvertising() {
    NimBLEAdvertising * adv = NimBLEDevice::getAdvertising();
    if ( nullptr == adv ) { return; }
    if ( false == adv->isAdvertising() ) {
        lNetLog("BLE: %p Start Advetising...\n",this);
        adv->start();
    }
}

void LoTBLE::StopAdvertising() {
    NimBLEAdvertising * adv = NimBLEDevice::getAdvertising();
    if ( nullptr == adv ) { return; }
    if ( adv->isAdvertising() ) {
        lNetLog("BLE: %p Stop Advetising...\n",this);
        adv->stop();
    }
}

size_t LoTBLE::Clients() {
    NimBLEServer * pServer = NimBLEDevice::getServer();
    if ( nullptr == pServer ) { return 0; }
    std::vector<uint16_t>  clients = pServer->getPeerDevices();
    return clients.size();

}

void SaveBLEDevicesToDB() {
    if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) == pdTRUE )  {
        BLEKnowDevices.remove_if([](lBLEDevice *dev){
            // if location is set, save to db
            if ( BLEZoneLocations::UNKNOWN !=  dev->locationGroup ) {
                SqlAddBluetoothDevice(dev->addr.toString().c_str(),dev->distance,dev->locationGroup);
                dev->dbSync=true;
            }
            lNetLog("BLE: Free know device: '%s'\n", dev->addr.toString().c_str());
            return true;
        });
        xSemaphoreGive( BLEKnowDevicesSemaphore );
    }
}

void LoTBLE::_BLELoopTask() {
    esp_task_wdt_delete(NULL);
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    running=true;
    xSemaphoreGive( taskLock );

    lNetLog("BLE: %p Init with name: '%s'...\n",this,BTName);
    BLEDevice::init(std::string(BTName)); // hate strings

    lNetLog("BLE: %p Apply security....\n",this);
    BLEDevice::setSecurityAuth(true,true,true);
    uint32_t generatedPin=random(0,999999);
    lNetLog("BLE: %p generated PIN: %06d\n",this,generatedPin);
    BLEDevice::setSecurityPasskey(generatedPin);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    NimBLEDevice::setPower(defaultBLEPowerLevel,ESP_BLE_PWR_TYPE_DEFAULT);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9,ESP_BLE_PWR_TYPE_ADV);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9,ESP_BLE_PWR_TYPE_SCAN);
    // create GATT the server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new LBLEServerCallbacks(this),true); // destroy on finish
    lNetLog("BLE: %p Server started\n",this);

    if ( nullptr == pServiceUART ) { pServiceUART = pServer->createService(SERVICE_UART_UUID); }

    // Create the BLE Service UART
    // UART data come here
    if ( nullptr == pTxCharacteristic ) { 
        pTxCharacteristic = pServiceUART->createCharacteristic( CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY );
        pTxCharacteristic->setCallbacks(new LBLEUARTCallbacks());
    }
    if ( nullptr == pRxCharacteristic ) { 
        pRxCharacteristic = pServiceUART->createCharacteristic(
                    CHARACTERISTIC_UUID_RX,NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN);
        pRxCharacteristic->setCallbacks(new LBLEUARTCallbacks());
    }
    pServiceUART->start();

    // battery services
    // https://circuitdigest.com/microcontroller-projects/esp32-ble-server-how-to-use-gatt-services-for-battery-level-indication
    pServiceBattery = pServer->createService(BLE_SERVICE_BATTERY);
    BatteryCharacteristic = pServiceBattery->createCharacteristic(BLE_CHARACTERISTIC_BATTERY,
                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN | NIMBLE_PROPERTY::NOTIFY );
    uint8_t level = 0;
    if ( -1 != batteryPercent ) { level = batteryPercent; }
    BatteryCharacteristic->setValue(level);    
    // capture sensor events and push to BLE
    if ( nullptr == battPercentPublish ) {
        battPercentPublish = new EventKVO([&](){
            if ( nullptr != BatteryCharacteristic ) {
                uint8_t level = 0;
                if ( -1 != batteryPercent ) { level = batteryPercent; }
                lNetLog("BLE: %p Notify Battery: %d%%\n",this,level); // notify to log
                BatteryCharacteristic->setValue(level);
                BatteryCharacteristic->notify();
            }
        },PMU_EVENT_BATT_PC);
    }
    pServiceBattery->start();
    
    pServer->start();

    BLEScan * pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new LBLEAdvertisedDeviceCallbacks(), false);



    StartAdvertising();
    unsigned long nextUserSettingCheckMS=0;
    unsigned long bleAdvertisingTimeoutMS=millis()+12000; // a little delay from first scan

    while ( running ) {
        TickType_t nextCheck = xTaskGetTickCount();
        xTaskDelayUntil( &nextCheck, (500 / portTICK_PERIOD_MS) );

        if ( millis() > nextUserSettingCheckMS ) {
            // check if user wants me
            bool enabled = NVS.getInt("BLEEnabled");
            if ( false == enabled ) { 
                lNetLog("BLE: %p User settings don't agree\n",this);
                running=false;
            }
            nextUserSettingCheckMS = millis()+UserSettingsCheckMS;
        }
        if ( millis() > bleAdvertisingTimeoutMS ) {
            StopAdvertising();
            lNetLog("BLE: %p Perform Location scan...\n");
            // clean old BLE peers here
            if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  {
                size_t b4Devs = BLEKnowDevices.size();
                BLEKnowDevices.remove_if([](lBLEDevice *dev) {
                    unsigned long seconds = (millis()-dev->lastSeen);
                    if ( seconds > LUNOKIOT_BLE_SCAN_LOCATION_INTERVAL ) {
                        // if location is set, save to db
                        if ( BLEZoneLocations::UNKNOWN != dev->locationGroup ) {
                            esp_task_wdt_reset();
                            SqlAddBluetoothDevice(dev->addr.toString().c_str(),dev->distance,dev->locationGroup);
                            dev->dbSync=true;
                        }
                        char *finalName = (char*)"";
                        if ( nullptr != dev->devName ) { finalName = dev->devName; }
                        lNetLog("BLE: Removed expired seen device: '%s' %s\n",finalName,dev->addr.toString().c_str());
                        delete dev;
                        dev=nullptr;
                        return true;
                    }
                    return false;
                });
                esp_task_wdt_reset();
                xSemaphoreGive( BLEKnowDevicesSemaphore );
            }
            NimBLEScan *pBLEScan = BLEDevice::getScan();
            if (pBLEScan->isScanning()) {
                pBLEScan->stop();
                lNetLog("BLE: Location recognition: Last Scan %d stopped!\n",bleLocationScanCounter-1);
                delay(100);
            }
            if (systemSleep) { continue; }

            Disable();
            lNetLog("BLE: Location recognition: Pasive scan %d begin\n", bleLocationScanCounter);
            SqlLog("BLE: pasive scan");
            //pBLEScan->clearResults();
            pBLEScan->setMaxResults(3);
            pBLEScan->setInterval(1000);
            pBLEScan->setWindow(999);  // less or equal setInterval value
            pBLEScan->setDuplicateFilter(true);
            //delay(50);
            BLEScanResults foundDevices = pBLEScan->start(2);
            lNetLog("BLE: Devices found: %d\n",foundDevices.getCount());
            //pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
            lNetLog("BLE: Refreshing devices from database...\n");
            if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  {
                for (auto const& dev : BLEKnowDevices) {
                    if ( false == dev->dbSync ) { SqlRefreshBluetoothDevice(dev->addr.toString().c_str()); }
                }
                xSemaphoreGive( BLEKnowDevicesSemaphore );
            }
            lNetLog("BLE: Location recognition: Pasive scan %d end\n", bleLocationScanCounter);
            bleLocationScanCounter++;
            if ( 0 == Clients() ) { StartAdvertising(); }
            Enable();
            bleAdvertisingTimeoutMS=millis()+LocationCheckMS;
        }
        // parse gadgetbridge command
        char * currentGadgetBridgeBufferPtr = nullptr;
        char * currentBangleJSBridgeBufferPtr = nullptr;
        if( xSemaphoreTake( BLEGadgetbridge, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) == pdTRUE )  {
            // bangleJS
            if ( BLEBangleJSCommandPending ) {
                currentBangleJSBridgeBufferPtr=gadgetBridgeBuffer;
                gadgetBridgeBuffer=nullptr; // this can seems leak, (but is a capture out of buffer) see parser code down here
                BLEBangleJSCommandPending = false;
            }
            // gadgetbridge
            if ( BLEGadgetbridgeCommandPending ) {
                currentGadgetBridgeBufferPtr=gadgetBridgeBuffer;
                gadgetBridgeBuffer=nullptr; // this can seems leak, (but is a capture out of buffer) see parser code down here
                BLEGadgetbridgeCommandPending = false;
            }
            // data avaliable on gadgetBridgeBuffer
            xSemaphoreGive( BLEGadgetbridge );
        }
        if ( nullptr != currentBangleJSBridgeBufferPtr ) {
                bool parsed = ParseBangleJSMessage(currentBangleJSBridgeBufferPtr);
                if ( false == parsed ) {
                    lNetLog("BLE: %p Unknown BangleJS message: '%s'\n",this,currentBangleJSBridgeBufferPtr);
                }
                free(currentBangleJSBridgeBufferPtr); // here is freed original gadgetBridgeBuffer (closing the leak)
                currentBangleJSBridgeBufferPtr=nullptr; // dontcare
        }
        if ( nullptr != currentGadgetBridgeBufferPtr ) {
                bool parsed = ParseGadgetBridgeMessage(currentGadgetBridgeBufferPtr);
                if ( false == parsed ) {
                    lNetLog("BLE: %p Unknown Gadgetbridge message: '%s'\n",this,currentGadgetBridgeBufferPtr);
                }
                free(currentGadgetBridgeBufferPtr); // here is freed original gadgetBridgeBuffer (closing the leak)
                currentGadgetBridgeBufferPtr=nullptr; // dontcare
        }
    }
    StopAdvertising();
    SaveBLEDevicesToDB();
    // Stop BLE
    lNetLog("BLE: %p Deinit...\n",this);
    BLEDevice::deinit(true);
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    running=false;
    xSemaphoreGive( taskLock );
}

void LoTBLE::_TryStopTask() {
    // if user whants BLE, don't do nothing
    bool enabled = NVS.getInt("BLEEnabled");
    if ( true == enabled ) {  return; }
    // stop the task
    if ( NULL != BLELoopTaskHandler ) {
        xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        running=false;
        xSemaphoreGive( taskLock );
        while ( NULL != BLELoopTaskHandler ) {
            lNetLog("BLE: %p Waiting for task ends...\n");
            TickType_t nextCheck = xTaskGetTickCount();
            xTaskDelayUntil( &nextCheck, (1000 / portTICK_PERIOD_MS) );
        }
    }
}

void LoTBLE::_TryLaunchTask() {
    // check if user wants me
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    enabled = NVS.getInt("BLEEnabled");
    xSemaphoreGive( taskLock );

    if ( false == enabled ) { 
        lNetLog("BLE: %p User settings don't agree\n",this);
        return;
    }

    if ( NULL != BLELoopTaskHandler ) { return; }
    // run the task thread
    BaseType_t taskOK = xTaskCreatePinnedToCore([](void *obj) {
        LoTBLE * self = (LoTBLE*)obj;
        lNetLog("BLE: %p loop task begin here\n", self);
        self->_BLELoopTask();
        self->BLELoopTaskHandler=NULL;
        lNetLog("BLE: %p loop task die here\n", self);
        vTaskDelete(NULL);
    }, "loTble", LUNOKIOT_MID_STACK_SIZE, this, tskIDLE_PRIORITY,
                    &BLELoopTaskHandler,CONFIG_BT_NIMBLE_PINNED_TO_CORE);
    if ( pdPASS != taskOK ) {
        lNetLog("BLE: %p ERROR Trying to launch loop BLE Task\n",this);
    }
}

LoTBLE::LoTBLE() {
    BLEWStartEvent = new EventKVO([&](){ _TryLaunchTask(); },SYSTEM_EVENT_WAKE);
    BLEWStopEvent = new EventKVO([&](){ _TryStopTask(); },SYSTEM_EVENT_STOP);
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    enabled = NVS.getInt("BLEEnabled");
    xSemaphoreGive( taskLock );

    // Set BLE name
    uint8_t BLEAddress[6];                  // 6 octets are the BLE address
    esp_read_mac(BLEAddress,ESP_MAC_BT);    // get from esp-idf :-*
    sprintf(BTName,"lunokIoT_%02x%02x", BLEAddress[4], BLEAddress[5]); // add last MAC bytes as name
    lNetLog("BLE: %p Device name: '%s'\n",this,BTName); // notify to log

    if ( false == enabled ) { 
        Disable();
        lNetLog("BLE: %p User settings don't agree\n",this);
        return;
    }
    Enable();

}

LoTBLE::~LoTBLE() {
    delete BLEWStartEvent;
    BLEWStartEvent=nullptr;
    delete BLEWStopEvent;
    BLEWStopEvent=nullptr;
    // stop the task
    if ( NULL != BLELoopTaskHandler ) {
        xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        running=false;
        xSemaphoreGive( taskLock );
        while ( NULL != BLELoopTaskHandler ) {
            lNetLog("BLE: %p Waiting for task ends...\n");
            TickType_t nextCheck = xTaskGetTickCount();
            xTaskDelayUntil( &nextCheck, (1000 / portTICK_PERIOD_MS) );
        }
    }
    Disable();
    lNetLog("BLE: %p die here\n");
}


void LoTBLE::Enable() {
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    enabled=true;
    SqlLog("BLE: enabled");
    xSemaphoreGive( taskLock );
}

void LoTBLE::Disable() {
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    enabled=false;
    SqlLog("BLE: disabled");
    xSemaphoreGive( taskLock );
}

bool LoTBLE::IsEnabled() {
    bool response;
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    response=enabled;
    xSemaphoreGive( taskLock );
    return response;
}

bool LoTBLE::InUse() {
    bool response;
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    response=running;
    xSemaphoreGive( taskLock );
    return response;

}









bool LoTBLE::BLESendUART(const char * data) {
    if ( 0 == Clients() ) {
        lNetLog("BLE: ERROR: Unable to send UART data, no client connected\n");
        return false;
    }
    if ( nullptr == pTxCharacteristic ) {
        lNetLog("BLE: ERROR: No UART TX characteristic loaded\n");
        return false;
    }
    uint16_t MaxDataSize = NimBLEDevice::getServer()->getPeerMTU(0)-3;
    size_t currentOffset=0;
    const size_t MaxOffset=strlen(data);
    uint8_t *dataPtr=(uint8_t *)data;
    lNetLog("BLE: Sending %u byte...\n",MaxOffset);
    while(currentOffset < MaxOffset) {
        dataPtr=((uint8_t *)data)+currentOffset;
        uint16_t currentChunkSize=strlen((char*)dataPtr);
        if ( 0 == currentChunkSize) { break; }
        if ( currentChunkSize > MaxDataSize ) { currentChunkSize=MaxDataSize; }
        /*
        for(int i=0;i<currentChunkSize;i++) {  lLog("'%c' %02x, ",dataPtr[i],dataPtr[i]); }
        lLog("\n");
        */
        pTxCharacteristic->notify(dataPtr,currentChunkSize);
        lNetLog("BLE: Send (%u byte) package from: %u~%u\n",currentChunkSize,currentOffset,currentOffset+currentChunkSize);
        currentOffset+=currentChunkSize;
        delay(40);
    }
    pTxCharacteristic->notify((uint8_t*)"\n",1);
    lNetLog("BLE: %p Sended UART message: '%s' %u bytes (MTU: %u)\n",this,data,strlen(data),MaxDataSize);
    return true;
}




lBLEDevice::~lBLEDevice() {
    if ( nullptr != devName ) {
        free(devName);
        devName=nullptr;
    }
    lNetLog("BLEDevice %p destroyed\n", this);
}

/*
void BLELoopTask(void * data) {
    bleServiceRunning=true; // notify outside that I'm running
    lNetLog("BLE: Task begin\n");
    esp_err_t rc = esp_task_wdt_status(NULL);
    if ( ESP_OK == rc ) {
        lNetLog("BLE: Disable Watchdog in loop task\n");
        rc = esp_task_wdt_delete(NULL);
        if ( ESP_OK != rc ) { lNetLog("BLE: disabled Watchdog from loop task: FAIL\n"); }
    }

    oldDeviceConnected = false;
    deviceConnected = false;
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa
    while(bleEnabled) {
        esp_task_wdt_reset();

        }

        // disconnecting
        if (!deviceConnected && oldDeviceConnected) {
            //screenShootInProgress=false;
            blePeer=false;
            lNetLog("BLE: Device disconnected\n");
            
            //if (!esp_vhci_host_check_send_available()) {
            // "Controller not ready to receive packets"
            // https://github.com/h2zero/NimBLE-Arduino/blob/c2cd460792c96915058e4b2b9f4a73ffe9c38f9c/src/nimble/esp_port/esp-hci/src/esp_nimble_hci.c
            //TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
            //xTaskDelayUntil( &nextCheck, (500 / portTICK_PERIOD_MS) ); // wait a ittle bit

            //delay(500); // give the bluetooth stack the chance to get things ready
            pServer->startAdvertising(); // restart advertising
            lNetLog("BLE: Start BLE advertising\n");
            oldDeviceConnected = deviceConnected;
            bleBeingUsed=false;
        }

        // connecting
        if (deviceConnected && !oldDeviceConnected) {
            // do stuff here on connecting
            blePeer=true;
            lNetLog("BLE: Device connected\n");
            oldDeviceConnected=deviceConnected;
        }

        


    }
    deviceConnected = false;
    oldDeviceConnected = false;
    blePeer = false;
    lNetLog("BLE: Task ends here\n");
    bleServiceRunning=false;
    bleEnabled=false;
    vTaskDelete(NULL); // harakiri x'D
}
*/
/*
static void BLEStartTask(void* args) {




    // lunokiot services
    pLunokIoTService = pServer->createService(BLE_SERVICE_LUNOKIOT);
    // version announce to peers
    NimBLECharacteristic * lVersionCharacteristic = 
                pLunokIoTService->createCharacteristic(BLE_CHARACTERISTIC_LUNOKIOT_VERSION,
                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN);
    const uint32_t currVer = LUNOKIOT_BUILD_NUMBER;
    lVersionCharacteristic->setValue<uint32_t>(currVer);
    // set format on GATT
    NimBLE2904 *verCharType=(NimBLE2904*)lVersionCharacteristic->createDescriptor(BLE_DESCRIPTOR_TYPE);
    verCharType->setFormat(NimBLE2904::FORMAT_UINT32);
#ifdef LUNOKIOT_DEBUG_NETWORK
    NimBLEDescriptor * lVersionDescCharacteristic = lVersionCharacteristic->createDescriptor(BLE_DESCRIPTOR_HUMAN_DESC,NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN);
    lVersionDescCharacteristic->setValue("Version");
#endif

    // battery temp announce to peers from PMU_EVENT_TEMPERATURE event
    lBattTempCharacteristic = 
                pLunokIoTService->createCharacteristic(BLE_CHARACTERISTIC_LUNOKIOT_BATTERY_TEMP,
                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN | NIMBLE_PROPERTY::NOTIFY);
    lBattTempCharacteristic->setValue(axpTemp);
    // set format on GATT
    NimBLE2904 *battTempCharType=(NimBLE2904*)lBattTempCharacteristic->createDescriptor(BLE_DESCRIPTOR_TYPE);
    battTempCharType->setFormat(NimBLE2904::FORMAT_FLOAT32);
    battTempCharType->setUnit(0x272F); // degrees
#ifdef LUNOKIOT_DEBUG_NETWORK
    NimBLEDescriptor * lBattTempDescCharacteristic = lBattTempCharacteristic->createDescriptor(BLE_DESCRIPTOR_HUMAN_DESC,NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN);
    lBattTempDescCharacteristic->setValue("Battery Temperature");
#endif

    // BMA temp announce to peers from BMA_EVENT_TEMP event
    lBMATempCharacteristic = 
                pLunokIoTService->createCharacteristic(BLE_CHARACTERISTIC_LUNOKIOT_BMA_TEMP,
                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN | NIMBLE_PROPERTY::NOTIFY);
    lBMATempCharacteristic->setValue(axpTemp);
    // set format on GATT
    NimBLE2904 *bmaTempCharType=(NimBLE2904*)lBMATempCharacteristic->createDescriptor(BLE_DESCRIPTOR_TYPE);
    bmaTempCharType->setFormat(NimBLE2904::FORMAT_FLOAT32);
    bmaTempCharType->setUnit(0x272F); // degrees
#ifdef LUNOKIOT_DEBUG_NETWORK
    NimBLEDescriptor * lBMATempDescCharacteristic = lBMATempCharacteristic->createDescriptor(BLE_DESCRIPTOR_HUMAN_DESC,NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN);
    lBMATempDescCharacteristic->setValue("Acceleromether Temperature");
#endif
    esp_task_wdt_reset();
    // Start the services
    pLunokIoTService->start();
    pBattService->start();
    pService->start();

    // Start advertising
    NimBLEAdvertising *adv = pServer->getAdvertising();
	//-DCONFIG_BT_NIMBLE_SVC_GAP_APPEARANCE=0x0086
	//-DCONFIG_BT_NIMBLE_SVC_GAP_APPEARANCE=0x00C2
    adv->setAppearance(CONFIG_BT_NIMBLE_SVC_GAP_APPEARANCE);
    //adv->setAppearance(ESP_BLE_APPEARANCE_GENERIC_WATCH);
    adv->addTxPower();
    adv->addServiceUUID(BLE_SERVICE_LUNOKIOT);
    adv->addServiceUUID(BLE_SERVICE_BATTERY);

    // generate the table if isnt created before
    esp_task_wdt_reset();
    if ( nullptr != systemDatabase ) {
        systemDatabase->SendSQL(BLECleanUnusedQuery);
        delay(50);
    }

    BLEScan * pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new LBLEAdvertisedDeviceCallbacks(), true);
    pBLEScan->setActiveScan(false); //active scan uses more power
    pBLEScan->setInterval(300);
    pBLEScan->setWindow(299);  // less or equal setInterval value
    pBLEScan->setMaxResults(5); // dont waste memory with cache
    pBLEScan->setDuplicateFilter(false);
    xSemaphoreGive( BLEUpDownStep );
    esp_task_wdt_reset();
    adv->start();

    FreeSpace();
    lNetLog("BLE: Started!\n");
    vTaskDelete(NULL);
}
*/
/*
// call to start
void StartBLE(bool synced) {
    bool enabled = NVS.getInt("BLEEnabled");
    if ( false == enabled ) { 
        liLog("BLE: Refusing to start (disabled by user)\n");
        return;
    }
    if ( bleServiceRunning ) {
        liLog("BLE: Refusing to start (already running?)\n");
        return;
    }
    if ( bleEnabled ) {
        liLog("BLE: Refusing to start (already started)\n");
        return;
    }

    BaseType_t taskOK = xTaskCreatePinnedToCore( BLEStartTask,
                        "bleStartTask",
                        LUNOKIOT_MID_STACK_SIZE,
                        nullptr,
                        tskIDLE_PRIORITY+7,
                        &BLEStartHandler,
                        CONFIG_BT_NIMBLE_PINNED_TO_CORE);
    if ( pdPASS != taskOK ) {
        lNetLog("BLE: ERROR Trying to launch start Task\n");
        return;
    }

    if ( synced ) {
        //if ( not bleServiceRunning ) {
        lNetLog("BLE: Waiting service starts...\n");
        while(not bleServiceRunning) {
            TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
            if ( NULL != BLEStartHandler ) {
                UBaseType_t highWater = uxTaskGetStackHighWaterMark(BLEStartHandler);
                lNetLog("BLE STACK: %u\n",highWater);
            }
            esp_task_wdt_reset();
            xTaskDelayUntil( &nextCheck, (100 / portTICK_PERIOD_MS) ); // wait a ittle bit
        }
        lNetLog("BLE: Service started!\n");
        //}
    }
}
*/
/*
void BLEKickAllPeers() {
    if ( false == bleEnabled ) { return; }
    if ( nullptr == pServer ) { return; }
    if ( false == IsBLEPeerConnected() ) { return; }
    //DISCONNECT THE CLIENTS
    std::vector<uint16_t> clients = pServer->getPeerDevices();
    for(uint16_t client : clients) {
        lNetLog("BLE: kicking out client %d\n",client);
        pServer->disconnect(client);
        //pServer->disconnect(client,0x13); // remote close
        //pServer->disconnect(client,0x16); // localhost close
        delay(100);
    }
}
*/
/*
static void BLEStopTask(void* args) {
    bool * setFlag=(bool *)args;
    lNetLog("BLE: Shut down...\n");
    if( xSemaphoreTake( BLEUpDownStep, LUNOKIOT_EVENT_TIME_TICKS) != pdTRUE )  {
        lNetLog("BLE: ERROR: Unable to obtain the lock, cannot stop\n");
        bleServiceRunning=false;
        vTaskDelete(NULL);
    }

    lNetLog("BLE: Stopping...\n");
    BLEKickAllPeers();
    if ( BLEDevice::getScan()->isScanning()) {
        lNetLog("BLE: Waiting scan stops...\n");
        BLEDevice::getScan()->stop();
        //BLEDevice::getScan()->clearResults();
    }
    bleEnabled=false;
    lNetLog("BLE: Waiting service stops...\n");
    while(bleServiceRunning) { delay(50); esp_task_wdt_reset(); }
    lNetLog("BLE: Service ended\n");

    BLEDevice::deinit(true);
    battCharacteristic = nullptr;
    lBattTempCharacteristic=nullptr;
    // destroy all BLE entries
    SaveBLEDevicesToDB();
    xSemaphoreGive( BLEUpDownStep );
    *setFlag=false;
    vTaskDelete(NULL);
}

// call for stop
void StopBLE() {
    if ( not bleEnabled ) { return; }
    bool waitFor=true;
    BaseType_t taskOK = xTaskCreatePinnedToCore( BLEStopTask,
                        "bleStopTask",
                        LUNOKIOT_TASK_STACK_SIZE,
                        &waitFor,
                        tskIDLE_PRIORITY+2,
                        NULL,
                        CONFIG_BT_NIMBLE_PINNED_TO_CORE);
    if ( pdPASS != taskOK ) {
        lNetLog("BLE: ERROR Trying to launch stop Task\n");
        return;
    }

    lNetLog("BLE: Waiting stop...\n");
    while (waitFor) {
        esp_task_wdt_reset();
        delay(50);
        //lNetLog("BLE: Waiting...\n");
    }
    lNetLog("BLE: Powerdown\n");
}
*/
// install hooks for events

/*
void BLESetupHooks() {
    if ( nullptr == BLEWStopEvent ) {
        lNetLog("BLE: @TODO DISABLED STOP ON EVENT STOP\n");
        //BLEWStopEvent = new EventKVO([](){ StopBLE(); },SYSTEM_EVENT_STOP);
    }
    //if ( nullptr == BLEWLightSleepEvent ) { // this call is later than stop
    //    BLEWLightSleepEvent = new EventKVO([](){ StopBLE(); },SYSTEM_EVENT_LIGHTSLEEP);
    //}
    


    if ( nullptr == battTempPublish ) {
        battTempPublish = new EventKVO([&](){
            if ( nullptr != lBattTempCharacteristic ) {
                // dont send if decimal change, only integers
                const float currValFloat = lBattTempCharacteristic->getValue<float>();
                if ( round(axpTemp) != round(currValFloat) ) {
                    //lNetLog("BLE: Notify Battery Temperature: %.1f ºC\n",axpTemp);
                    lBattTempCharacteristic->setValue(axpTemp);
                    lBattTempCharacteristic->notify();
                }
            }
        },PMU_EVENT_TEMPERATURE);
    }
    if ( nullptr == envTempPublish ) {
        envTempPublish = new EventKVO([&](){
            if ( nullptr != lBMATempCharacteristic ) {
                // dont send if decimal change, only integers
                const float currValFloat = lBMATempCharacteristic->getValue<float>();
                //const float currValFloat = (float)lBMATempCharacteristic->getValue();
                if ( round(bmaTemp) != round(currValFloat) ) {
                    //lNetLog("BLE: Notify BMA Temperature: %.1f ºC\n",bmaTemp);
                    lBMATempCharacteristic->setValue(bmaTemp);
                    lBMATempCharacteristic->notify();
                }
            }
        },BMA_EVENT_TEMP);
    }
}
*/