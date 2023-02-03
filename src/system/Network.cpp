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
#include <Arduino.h>
#include <LilyGoWatch.h>

#include "Network.hpp"

#include <functional>
#include <list>
#include <NimBLEDevice.h>

#include "lunokiot_config.hpp"
#include "SystemEvents.hpp"

#include <ArduinoNvs.h> // persistent values
#include <WiFi.h>
#include "UI/UI.hpp" // for rgb unions
#include <HTTPClient.h>

#ifdef LUNOKIOT_LOCAL_CLOUD_ENABLED
#include "LocalCloud.hpp"
#endif
#include "Datasources/kvo.hpp"
#include "../app/LogView.hpp"
#include <esp_task_wdt.h>
// https://stackoverflow.com/questions/44951078/when-how-is-a-ble-gatt-notify-indicate-is-send-on-physical-layer
extern char * latestBuildFoundString;
#include "../app/OTAUpdate.hpp"
extern bool screenShootInProgress;
#include <Ticker.h>
Ticker BootNetworkTicker; // first shoot
Ticker NetworkTicker; // loop every 5 minutes
extern bool provisioned;
bool wifiOverride=false; // this var disables the wifi automatic disconnection
int BLEscanTime = 5; //In seconds
unsigned long BLENextscanTime = 0;
BLEServer *pServer = nullptr;

BLEService *pService = nullptr;
BLEService *pLunokIoTService = nullptr;
BLEService *pBattService = nullptr;
extern TFT_eSprite *screenShootCanvas;
BLECharacteristic * pTxCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;
SemaphoreHandle_t BLEKnowDevicesSemaphore = xSemaphoreCreateMutex();
NimBLECharacteristic * battCharacteristic = nullptr;
NimBLECharacteristic * lBattTempCharacteristic = nullptr;
NimBLECharacteristic * lBMATempCharacteristic = nullptr;
std::list <lBLEDevice*>BLEKnowDevices;
EventKVO * battPercentPublish = nullptr; 
EventKVO * battTempPublish = nullptr; 
EventKVO * envTempPublish = nullptr; 
bool networkActivity = false;
extern TFT_eSprite *screenShootCanvas;    // the last screenshoot
extern bool screenShootInProgress;        // Notify Watchface about send screenshoot
size_t imageUploadSize=0;
size_t realImageSize=0;
rgb565 myLastColor = { .u16 = (uint16_t)TFT_TRANSPARENT }; // improbable color
uint16_t lastRLECount = 0;
#ifdef LUNOKIOT_UPDATES_LOCAL_URL
// Use alternative keys
extern const PROGMEM uint8_t githubPEM_start[] asm("_binary_asset_server_pem_start");
extern const PROGMEM uint8_t githubPEM_end[] asm("_binary_asset_server_pem_end");
#else
extern const PROGMEM uint8_t githubPEM_start[] asm("_binary_asset_raw_githubusercontent_com_pem_start");
extern const PROGMEM uint8_t githubPEM_end[] asm("_binary_asset_raw_githubusercontent_com_pem_end");
#endif

TaskHandle_t BLETaskHandler=NULL;
volatile bool bleEnabled = false;
volatile bool bleServiceRunning = false;
volatile bool blePeer = false;
bool BLESendScreenShootCommand = false;
// https://mynewt.apache.org/latest/network/ble_sec.html

