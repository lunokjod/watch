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

#include "../app/LogView.hpp"
#include <esp_task_wdt.h>

int BLEscanTime = 5; //In seconds
unsigned long BLENextscanTime = 0;
BLEServer *pServer = nullptr;
BLEService *pService = nullptr;
BLECharacteristic * pTxCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;
SemaphoreHandle_t BLEKnowDevicesSemaphore = xSemaphoreCreateMutex();

std::list <lBLEDevice*>BLEKnowDevices;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UART_UUID      "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

extern const uint8_t githubPEM_start[] asm("_binary_asset_raw_githubusercontent_com_pem_start");
extern const uint8_t githubPEM_end[] asm("_binary_asset_raw_githubusercontent_com_pem_end");

bool bleEnabled = false;
bool bleServiceRunning = false;
bool blePeer = false;
bool BLESendScreenShootCommand = false;
// https://mynewt.apache.org/latest/network/ble_sec.html

// Network scheduler list
std::list<NetworkTaskDescriptor *> networkPendingTasks = {};

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
char * latestBuildFoundString = nullptr;

void SystemUpdateAvailiable() {
    lLog("@TODO notify user for update\n");
    lLog("@TODO availiable: '%s' running: '%s'\n",latestBuildFoundString,LUNOKIOT_BUILD_STRING);
}


