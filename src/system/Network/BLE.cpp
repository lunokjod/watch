#include <functional>
#include <list>
#include <esp_task_wdt.h>
#include <Arduino.h>
#include <ArduinoNvs.h>

#include "BLE.hpp"
#include "../SystemEvents.hpp"
#include "../Datasources/kvo.hpp"

#include "../../app/Bluetooth.hpp"
#include "../Datasources/database.hpp"
#include "Gagetbridge.hpp"
#include <cstring>

extern bool networkActivity;
// monitors the wake and sleep of BLE
EventKVO * BLEWStartEvent = nullptr; 
//EventKVO * BLEWLightSleepEvent = nullptr;
EventKVO * BLEWStopEvent = nullptr;
// monitor the sensors
EventKVO * battPercentPublish = nullptr; 
EventKVO * battTempPublish = nullptr; 
EventKVO * envTempPublish = nullptr; 
// service start/stop flag
//StaticTask_t BLETaskHandler;
//StackType_t BLEStack[LUNOKIOT_TASK_STACK_SIZE];
SemaphoreHandle_t BLEUpDownStep = xSemaphoreCreateMutex(); // locked during UP/DOWN BLE service
TaskHandle_t BLELoopTaskHandler;
// monitor devices to publish on GATT
NimBLECharacteristic * battCharacteristic = nullptr;
NimBLECharacteristic * lBattTempCharacteristic = nullptr;
NimBLECharacteristic * lBMATempCharacteristic = nullptr;
// nimble things
BLEServer *pServer = nullptr;
BLEService *pService = nullptr;
BLEService *pLunokIoTService = nullptr;
BLEService *pBattService = nullptr;
BLECharacteristic * pTxCharacteristic = nullptr;

// list of know devices
SemaphoreHandle_t BLEKnowDevicesSemaphore = xSemaphoreCreateMutex();
std::list <lBLEDevice*>BLEKnowDevices;
// internal flags
volatile bool bleWaitStop=false; // internal flag to wait BLEStop task
volatile bool bleEnabled=false; // is the service on?

//@TODO old style trash must be cleaned
volatile bool bleServiceRunning=false;
volatile bool blePeer = false;
volatile bool deviceConnected = false;
volatile bool oldDeviceConnected = false;

// Server callbacks
class LBLEServerCallbacks: public BLEServerCallbacks {
    /*
    void onConnect(BLEServer* pServer) {
        lNetLog("BLE: Client connect A\n");
        deviceConnected = true;
    };*/
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        lNetLog("BLE: Client connect\n");
        deviceConnected = true;
        if ( nullptr != currentApplication ) {
            if ( 0 != strcmp(currentApplication->AppName(),"BLE pairing")) {
                LaunchApplication(new BluetoothApplication());
            }
        }
    }
    /*
    void onDisconnect(BLEServer* pServer) {
        lNetLog("BLE: Client disconnect A\n");
        deviceConnected = false;
    }*/
    void onDisconnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        lNetLog("BLE: Client disconnect\n");
        deviceConnected = false;
    }
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
        lNetLog("BLE: MTU changed to: %u\n",MTU);
    }
    uint32_t onPassKeyRequest() {
        lNetLog("BLE: Password request\n");
        return BLEDevice::getSecurityPasskey();
    }
    void onAuthenticationComplete(ble_gap_conn_desc* desc) {
        lNetLog("BLE: Authentication complete\n");
        if ( nullptr != currentApplication ) {
            if ( 0 == strcmp(currentApplication->AppName(),"BLE pairing")) {
                LaunchWatchface();
            }
        }
    }
    bool onConfirmPIN(uint32_t pin) {
        lNetLog("BLE: PIN confirmation: ");
        if ( pin == BLEDevice::getSecurityPasskey() ) {
            lLog("OK\n");
            return true;
        }
        lLog("FAIL\n");
        return false;
    }
};