// Network scheduler list
std::list<NetworkTaskDescriptor *> networkPendingTasks = {};

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */  
class LBLEServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        lNetLog("BLE: Client connect A\n");
        deviceConnected = true;
    };
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        lNetLog("BLE: Client connect B\n");

    }
    void onDisconnect(BLEServer* pServer) {
        lNetLog("BLE: Client disconnect A\n");
        deviceConnected = false;
    }
    void onDisconnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        lNetLog("BLE: Client disconnect B\n");
        deviceConnected = false;
    }
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
        lNetLog("BLE: MTU change\n");
    }
    uint32_t onPassKeyRequest() {
        lNetLog("BLE: Password request\n");
        return BLEDevice::getSecurityPasskey();
    }
    void onAuthenticationComplete(ble_gap_conn_desc* desc) {
        lNetLog("BLE: Authentication complete\n");
        //LaunchWatchface();
    }
    bool onConfirmPIN(uint32_t pin) {
        if ( pin == BLEDevice::getSecurityPasskey() ) { return true; }
        return false;
    }
    /*
    uint32_t onPassKeyRequest(){
      lNetLog("BLE: Server PassKeyRequest @TODO\n");
      return 123456; 
    }

    bool onConfirmPIN(uint32_t pass_key){
      lNetLog("BLE: The passkey YES/NO number: '%s'\n",pass_key);
      return true; 
    }*/
};

class LBLEAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice* advertisedDevice) {
        /* lLog("BLE: '%s' '%s'\n", 
            advertisedDevice->getAddress().toString().c_str(),
            advertisedDevice->getName().c_str()
        ); */
        bool alreadyKnown = false;
        if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS) == pdTRUE )  {
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
            }
            //lNetLog("BLE: seen devices: %d\n",BLEKnowDevices.size());
            xSemaphoreGive( BLEKnowDevicesSemaphore );
        }
    }
};

class LBLEUARTCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            const char * receivedCommand = rxValue.c_str();
            const char * ScreenShootCommand = "GETSCREENSHOOT";
            const char * PushMessageCommand = "PUSHMSG";

            if ( 0 == strncmp(PushMessageCommand,receivedCommand, strlen(PushMessageCommand))) {
                lNetLog("BLE: UART: '%s' command received\n",PushMessageCommand);
            } else if ( 0 == strncmp(ScreenShootCommand,receivedCommand, strlen(ScreenShootCommand))) {
                lNetLog("BLE: UART: '%s' command received\n",ScreenShootCommand);
                BLESendScreenShootCommand = true;
            } else {
                BLESendScreenShootCommand=false;
                lNetLog("BLE: UART Received Value: ");
                for (int i = 0; i < rxValue.length(); i++) {
                    lLog("'%c'(0x%x) ",rxValue[i],rxValue[i]);
                }
                lLog("\n");
            }
        }
    }
};

lBLEDevice::~lBLEDevice() {
    if ( nullptr != devName ) {
        free(devName);
        devName=nullptr;
    }
    lNetLog("BLEDevice %p destroyed\n", this);
}

/*
 * Stop desired task from network scheduler
 */
bool RemoveNetworkTask(NetworkTaskDescriptor *oldTsk) {
    lNetLog("RemoveNetworkTask: Task %p '%s' removed\n", oldTsk,oldTsk->name);
    networkPendingTasks.remove(oldTsk);
    return true;
}

/*
 * Add a timed network task to the network scheduler
 */
bool AddNetworkTask(NetworkTaskDescriptor *nuTsk) {
    size_t offset = 0;
    for (auto const& tsk : networkPendingTasks) {
        if ( tsk == nuTsk ) {
            lNetLog("AddNetworkTask: Task %p '%s' already on the list (offset: %d)\n", tsk,tsk->name, offset);
            return false;
        }
        offset++;
    }
    networkPendingTasks.push_back(nuTsk);
    lNetLog("AddNetworkTask: Task %p '%s' added (offset: %d)\n", nuTsk,nuTsk->name, offset);
    return true;
}
/*
 * Search for update system network task
 */

void SystemUpdateAvailiable(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    if ( nullptr == latestBuildFoundString) {
        lNetLog("OTA: Version must be checked online before launch\n");
        return;
    }
    const uint32_t myVersion = LUNOKIOT_BUILD_NUMBER;
    uint32_t remoteVersion = atoi(latestBuildFoundString);
    if ( remoteVersion > myVersion ) {
        lNetLog("OTA: new version availiable: %d, current: %d :) Launching Update...\n",remoteVersion,myVersion);
        LaunchApplication(new OTAUpdateApplication(),true);
    }
}