NetworkTaskDescriptor * SearchUpdateNetworkTask = nullptr;
void SearchUpdateAsNetworkTask() {
    if ( nullptr == SearchUpdateNetworkTask ) {
        SearchUpdateNetworkTask = new NetworkTaskDescriptor();
        SearchUpdateNetworkTask->name = (char *)"Check updates";
        SearchUpdateNetworkTask->everyTimeMS = (((1000*60)*60)*24*7); // once a week x'D (continue dreaming!)
        SearchUpdateNetworkTask->_lastCheck=millis();
        SearchUpdateNetworkTask->_nextTrigger=0; // launch NOW if no synched never again
        SearchUpdateNetworkTask->callback = [&]() {
            // @NOTE to get the PEM from remote server:
            // openssl s_client -connect raw.githubusercontent.com:443 -showcerts </dev/null | openssl x509 -outform pem > raw_githubusercontent_com.pem
            // echo "" | openssl s_client -showcerts -connect raw.githubusercontent.com:443 | sed -n "1,/Root/d; /BEGIN/,/END/p" | openssl x509 -outform PEM >raw_githubusercontent_com.pem
            WiFiClientSecure *client = new WiFiClientSecure;
            if (nullptr != client ) {
                client->setCACert((const char*)githubPEM_start);
                { // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
                    HTTPClient https;
                    https.setConnectTimeout(8*1000);
                    https.setTimeout(8*1000);
                    //https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
                    String serverPath = LastVersionURL;
                    if (https.begin(*client, serverPath)) {
                        // HTTPS
                        lNetLog("SearchUpdate: https get '%s'...\n", serverPath.c_str());
                        // start connection and send HTTP header
                        int httpCode = https.GET();

                        // httpCode will be negative on error
                        if (httpCode > 0) {
                            // HTTP header has been send and Server response header has been handled
                            lNetLog("SearchUpdate: https get code: %d\n", httpCode);
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
                                    SystemUpdateAvailiable();
                                }
                            }
                        } else {
                            lNetLog("SearchUpdate: http get failed, error: %s\n", https.errorToString(httpCode).c_str());
                            https.end();
                            delete client;
                            return false;
                        }
                        https.end();
                    } else {
                        lNetLog("SearchUpdate: Unable to connect\n");
                        delete client;
                        return false;
                    }
                }
                delete client;
                return true;
            }
            lNetLog("Watchface: Weather: Unable to create WiFiClientSecure\n");
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
static void NetworkHandlerTask(void* args) {
    delay(8000); // arbitrary wait before begin first connection
    unsigned long nextConnectMS = 0;
    unsigned long beginConnected = -1;
    unsigned long beginIdle = -1;
    while(true) {
        delay(1000); // allow other tasks to do their chance :)

        if ( systemSleep ) { continue; } // fuck off... giveup/sleep in progress... (shaded area)
        if ( false == provisioned ) { continue; } // nothing to do without provisioning :(
        
        if ( millis() > nextConnectMS ) { // begin connection?
            if ( false == NVS.getInt("WifiEnabled") ) {
                nextConnectMS = millis()+ReconnectPeriodMs;
                continue;
            }
            //check the pending tasks... if no one, don't connect
            lNetLog("Network: Timed WiFi connection procedure begin\n");
            bool mustStart = false;

            for (auto const& tsk : networkPendingTasks) {
                //CHECK THE TIMEOUTS
                if ( -1 == tsk->_nextTrigger ) {
                    tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                } else {
                    if ( tsk->enabled ) {
                        if ( millis() > tsk->_nextTrigger ) {
                            lNetLog("NetworkTask: Pending task '%s'\n", tsk->name);
                            mustStart = true;
                        }
                    } 
                }
                tsk->_lastCheck = millis();
            }
            if ( mustStart ) {
                lNetLog("Network: BLE must be disabled to maximize WiFi effort\n");
                if ( bleEnabled ) { StopBLE(); }
                delay(100);
                WiFi.begin();
                //WiFi.setAutoReconnect(false);
            } else {
                lNetLog("Network: No tasks pending for this period... don't launch WiFi\n");
                for (auto const& tsk : networkPendingTasks) {
                    lNetLog("NetworkTask: Task '%s' In: %d secs\n", tsk->name, (tsk->_nextTrigger-millis())/1000);
                }
            }
            nextConnectMS = millis()+ReconnectPeriodMs;
            continue;
        }

        wl_status_t currStat = WiFi.status();
        /*
        if ( WL_IDLE_STATUS == currStat) {
            if ( -1 == beginIdle ) {
                beginIdle = millis();
            } else {
                unsigned long idleMS = millis()-beginIdle;
                if ( idleMS > NetworkTimeout ) {
                    Serial.printf("Network: WiFi TIMEOUT (idle) at %d sec\n", idleMS/1000);
                    WiFi.disconnect();
                    WiFi.mode(WIFI_OFF);
                    beginIdle=-1;
                    continue;
                }
                Serial.printf("Network: WiFi idle: %d sec\n", idleMS/1000);
            }
            continue;
        } else */
        delay(10);
        if ( WL_CONNECTED == currStat) {
            if ( -1 == beginConnected ) {
                beginConnected = millis();
            } else {
                unsigned long connectedMS = millis()-beginConnected;
                /*
                if ( connectedMS > NetworkTimeout ) {
                    Serial.println("Network: WiFi online TIMEOUT");
                    WiFi.disconnect(true);
                    delay(100);
                    WiFi.mode(WIFI_OFF);
                    beginConnected=-1;
                    continue;
                }*/
                //CHECK THE tasks

                std::list<NetworkTaskDescriptor *> failedTasks = {};
                for (auto const& tsk : networkPendingTasks) {
                    if ( -1 == tsk->_nextTrigger ) {
                        tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                    } else {
                        if ( millis() > tsk->_nextTrigger ) {
                            delay(150);
                            lNetLog("NetworkTask: Running task '%s'...\n", tsk->name);
                            bool res = tsk->callback();
                            if ( res ) {
                                tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                                tsk->_lastCheck = millis();
                            } else { 
                                failedTasks.push_back(tsk);
                                lNetLog("NetworkTask: Task '%s' FAILED (wait a bit)\n", tsk->name);
                            }
                        }
                    }
                }
                if ( failedTasks.size() > 0 ) {
                    lNetLog("NetworkTask: Retrying failed tasks...\n");
                    delay(1000);
                    for (auto const& tsk : failedTasks) {
                        if ( -1 == tsk->_nextTrigger ) {
                            tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                        } else {
                            if ( millis() > tsk->_nextTrigger ) {
                                delay(250);
                                lNetLog("NetworkTask: Running task (more slow) '%s'...\n", tsk->name);
                                bool res = tsk->callback();
                                if ( res ) {
                                    tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                                    tsk->_lastCheck = millis();
                                } else { 
                                    lNetLog("NetworkTask: Task '%s' FAILED (retry on next network event)\n", tsk->name);
                                }
                            }
                        }
                    }
                }
                WiFi.disconnect();
                delay(100);
                WiFi.mode(WIFI_OFF);
                connectedMS = millis()-beginConnected;
                lNetLog("Network: WiFi connection: %d sec\n", connectedMS/1000);
            }
            continue;
        }
        // empty loop, use it to reset all counters
        beginConnected = -1;
        beginIdle = -1;
    }
    vTaskDelete(NULL); // never return
}

/*
 * Create the task for network scheduler
 * How it works?
 * * Call NetworkHandler on setup()
 * * Later on create a new NetworkTaskDescriptor() (in heap!)
 * * Push to the scheduler using AddNetworkTask()
 * * Note: ReconnectPeriodMs determines minimal period of scheduling (keep it high for battery saving)
 */
bool NetworkHandler() {
    provisioned = (bool)NVS.getInt("provisioned"); // initial value load
#ifdef LUNOKIOT_WIFI_ENABLED
    xTaskCreate(NetworkHandlerTask, "lwifi", LUNOKIOT_APP_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);
#ifdef LUNOKIOT_UPDATES_ENABLED
    SearchUpdateAsNetworkTask();
#endif
#endif
#ifdef LUNOKIOT_BLE_ENABLED
    BLESetupHooks();
#endif

#ifdef LUNOKIOT_LOCAL_CLOUD_ENABLED
    StartLocalCloudClient();
#endif

    if ( provisioned ) { return true; }
    return false;
}

static void BLEWake(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    bool enabled = NVS.getInt("BLEEnabled");
    if ( enabled ) {
        StartBLE();
    }
}

static void BLEDown(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    StopBLE();
    delay(100);
}

void BLESetupHooks() {
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_WAKE, BLEWake, nullptr, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_LIGHTSLEEP, BLEDown, nullptr, NULL));

}
bool networkActivity = false;
extern TFT_eSprite *screenShootCanvas;    // the last screenshoot
extern bool screenShootInProgress;        // Notify Watchface about send screenshoot
size_t imageUploadSize=0;
size_t realImageSize=0;
rgb565 myLastColor = { .u16 = (uint16_t)TFT_TRANSPARENT }; // improbable color
uint16_t lastRLECount = 0;