extern void SqlAddBluetoothDevice(const char * mac, double distance);
// advertiser callbacks
class LBLEAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice* advertisedDevice) {
        /* lLog("BLE: '%s' '%s'\n", 
            advertisedDevice->getAddress().toString().c_str(),
            advertisedDevice->getName().c_str()
        ); */
        if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS) == pdTRUE )  {
            bool alreadyKnown = false;
            for (auto const& dev : BLEKnowDevices) {
                if ( dev->addr == advertisedDevice->getAddress() ) {
                    /*
                    lNetLog("BLE: Know dev: '%s'(%s) (from: %d secs) last seen: %d secs (%d times)\n",
                                            dev->devName,dev->addr.toString().c_str(),
                                            ((millis()-dev->firstSeen)/1000),
                                            ((millis()-dev->lastSeen)/1000),
                                            dev->seenCount);
                    */
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
                    alreadyKnown=true;
                }
            }

            if ( false == alreadyKnown ) {
                lBLEDevice * newDev = new lBLEDevice();
                newDev->addr =  advertisedDevice->getAddress();
                newDev->devName = (char *)ps_malloc(advertisedDevice->getName().length()+1);
                sprintf(newDev->devName,"%s",advertisedDevice->getName().c_str());
                //lNetLog("BLE: New dev: '%s'(%s)\n",newDev->devName, newDev->addr.toString().c_str());
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
                newDev->firstSeen = millis();
                newDev->lastSeen = newDev->firstSeen;
                BLEKnowDevices.push_back(newDev);
                SqlAddBluetoothDevice(newDev->addr.toString().c_str(),newDev->distance);
            }
            //lNetLog("BLE: seen devices: %d\n",BLEKnowDevices.size());
            xSemaphoreGive( BLEKnowDevicesSemaphore );
        }
    }
};

// http://www.espruino.com/Gadgetbridge
const size_t gadgetBridgeBufferSize=8*1024;
char *gadgetBridgeBuffer=nullptr;
size_t gadgetBridgeBufferOffset=0;

class LBLEUARTCallbacks: public BLECharacteristicCallbacks {
    
    bool gadgetbridgeCommandInProgress=false;
    bool bangleCommandInProgress=false;

    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        const char * receivedData = rxValue.c_str();

        // check for GADGETBRIDGE commands
        const char GadgetbridgeCommandEND[] = ")\n";
        const char GadgetbridgeCommandBEGIN[] = "\x10GB(";

        if ( 0 == strncmp(receivedData,GadgetbridgeCommandBEGIN,strlen(GadgetbridgeCommandBEGIN))) {
            lNetLog("BLE: UART: Begin Gadgetbridge GB command\n");
            gadgetbridgeCommandInProgress=true;
            if ( nullptr != gadgetBridgeBuffer ) { free(gadgetBridgeBuffer); gadgetBridgeBuffer=nullptr; }
            gadgetBridgeBuffer = (char*)ps_malloc(gadgetBridgeBufferSize);
            sprintf(gadgetBridgeBuffer,"%s",receivedData+strlen(GadgetbridgeCommandBEGIN)); // ignore cmd string
            gadgetBridgeBufferOffset=strlen(gadgetBridgeBuffer);
            return;
        } else if ( gadgetbridgeCommandInProgress ) {
            if (nullptr == gadgetBridgeBuffer) {
                gadgetbridgeCommandInProgress=false;
                lNetLog("BLE: UART: memory ERROR!\n");
                gadgetBridgeBufferOffset=0;
                return;
            }
            lNetLog("BLE: UART: Continue Gadgetbridge GB command\n");
            // copy to buffer
            for (int i = 0; i < rxValue.length(); i++) {
                gadgetBridgeBuffer[gadgetBridgeBufferOffset]=rxValue[i];
                gadgetBridgeBufferOffset++;
            }
            // check if is the end
            const char *toLast=receivedData;
            toLast+=strlen(receivedData)-strlen(GadgetbridgeCommandEND);
            if ( 0 == strcmp(GadgetbridgeCommandEND,toLast)) {
                gadgetBridgeBuffer[gadgetBridgeBufferOffset-2]=0; // correct eol
                gadgetbridgeCommandInProgress=false;
                lNetLog("BLE: UART: End Gadgetbridge GB command\n");
                bool parsed = ParseGadgetBridgeMessage(gadgetBridgeBuffer);
                if ( false == parsed ) {
                    lNetLog("BLE: UART: Received UNKNOWN Gadgetbridge: '%s'\n",gadgetBridgeBuffer);
                }
                gadgetBridgeBufferOffset=0;
                free(gadgetBridgeBuffer);
                gadgetBridgeBuffer=nullptr;
            }
            return;
        }
        // check for BANGLEJS extra commands
        // setTime
        const char BangleJSCommandBEGIN[] = "\x10setTime(";
        const char BangleJSCommandEND[] = "\n";