NetworkTaskDescriptor * SearchUpdateNetworkTask = nullptr;
void SearchUpdateAsNetworkTask() {
    if ( nullptr == SearchUpdateNetworkTask ) {
        SearchUpdateNetworkTask = new NetworkTaskDescriptor();
        SearchUpdateNetworkTask->name = (char *)"Check updates";
        SearchUpdateNetworkTask->everyTimeMS = (((1000*60)*60)*24*3); // every 3 days
        SearchUpdateNetworkTask->_lastCheck=millis();
        SearchUpdateNetworkTask->_nextTrigger=0; // launch NOW if no synched never again
        SearchUpdateNetworkTask->callback = []() {
            lNetLog("Checking for updates...\n");
            // @NOTE to get the PEM from remote server:
            // openssl s_client -connect raw.githubusercontent.com:443 -showcerts </dev/null | openssl x509 -outform pem > raw_githubusercontent_com.pem
            // echo "" | openssl s_client -showcerts -connect raw.githubusercontent.com:443 | sed -n "1,/Root/d; /BEGIN/,/END/p" | openssl x509 -outform PEM >raw_githubusercontent_com.pem
            //#ifdef LUNOKIOT_UPDATES_LOCAL_URL
                //WiFiClient*client = new WiFiClient;
            //#else
                WiFiClientSecure *client = new WiFiClientSecure;
            //#endif
            if (nullptr != client ) {
                HTTPClient https;
                https.setConnectTimeout(LUNOKIOT_UPDATE_TIMEOUT);
                https.setTimeout(LUNOKIOT_UPDATE_TIMEOUT);
                https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
                client->setCACert((const char*)githubPEM_start);
                #ifdef LUNOKIOT_UPDATES_LOCAL_URL
                    lNetLog("WARNING: Insecure updates due local self-signed certificate (-DLUNOKIOT_UPDATES_LOCAL_URL)\n");
                    client->setInsecure();
                #endif
                //String serverPath = LastVersionURL;
                bool mustUpdate = false;
                lNetLog("SearchUpdate: Using URL '%s'\n",LastVersionURL);
                if (https.begin(*client, String(LastVersionURL))) {
                    // HTTPS
                    //lNetLog("SearchUpdate: https get '%s'...\n", serverPath.c_str());
                    // start connection and send HTTP header
                    int httpCode = https.GET();

                    // httpCode will be negative on error
                    if (httpCode > 0) {
                        // HTTP header has been send and Server response header has been handled
                        //lNetLog("SearchUpdate: https get code: %d\n", httpCode);
                        // file found at server
                        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                            String payload = https.getString();
                            if ( nullptr != latestBuildFoundString ) {
                                free(latestBuildFoundString);
                                latestBuildFoundString = nullptr;
                            }
                            latestBuildFoundString = (char*)ps_malloc(payload.length()+1);
                            strcpy(latestBuildFoundString,payload.c_str());
                            if ( 0 != strcmp(LUNOKIOT_BUILD_STRING,latestBuildFoundString)) {
                                lNetLog("SearchUpdate: Received: '%s' current: '%s'\n",latestBuildFoundString,LUNOKIOT_BUILD_STRING);
                                mustUpdate=true;
                            }
                        } else {
                            lNetLog("SearchUpdate: http get rough response: %s\n", https.errorToString(httpCode).c_str());
                        }
                    } else {
                        lNetLog("SearchUpdate: http get failed, error: %s\n", https.errorToString(httpCode).c_str());
                    }
                    https.end();
                } else {
                    lNetLog("SearchUpdate: Unable to connect\n");
                    delete client;
                    return true; // fake success return to do not call me again until next loop (isnt prioritary)
                }
                lNetLog("SearchUpdate: end\n");
                delete client;
                if ( mustUpdate ) {
                    esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_UPDATE, nullptr, 0, LUNOKIOT_EVENT_FAST_TIME_TICKS);
                }
                return true;
            }
            lNetLog("SearchUpdate: Unable to create WiFiClientSecure\n");
            return false;
        };
        AddNetworkTask(SearchUpdateNetworkTask);
    }
}