void BLELoopTask(void * data) {
    bleServiceRunning=true; // notify outside that I'm running

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
                    return true;
                }
                return false;
            });
            xSemaphoreGive( BLEKnowDevicesSemaphore );
        }
        if ( blePeer ) {
            if ( BLESendScreenShootCommand ) { // the client has sended "GETSCREENSHOOT" in the BLE UART
                // first iteration
                if ( false == screenShootInProgress ) {
                    theScreenShotToSend = (void*)screenShootCanvas;
                    screenShootInProgress = true;
                    lNetLog("Network: begin sending screenshoot via BLE UART\n");
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
        }
        // disconnecting
        if (!deviceConnected && oldDeviceConnected) {
            screenShootInProgress=false;
            blePeer=false;
            lNetLog("Network: BLE Device disconnected\n");
            //delay(500); // give the bluetooth stack the chance to get things ready
            pServer->startAdvertising(); // restart advertising
            lNetLog("Network: Start BLE advertising\n");
            oldDeviceConnected = deviceConnected;
            networkActivity=false;
        }

        // connecting
        if (deviceConnected && !oldDeviceConnected) {
            // do stuff here on connecting
            blePeer=true;
            lNetLog("Network: BLE Device connected\n");
            oldDeviceConnected=deviceConnected;
            networkActivity=true;
        }
    }
    networkActivity=false;
    screenShootInProgress=false;
    deviceConnected = false;
    oldDeviceConnected = false;
    bleServiceRunning=false;
    blePeer = false;
    lNetLog("BLE: Task ends here\n");
    vTaskDelete(NULL);
}

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */  
class LBLEServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        lNetLog("BLE: Server client connect\n");
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        lNetLog("BLE: Server client disconnect\n");
        deviceConnected = false;
    }
  /***************** New - Security handled here ********************
  ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest(){
      lNetLog("BLE: Server PassKeyRequest @TODO\n");
      return 123456; 
    }

    bool onConfirmPIN(uint32_t pass_key){
      lNetLog("BLE: The passkey YES/NO number: '%s'\n",pass_key);
      return true; 
    }

    void onAuthenticationComplete(ble_gap_conn_desc desc){
      lNetLog("BLE: Starting BLE work!\n");
    }
  /*******************************************************************/
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

            if ( 0 == strncmp(PushMessageCommand,receivedCommand, strlen(ScreenShootCommand))) {
                lNetLog("BLE: UART: '%s' command received\n",PushMessageCommand);
            } else if ( 0 == strncmp(ScreenShootCommand,receivedCommand, strlen(ScreenShootCommand))) {
                lNetLog("BLE: UART: '%s' command received\n",ScreenShootCommand);
                BLESendScreenShootCommand = true;
            } else {
                BLESendScreenShootCommand=false;
                lNetLog("BLE: UART Received Value: %c",receivedCommand);
                for (int i = 0; i < rxValue.length(); i++) {
                    lNetLog("%c",rxValue[i]);
                }
                lNetLog("\n");
            }
        }
    }
};