        if ( 0 == strncmp(receivedData,BangleJSCommandBEGIN,strlen(BangleJSCommandBEGIN))) {
            lNetLog("BLE: UART: Begin BangleJS command\n");
            bangleCommandInProgress=true;
            if ( nullptr != gadgetBridgeBuffer ) { free(gadgetBridgeBuffer); gadgetBridgeBuffer=nullptr; }
            gadgetBridgeBuffer = (char*)ps_malloc(gadgetBridgeBufferSize);
            sprintf(gadgetBridgeBuffer,"%s",receivedData+1); // bypass \x10
            gadgetBridgeBufferOffset=strlen(gadgetBridgeBuffer);
            return;
        } else if ( bangleCommandInProgress ) {
            if (nullptr == gadgetBridgeBuffer) {
                bangleCommandInProgress=false;
                lNetLog("BLE: UART: memory ERROR!\n");
                gadgetBridgeBufferOffset=0;
                return;
            }
            lNetLog("BLE: UART: Continue BangleJS command\n");
            // copy to buffer
            for (int i = 0; i < rxValue.length(); i++) {
                gadgetBridgeBuffer[gadgetBridgeBufferOffset]=rxValue[i];
                gadgetBridgeBufferOffset++;
            }
            // check if is the end
            const char *toLast=receivedData;
            toLast+=strlen(receivedData)-strlen(BangleJSCommandEND);
            if ( 0 == strcmp(BangleJSCommandEND,toLast)) {
                gadgetBridgeBuffer[gadgetBridgeBufferOffset-1]=0; // correct eol
                bangleCommandInProgress=false;
                lNetLog("BLE: UART: End BangleJS command\n");
                bool parsed = ParseBangleJSMessage(gadgetBridgeBuffer);
                if ( false == parsed ) {
                    lNetLog("BLE: UART: Received UNKNOWN BangleJS: '%s'\n",gadgetBridgeBuffer);
                }
                gadgetBridgeBufferOffset=0;
                free(gadgetBridgeBuffer);
                gadgetBridgeBuffer=nullptr;
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

lBLEDevice::~lBLEDevice() {
    if ( nullptr != devName ) {
        free(devName);
        devName=nullptr;
    }
    lNetLog("BLEDevice %p destroyed\n", this);
}


void BLELoopTask(void * data) {
    bleServiceRunning=true; // notify outside that I'm running
    lNetLog("BLE: Task begin\n");
    /*
    union RLEPackage { // description for send IMAGES via UART TX notification
        uint8_t bytes[5];
        struct {
            uint16_t color;
            uint8_t repeat; // the same color must repeat in next N'th pixels (RLE)
            uint8_t x; // screen is 240x240, don't waste my time! :(
            uint8_t y; // why send coordinates? because BLE notify can be lost
        };
    };*/

    void *theScreenShotToSend=nullptr; // point to current screenshoot
    // temp values
    screenShootCurrentImageX=0;
    screenShootCurrentImageY=0;
    oldDeviceConnected = false;
    deviceConnected = false;
    while(bleEnabled) {
        esp_task_wdt_reset();
        delay(10);
        // clean old BLE peers here
        if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS) == pdTRUE )  {
            size_t b4Devs = BLEKnowDevices.size();
            BLEKnowDevices.remove_if([](lBLEDevice *dev){
                int16_t seconds = (millis()-dev->lastSeen)/1000;
                if ( seconds > 80 ) {
                    //lNetLog("BLE: Removed expired seen device: '%s' %s\n",dev->devName,dev->addr.toString().c_str());
                    delete dev;
                    dev=nullptr;
                    return true;
                }
                return false;
            });
            xSemaphoreGive( BLEKnowDevicesSemaphore );
        }
        /*
        if ( blePeer ) {
            if ( BLESendScreenShootCommand ) { // the client has sended "GETSCREENSHOOT" in the BLE UART
                // first iteration
                if ( false == screenShootInProgress ) {
                    theScreenShotToSend = (void*)screenShootCanvas;
                    screenShootInProgress = true;
                    lNetLog("Begin send screenshoot via BLE UART\n");
                    screenShootCurrentImageY=0;
                    screenShootCurrentImageX=0;
                    networkActivity=true;
                    lastRLECount=0;
                } else { // in progress
                    if ( nullptr != theScreenShotToSend) {
                        rgb565 myColor;
                        myColor.u16 = screenShootCanvas->readPixel(screenShootCurrentImageX,screenShootCurrentImageY);
                        //Serial.printf("X: %d Y: %d VAL: 0x%x\n", imageX, imageY, myColor.u16);
                        if ( myLastColor.u16 == myColor.u16 ) {
                            lastRLECount++;
                            if ( lastRLECount  > 254 ) { // byte full...must send
                                RLEPackage valToSend = { .color = myLastColor.u16 };
                                valToSend.repeat = 255;
                                valToSend.x = screenShootCurrentImageX;
                                valToSend.y = screenShootCurrentImageY;
                                pTxCharacteristic->setValue(&valToSend.bytes[0], sizeof(RLEPackage));
                                pTxCharacteristic->notify(true);
                                delay(20);
                                lastRLECount=0;
                                myLastColor=myColor;
                                continue;
                            }                            
                            //Serial.printf("REPEATED RLE: %d RGB565: 0x%x\n", lastRLECount,myLastColor.u16);
                        } else if ( myLastColor.u16 != myColor.u16 ) {
                            RLEPackage valToSend = { .color = myLastColor.u16 };
                            valToSend.repeat = lastRLECount;
                            valToSend.x = screenShootCurrentImageX;
                            valToSend.y = screenShootCurrentImageY;
                            //Serial.printf("RLEDATA: 0x%x\n", valToSend.rleData);
                            pTxCharacteristic->setValue(&valToSend.bytes[0], sizeof(RLEPackage));
                            pTxCharacteristic->notify(true);
                            realImageSize +=(lastRLECount*2); // image is RGB565 (16bit per pixel)
                            imageUploadSize+=sizeof(RLEPackage); // real packet data
                            delay(20);
                            lastRLECount=1;
                            myLastColor=myColor;
                            UINextTimeout=millis()+UITimeout; // don't allow the UI stops
                            continue;
                        }

                        myLastColor=myColor;
                        screenShootCurrentImageX++;
                        if ( screenShootCurrentImageX >= screenShootCanvas->width() ) {
                            screenShootCurrentImageY++;
                            screenShootCurrentImageX=0;
                            //imageY = screenShootCanvas->height()+1;
                        }
                        if ( screenShootCurrentImageY >= screenShootCanvas->height() ) {
                            realImageSize=TFT_WIDTH*TFT_HEIGHT*sizeof(uint16_t);
                            lNetLog("Network: Screenshot served by BLE (image: %d byte) upload: %d byte\n",realImageSize,imageUploadSize);
                            delay(10);
                            theScreenShotToSend=nullptr;
                            realImageSize=0;
                            imageUploadSize=0;
                            screenShootCurrentImageY=0;
                            screenShootCurrentImageX=0;
                            BLESendScreenShootCommand=false;
                            screenShootInProgress = false;
                            //DISCONNECT THE CLIENT to notify end of picture
                            BLEKickAllPeers();
                            networkActivity=false;
                        }
                    }
                }
            }
        }*/
        // disconnecting
        if (!deviceConnected && oldDeviceConnected) {
            //screenShootInProgress=false;
            blePeer=false;
            lNetLog("BLE: Device disconnected\n");
            //delay(500); // give the bluetooth stack the chance to get things ready
            pServer->startAdvertising(); // restart advertising
            lNetLog("BLE: Start BLE advertising\n");
            oldDeviceConnected = deviceConnected;
            networkActivity=false;
        }

        // connecting
        if (deviceConnected && !oldDeviceConnected) {
            // do stuff here on connecting
            blePeer=true;
            lNetLog("BLE: Device connected\n");
            oldDeviceConnected=deviceConnected;
            networkActivity=true;
        }
    }
    networkActivity=false;
    //screenShootInProgress=false;
    deviceConnected = false;
    oldDeviceConnected = false;
    blePeer = false;
    lNetLog("BLE: Task ends here\n");
    bleServiceRunning=false;
    bleEnabled=false;
    vTaskDelete(NULL); // harakiri x'D
}
//extern void _intrnalSqlStatic(void *args);
static void BLEStartTask(void* args) {
    // get lock 
    if( xSemaphoreTake( BLEUpDownStep, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) != pdTRUE )  {
        lNetLog("BLE: ERROR: Unable to obtain the lock, cannot start\n");
        vTaskDelete(NULL);
    }
    // check if user wants BLE
    bool enabled = NVS.getInt("BLEEnabled");
    if ( false == enabled ) { 
        liLog("BLE disabled by user\n");
        xSemaphoreGive( BLEUpDownStep );
        vTaskDelete(NULL);
    }
    // lets bring up BLE
    lNetLog("BLE: Starting...\n"); // notify to log
    bleEnabled = true;             // notify BLE is in use
    /*
    if( xSemaphoreTake( SqlLogSemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) == pdTRUE )  {
        const char *query3=(char *)"CREATE TABLE if not exists bluetooth ( id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, address text NOT NULL, distance INT DEFAULT 0);";
        esp_task_wdt_reset();
        int  rc = db_exec(lIoTsystemDatabase,query3);
        if (rc != SQLITE_OK) {
            lSysLog("SQL: ERROR: Unable exec: '%s'\n", query3);
        }
        esp_task_wdt_reset();
        xSemaphoreGive( SqlLogSemaphore ); // free
    }*/
    uint8_t BLEAddress[6];                  // 6 octets are the BLE address
    esp_read_mac(BLEAddress,ESP_MAC_BT);    // get from esp-idf :-*

    char BTName[15] = { 0 };                // buffer for build the name like: "lunokIoT_69fa"
    sprintf(BTName,"lunokIoT_%02x%02x", BLEAddress[4], BLEAddress[5]); // add last MAC bytes as name

    lNetLog("@WARNING CRASH CAN BECOME HERE!!!!!!!!!!!!!!!!!!!!!!\n");
    //BLEDevice::deinit();
    lNetLog("BLE: Device name: '%s'\n",BTName); // notify to log
    delay(100);
    FreeSpace();
    // Create the BLE Device
    BLEDevice::init(std::string(BTName)); // hate strings
    esp_task_wdt_reset();
    
    BLEDevice::setSecurityAuth(true,true,true);
    uint32_t generatedPin=random(0,999999);
    lNetLog("BLE: generated PIN: %06d\n",generatedPin);
    //FreeSpace();
    BLEDevice::setSecurityPasskey(generatedPin);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    NimBLEDevice::setPower(ESP_PWR_LVL_N3);
        //ESP_PWR_LVL_P9); /** +9db */
    // create GATT the server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new LBLEServerCallbacks(),true); // destroy on finish
    esp_task_wdt_reset();

    lNetLog("BLE: Server started\n");

    // Create the BLE Service UART
    pService = pServer->createService(SERVICE_UART_UUID);
    pTxCharacteristic = pService->createCharacteristic( CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY );
    // UART data come here
    BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_RX,NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN);
    pRxCharacteristic->setCallbacks(new LBLEUARTCallbacks());