/*
 * The network scheduler loop
 * Check all network tasks and determine if must be launched
 * Note: ReconnectPeriodMs determines minimal period of scheduling (keep it high for battery saving)
 */
bool networkTaskRunning = false; // is network task running?
bool networkTaskResult = false; // result from last
void NetworkTaskCallTask(void *data) {
    NetworkTaskDescriptor * task = (NetworkTaskDescriptor *)data;
    lNetLog("-[ NET TASK BEGIN ] ------------------------------------\n");
    FreeSpace();
    networkTaskResult = task->callback();
    FreeSpace();
    lNetLog("-[ NET TASK END   ] ------------------------------------\n");
    networkTaskRunning=false;
    vTaskDelete(NULL);
}

// any task needs to start?
bool NetworkTaskIsPending() {
    bool mustStart = false;
    // determine if need to start wifi in this period
    for (auto const& tsk : networkPendingTasks) {
        if ( false == tsk->enabled ) { continue; } // isnt enabled, ignore
        //need to run now or in the next? (0/-1)
        if ( ( -1 == tsk->_nextTrigger ) || ( millis() > tsk->_nextTrigger )) { // launch now or in the next iteration?
            lNetLog("NetworkTask: Pending task '%s'\n", tsk->name);
            mustStart = true;
        }
        tsk->_lastCheck = millis(); // mark as revised
    }
    return mustStart;
}
void NetworkTaskRun(void *data) {
    // Tasks pending!!!
    lNetLog("Network: Timed Tasks WiFi procedure begin\n");
    FreeSpace();

    String WSSID = NVS.getString("provSSID");
    String WPass = NVS.getString("provPWD");
    if ( 0 == WSSID.length() ) {
        lNetLog("WARNING: Provisioned data not set?\n");
        provisioned=false;
        vTaskDelete(NULL);
    }
    //lNetLog("WiFi SSID: '%s' pass: '%s'\n",WSSID.c_str(),WPass.c_str());
    wifiOverride=true;
    wl_status_t currStat = WiFi.begin(WSSID.c_str(),WPass.c_str());
    if ( WL_CONNECT_FAILED == currStat ) {
        lNetLog("WiFi: Connect failed\n");
        wifiOverride=false;
        vTaskDelete(NULL);
    }
    delay(300);
    FreeSpace();
    bool connected=false;
    unsigned long timeout = millis()+15000; 
    while(timeout>millis()) {
        lNetLog("Network: WiFi Try to connect (%d)...\n",currStat);
        if ( WL_CONNECTED == currStat) { connected=true; break; }
        delay(1000);
        currStat = WiFi.status();
    }
    if (false == connected) {
        wifiOverride=false;
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        lNetLog("Network: WiFi timeout! (more luck next time!)\n");
        StartBLE();
        vTaskDelete(NULL);
    }
    // party continues!!! (now are online)

    // Run pending tasks...
    for (auto const& tsk : networkPendingTasks) {
        if ( -1 == tsk->_nextTrigger ) {
            tsk->_nextTrigger = millis()+tsk->everyTimeMS;
        } else {
            if ( millis() > tsk->_nextTrigger ) {
                delay(1000);
                if ( WL_CONNECTED != WiFi.status() ) {
                    lNetLog("NetworkTask: ERROR: No connectivity, Network Tasks cannot run\n");
                    break;
                }
                UINextTimeout = millis()+UITimeout;  // dont allow screen sleep
                lNetLog("NetworkTask: Running task '%s' (stack: %u)...\n", tsk->name,tsk->desiredStack);
                networkTaskRunning=true;
                TaskHandle_t taskHandle;
                BaseType_t taskOK = xTaskCreatePinnedToCore(NetworkTaskCallTask,"",tsk->desiredStack,(void*)tsk,uxTaskPriorityGet(NULL),&taskHandle,0);
                if ( pdPASS != taskOK ) {
                    lNetLog("NetworkTask: ERROR Trying to run task: '%s'\n", tsk->name);
                    tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                    tsk->_lastCheck = millis();
                    continue;
                }


                unsigned long taskTimeout = millis()+15000;
                bool taskAborted=false;
                while(networkTaskRunning) { //@TODO must implement timeout
                    delay(1000); // one second
                    if ( networkTaskRunning ) {
                        lNetLog("NetworkTask: Waiting task '%s'...\n", tsk->name);
                    }
                    if ( millis() > taskTimeout ) {
                        taskAborted = true;
                        lNetLog("NetworkTask: Abort task '%s' by TIMEOUT!\n", tsk->name);
                        if ( eDeleted != eTaskGetState(taskHandle) ) {
                            vTaskDelete(taskHandle);
                        }
                        break;
                    }
                }
                if ( false == taskAborted ) {
                    lNetLog("NetworkTask: Task end '%s' Result: '%s'\n", tsk->name, (networkTaskResult?"OK":"ERROR"));
                }

                tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                tsk->_lastCheck = millis();
                //delay(150);
            }
        }
    }
    WiFi.disconnect();
    delay(200);
    WiFi.mode(WIFI_OFF);
    delay(200);
    wifiOverride=false;
    lNetLog("Network: Pending tasks end!\n");
    StartBLE();

    vTaskDelete(NULL);
}
void NetworkTasksCheck() {
    if ( systemSleep ) { return; }
    if ( wifiOverride ) { liLog("WiFi: override in progress\n"); return; }
    if ( false == NVS.getInt("WifiEnabled") ) {
        liLog("WiFi: WiFi disabled by user\n");
        StartBLE(); // restore BLE
        return;
    }

    if ( false == provisioned ) { liLog("WiFi: WARNING: Not provisioned\n"); return; }
    if ( false == NetworkTaskIsPending() ) { lNetLog("Network: No pending Timed Tasks\n"); return; }

    if ( bleServiceRunning ) {
        lNetLog("Network: BLE must be temporaly disabled to maximize WiFi effort\n");
        StopBLE();
    }

    BaseType_t taskOK = xTaskCreatePinnedToCore(NetworkTaskRun,"",LUNOKIOT_NETWORK_STACK_SIZE,NULL,uxTaskPriorityGet(NULL), NULL,0);
    if ( pdPASS != taskOK ) {
        lNetLog("NetworkTask: ERROR Trying to launch Tasks\n");
        StartBLE();
    }
}