void BLEKickAllPeers() {
    if ( false == bleEnabled ) { return; }
    //DISCONNECT THE CLIENTS
    std::vector<uint16_t> clients = pServer->getPeerDevices();
    for(uint16_t client : clients) {
        delay(100);
        lNetLog("Network: BLE kicking out client %d\n",client);
        pServer->disconnect(client,0x13); // remote close
        pServer->disconnect(client,0x16); // localhost close
    }
}


// https://h2zero.github.io/NimBLE-Arduino/index.html
// https://github.com/h2zero/NimBLE-Arduino
void StopBLE() {
    if ( false == bleEnabled ) { return; }
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
        delay(100);
    }

    bleEnabled = false;
    if ( bleServiceRunning ) {
        lNetLog("BLE: Waiting service stops...\n");
        while(bleServiceRunning) { delay(5); esp_task_wdt_reset(); }
        lNetLog("BLE: Service ended\n");

    }
 

    //pService->removeCharacteristic(pTxCharacteristic, true);
    //pServer->removeService(pService, true);
    BLEDevice::deinit(true);
    //delay(100);
    /*
    pServer = nullptr;
    pService = nullptr;
    pTxCharacteristic = nullptr;
    */
}

void StartBLE() {
    if ( bleEnabled ) { return; }  // global flag enabled, already called, safe to ignore
    bleEnabled = true;             // set the unsafe "lock"
    lNetLog("BLE: Starting...\n"); // notify to log

    uint8_t BLEAddress[6];                  // 6 octets are the BLE address
    esp_read_mac(BLEAddress,ESP_MAC_BT);    // get from esp-idf :-*

    char BTName[14] = { 0 };                // buffer for build the name like: "lunokIoT_69fa"
    sprintf(BTName,"lunokIoT_%02x%02x", BLEAddress[4], BLEAddress[5]); // add last MAC bytes as name

    // Create the BLE Device
    BLEDevice::init(std::string(BTName)); // hate strings

    // create the server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new LBLEServerCallbacks(),true); // destroy on finish

    // Create the BLE Service UART
    pService = pServer->createService(SERVICE_UART_UUID);

    //if ( nullptr == pTxCharacteristic ) {
        // Create a BLE Characteristic
        pTxCharacteristic = pService->createCharacteristic(
            CHARACTERISTIC_UUID_TX,
            NIMBLE_PROPERTY::NOTIFY
        );

    // UART data come here
    BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                                            CHARACTERISTIC_UUID_RX,NIMBLE_PROPERTY::WRITE);

    pRxCharacteristic->setCallbacks(new LBLEUARTCallbacks()); // @TODO here can be a leak

    // Start the service
    pService->start();

    // Start advertising
    NimBLEAdvertising *adv = pServer->getAdvertising();
    adv->setAppearance(CONFIG_BT_NIMBLE_SVC_GAP_APPEARANCE);
    adv->start();

    BLEScan * pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new LBLEAdvertisedDeviceCallbacks(), true);
    pBLEScan->setActiveScan(false); //active scan uses more power
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);  // less or equal setInterval value
    pBLEScan->setMaxResults(3); // dont waste memory with cache
    //pBLEScan->setDuplicateFilter(false);

    xTaskCreate(BLELoopTask, "ble", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);

    //Serial.println("Waiting a client connection to notify...");
}