    // battery services
    // https://circuitdigest.com/microcontroller-projects/esp32-ble-server-how-to-use-gatt-services-for-battery-level-indication
    pBattService = pServer->createService(BLE_SERVICE_BATTERY);
    battCharacteristic = pBattService->createCharacteristic(BLE_CHARACTERISTIC_BATTERY,
                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN | NIMBLE_PROPERTY::NOTIFY );
    uint8_t level = 0;
    if ( -1 != batteryPercent ) { level = batteryPercent; }
    battCharacteristic->setValue(level);    
#ifdef LUNOKIOT_DEBUG_NETWORK
    NimBLEDescriptor * battDescriptor = battCharacteristic->createDescriptor(BLE_DESCRIPTOR_HUMAN_DESC,NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN);
    battDescriptor->setValue("Percentage 0 - 100");
#endif

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
    adv->start();

    BLEScan * pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new LBLEAdvertisedDeviceCallbacks(), true);
    pBLEScan->setActiveScan(false); //active scan uses more power
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);  // less or equal setInterval value
    pBLEScan->setMaxResults(3); // dont waste memory with cache
    pBLEScan->setDuplicateFilter(false);
    xSemaphoreGive( BLEUpDownStep );
    //bleWaitStop=true;
    BaseType_t taskOK = xTaskCreatePinnedToCore(BLELoopTask, "lble",
                    LUNOKIOT_APP_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), &BLELoopTaskHandler,1);
    if ( pdPASS != taskOK ) {
        lNetLog("BLE: ERROR Trying to launch loop BLE Task\n");
    }
    /// esp_err_t sleepEnabled = esp_bt_sleep_enable(); <== @TODO looking for device wake on ble from sleep :(
    vTaskDelete(NULL);
}

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
    delay(50);
    BaseType_t taskOK = xTaskCreatePinnedToCore( BLEStartTask,
                        "bleStartTask",
                        LUNOKIOT_MID_STACK_SIZE,
                        nullptr,
                        tskIDLE_PRIORITY,
                        NULL,
                        1);
    if ( pdPASS != taskOK ) {
        lNetLog("BLE: ERROR Trying to launch start Task\n");
        return;
    }
    if ( synced ) {
        //if ( not bleServiceRunning ) {
        lNetLog("BLE: Waiting service starts...\n");
        while(not bleServiceRunning) { delay(50); esp_task_wdt_reset(); }
        lNetLog("BLE: Service started!\n");
        //}
    }
}