/*
 * Create the task for network scheduler
 * How it works?
 * * Call NetworkHandler on setup()
 * * Later on create a new NetworkTaskDescriptor() (in heap!)
 * * Push to the scheduler using AddNetworkTask()
 * * Note: ReconnectPeriodMs determines minimal period of scheduling (keep it high for battery saving)
 */

//StackType_t NetworkTaskStack[ LUNOKIOT_NETWORK_TASK_STACK_SIZE ];
//StaticTask_t NetworkTaskBuffer;
bool NetworkHandler() {
    lNetLog("NetworkHandler\n");
    #ifdef LUNOKIOT_WIFI_ENABLED
        provisioned = NVS.getInt("provisioned");
        if ( false == provisioned ) { liLog("WiFi: WARNING: Not provisioned\n"); }
        lNetLog("WiFi: Network tasks handler\n");
        if ( provisioned ) {
            lNetLog("WiFi: Provisioned :) Task runs in few seconds...\n");
            BootNetworkTicker.once(8,NetworkTasksCheck);
        }
        NetworkTicker.attach(60*30,NetworkTasksCheck); // every 30 minutes

        esp_err_t registered = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_UPDATE, SystemUpdateAvailiable, nullptr, NULL);
        if ( ESP_OK != registered ) {
            lSysLog("NetworkHandler: ERROR: Unable register UPDATE manager\n");
        }

        
        #ifdef LUNOKIOT_UPDATES_ENABLED
            SearchUpdateAsNetworkTask();
        #endif
    #endif

    #ifdef LUNOKIOT_BLE_ENABLED
        lNetLog("BLE: System hooks\n");
        BLESetupHooks();
    #endif

    #ifdef LUNOKIOT_LOCAL_CLOUD_ENABLED
        StartLocalCloudClient();
    #endif

    if ( provisioned ) { return true; }
    return false;
}

void BLELoopTask(void * data) {
    bleServiceRunning=true; // notify outside that I'm running
    lNetLog("BLE: Task begin\n");

    union RLEPackage { // description for send IMAGES via UART TX notification
        uint8_t bytes[5];
        struct {
            uint16_t color;
            uint8_t repeat; // the same color must repeat in next N'th pixels (RLE)
            uint8_t x; // screen is 240x240, don't waste my time! :(
            uint8_t y; // why send coordinates? because BLE notify can be lost
        };
    };

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
            screenShootInProgress=false;
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
    screenShootInProgress=false;
    deviceConnected = false;
    oldDeviceConnected = false;
    blePeer = false;
    lNetLog("BLE: Task ends here\n");
    BLETaskHandler=NULL; // destroy my own descriptor
    bleServiceRunning=false;
    vTaskDelete(NULL); // harakiri x'D
}

static void BLELauncherTask(void* args) {
    bleEnabled = true;             // set the unsafe "lock"
    lNetLog("BLE: Starting...\n"); // notify to log
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
                if ( round(bmaTemp) != round(currValFloat) ) {
                    //lNetLog("BLE: Notify BMA Temperature: %.1f ºC\n",bmaTemp);
                    lBMATempCharacteristic->setValue(bmaTemp);
                    lBMATempCharacteristic->notify();
                }
            }
        },BMA_EVENT_TEMP);
    }

    uint8_t BLEAddress[6];                  // 6 octets are the BLE address
    esp_read_mac(BLEAddress,ESP_MAC_BT);    // get from esp-idf :-*

    char BTName[14] = { 0 };                // buffer for build the name like: "lunokIoT_69fa"
    sprintf(BTName,"lunokIoT_%02x%02x", BLEAddress[4], BLEAddress[5]); // add last MAC bytes as name

    // Create the BLE Device
    BLEDevice::init(std::string(BTName)); // hate strings
    lNetLog("BLE: Device name: '%s'\n",BTName); // notify to log

    BLEDevice::setSecurityAuth(true,true,true);
    uint32_t generatedPin=random(0,999999);
    lNetLog("BLE: generated PIN: %06d\n",generatedPin);
    BLEDevice::setSecurityPasskey(generatedPin);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
    // create GATT the server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new LBLEServerCallbacks(),true); // destroy on finish
 
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

    xTaskCreatePinnedToCore(BLELoopTask, "lble", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL,0);
    //Serial.println("Waiting a client connection to notify...");
    vTaskDelete(NULL);
}

static void BLEWake(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    StartBLE();
}

static void BLEDown(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    StopBLE();
    //delay(100);
}

void BLESetupHooks() {
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_WAKE, BLEWake, nullptr, NULL);
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_LIGHTSLEEP, BLEDown, nullptr, NULL);
}


void BLEKickAllPeers() {
    if ( false == bleEnabled ) { return; }
    if ( nullptr != pServer ) {
        //DISCONNECT THE CLIENTS
        std::vector<uint16_t> clients = pServer->getPeerDevices();
        for(uint16_t client : clients) {
            delay(100);
            lNetLog("Network: BLE kicking out client %d\n",client);
            pServer->disconnect(client,0x13); // remote close
            pServer->disconnect(client,0x16); // localhost close
        }
    }

}


// https://h2zero.github.io/NimBLE-Arduino/index.html
// https://github.com/h2zero/NimBLE-Arduino