void BLEKickAllPeers() { //@TODO SEEMS NOT WORK
    if ( false == bleEnabled ) { return; }
    if ( nullptr != pServer ) {
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

}

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
        BLEDevice::getScan()->clearResults();
        BLEDevice::getScan()->clearDuplicateCache();
        //delay(100);
    }
    bleEnabled=false;
    //if ( bleServiceRunning ) {
    lNetLog("BLE: Waiting service stops...\n");
    while(bleServiceRunning) { delay(50); esp_task_wdt_reset(); }
    lNetLog("BLE: Service ended\n");
    //}

    BLEDevice::deinit(true);
    battCharacteristic = nullptr;
    lBattTempCharacteristic=nullptr;
    // destroy all BLE entries
    if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS) == pdTRUE )  {
        BLEKnowDevices.remove_if([](lBLEDevice *dev){
            lNetLog("BLE: Free know device: '%s'\n", dev->addr.toString().c_str());
            return true;
        });
        xSemaphoreGive( BLEKnowDevicesSemaphore );
    }
    xSemaphoreGive( BLEUpDownStep );
    *setFlag=false;
    vTaskDelete(NULL);
}

// call for stop
void StopBLE() {
    if ( not bleEnabled ) { return; }
    bool waitFor=true;
    /*
    xTaskCreateStaticPinnedToCore( BLEStopTask,
                    "bleStopTask",
                    LUNOKIOT_TASK_STACK_SIZE,
                    &waitFor,
                    tskIDLE_PRIORITY+1,
                    BLEStack,
                    &BLETaskHandler,
                    1);
                    */
    BaseType_t taskOK = xTaskCreatePinnedToCore( BLEStopTask,
                        "bleStopTask",
                        LUNOKIOT_TASK_STACK_SIZE,
                        &waitFor,
                        tskIDLE_PRIORITY,
                        NULL,
                        1);
    if ( pdPASS != taskOK ) {
        lNetLog("BLE: ERROR Trying to launch stop Task\n");
        return;
    }

    lNetLog("BLE: Waiting stop...\n");
    while (waitFor) {
        esp_task_wdt_reset();
        delay(20);
        //lNetLog("BLE: Waiting...\n");
    }
    lNetLog("BLE: Powerdown\n");
    //lNetLog("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB END\n");
}

// install hooks for events
void BLESetupHooks() {
    if ( nullptr == BLEWStartEvent ) {
        BLEWStartEvent = new EventKVO([](){ StartBLE(); },SYSTEM_EVENT_WAKE);
    }
    if ( nullptr == BLEWStopEvent ) {
        lNetLog("BLE: @TODO DISABLED STOP ON EVENT STOP\n");
        //BLEWStopEvent = new EventKVO([](){ StopBLE(); },SYSTEM_EVENT_STOP);
    }
    //if ( nullptr == BLEWLightSleepEvent ) { // this call is later than stop
    //    BLEWLightSleepEvent = new EventKVO([](){ StopBLE(); },SYSTEM_EVENT_LIGHTSLEEP);
    //}
    

    // capture sensor events and push to BLE
    if ( nullptr == battPercentPublish ) {
        battPercentPublish = new EventKVO([&](){
            if ( nullptr != battCharacteristic ) {
                uint8_t level = 0;
                if ( -1 != batteryPercent ) { level = batteryPercent; }
                //lNetLog("BLE: Notify Battery: %d%%\n",level); // notify to log
                battCharacteristic->setValue(level);
                battCharacteristic->notify();
            }
        },PMU_EVENT_BATT_PC);
    }
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