static void BLEStopTask(void* args) {
    lNetLog("BLE: Stopping...\n");
    BLEKickAllPeers();
    /*
    pServer->removeService(pService,true);
    pService = nullptr;
    if ( nullptr != pTxCharacteristic ) {
        delete pTxCharacteristic;
        pTxCharacteristic=nullptr;
    }
    */
   
    if ( BLEDevice::getScan()->isScanning()) {
        lNetLog("BLE: Waiting scan stops...\n");
        BLEDevice::getScan()->stop();
        BLEDevice::getScan()->clearResults();
        BLEDevice::getScan()->clearDuplicateCache();
        //delay(100);
    }

    bleEnabled = false; // notify end task
    if ( bleServiceRunning ) {
        lNetLog("BLE: Waiting service stops...\n");
        while(bleServiceRunning) { delay(5); esp_task_wdt_reset(); }
        lNetLog("BLE: Service ended\n");
        delay(10); // do little time to execute vTaskDelete on task (profilactic delay xD)
    }
    if ( NULL != BLETaskHandler ) {
        lNetLog("BLE: Killing BLE manager task?...\n");
        vTaskDelete(BLETaskHandler);
        BLETaskHandler=NULL;
    }
    

    if ( nullptr != battPercentPublish ) {
        delete battPercentPublish;
        battPercentPublish=nullptr;
    }
    if ( nullptr != battTempPublish ) {
        delete battTempPublish;
        battTempPublish=nullptr;
    }
    if ( nullptr != envTempPublish ) {
        delete envTempPublish;
        envTempPublish=nullptr;
    }

    //pService->removeCharacteristic(pTxCharacteristic, true);
    //pServer->removeService(pService, true);
    BLEDevice::deinit(true);
    battCharacteristic = nullptr;
    lBattTempCharacteristic=nullptr;
    //delay(100);
    /*
    pServer = nullptr;
    pService = nullptr;
    pTxCharacteristic = nullptr;
    */
   vTaskDelete(NULL);
}

void StopBLE() {
    if ( false == bleEnabled ) { return; }
    if ( NULL != BLETaskHandler ) {
        lNetLog("BLE: Already stopped?\n");
        return;
    }
    BaseType_t taskOK = xTaskCreatePinnedToCore( BLEStopTask,
                        "bleStop",
                        LUNOKIOT_TASK_STACK_SIZE,
                        nullptr,
                        tskIDLE_PRIORITY,
                        NULL,
                        1);
    if ( pdPASS != taskOK ) {
        lNetLog("BLE: ERROR Trying to launch stop BLE Task\n");
    }
}

float ReverseFloat( const float inFloat )
{
   float retVal;
   char *floatToConvert = ( char* ) & inFloat;
   char *returnFloat = ( char* ) & retVal;

   // swap the bytes into a temporary buffer
   returnFloat[0] = floatToConvert[3];
   returnFloat[1] = floatToConvert[2];
   returnFloat[2] = floatToConvert[1];
   returnFloat[3] = floatToConvert[0];

   return retVal;
}

void StartBLE() {
    bool enabled = NVS.getInt("BLEEnabled");
    if ( false == enabled ) { 
        liLog("BLE: BLE disabled by user\n");
        return;
    }
    // https://h2zero.github.io/NimBLE-Arduino/nimconfig_8h_source.html
    if ( bleEnabled ) {
        lNetLog("BLE: unable to start, (earlier StartBLE() is in progress)\n");
        return;
    }  // global flag enabled, already called, safe to ignore
    if ( NULL != BLETaskHandler ) {
        lNetLog("BLE: ¿Task already running? refusing to launch\n");
        return;
    }
    BaseType_t taskOK = xTaskCreatePinnedToCore( BLELauncherTask,
                        "bleLauncher",
                        LUNOKIOT_TASK_STACK_SIZE,
                        nullptr,
                        tskIDLE_PRIORITY,
                        &BLETaskHandler,
                        1);
    if ( pdPASS != taskOK ) {
        lNetLog("BLE: ERROR Trying to launch Task\n");
    }

}
