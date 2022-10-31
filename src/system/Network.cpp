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

#include "lunokiot_config.hpp"
#include "SystemEvents.hpp"

#include <NimBLEDevice.h>
#include <NimBLELog.h>

#include <ArduinoNvs.h> // persistent values
#include <WiFi.h>
#include "UI/UI.hpp" // for rgb unions
#include <HTTPClient.h>

#ifdef LUNOKIOT_LOCAL_CLOUD_ENABLED
#include "LocalCloud.hpp"
#endif

BLEServer *pServer = nullptr;
BLEService *pService = nullptr;
BLECharacteristic * pTxCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
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

/*
 * Stop desired task from network scheduler
 */
bool RemoveNetworkTask(NetworkTaskDescriptor *oldTsk) {
    Serial.printf("RemoveNetworkTask: Task %p '%s' removed\n", oldTsk,oldTsk->name);
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
            Serial.printf("AddNetworkTask: Task %p '%s' already on the list (offset: %d)\n", tsk,tsk->name, offset);
            return false;
        }
        offset++;
    }
    networkPendingTasks.push_back(nuTsk);
    Serial.printf("AddNetworkTask: Task %p '%s' added (offset: %d)\n", nuTsk,nuTsk->name, offset);
    return true;
}
/*
 * Search for update system network task
 */
char * latestBuildFoundString = nullptr;

void SystemUpdateAvailiable() {
    Serial.println("@TODO notify user for update");
    Serial.printf("@TODO availiable: '%s' running: '%s'\n",latestBuildFoundString,LUNOKIOT_BUILD_STRING);
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
                        Serial.printf("SearchUpdate: https get '%s'...\n", serverPath.c_str());
                        // start connection and send HTTP header
                        int httpCode = https.GET();

                        // httpCode will be negative on error
                        if (httpCode > 0) {
                            // HTTP header has been send and Server response header has been handled
                            Serial.printf("SearchUpdate: https get code: %d\n", httpCode);
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
                                    Serial.printf("SearchUpdate: Received: '%s' current: '%s'\n",latestBuildFoundString,LUNOKIOT_BUILD_STRING);
                                    SystemUpdateAvailiable();
                                }
                            }
                        } else {
                            Serial.printf("SearchUpdate: http get failed, error: %s\n", https.errorToString(httpCode).c_str());
                            https.end();
                            delete client;
                            return false;
                        }
                        https.end();
                    } else {
                        Serial.println("SearchUpdate: Unable to connect");
                        delete client;
                        return false;
                    }
                }
                delete client;
                return true;
            }
            Serial.println("Watchface: Weather: Unable to create WiFiClientSecure");
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
            Serial.println("Network: Timed WiFi connection procedure begin");
            bool mustStart = false;

            for (auto const& tsk : networkPendingTasks) {
                //CHECK THE TIMEOUTS
                if ( -1 == tsk->_nextTrigger ) {
                    tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                } else {
                    if ( tsk->enabled ) {
                        if ( millis() > tsk->_nextTrigger ) {
                            Serial.printf("NetworkTask: Pending task '%s'\n", tsk->name);
                            mustStart = true;
                        }
                    } 
                }
                tsk->_lastCheck = millis();
            }
            if ( mustStart ) {
                Serial.println("Network: BLE must be disabled to maximize WiFi effort");
                if ( bleEnabled ) { StopBLE(); }
                delay(100);
                WiFi.begin();
                //WiFi.setAutoReconnect(false);
            } else {
                Serial.println("Network: No tasks pending for this period... don't launch WiFi");
                for (auto const& tsk : networkPendingTasks) {
                    Serial.printf("NetworkTask: Task '%s' In: %d secs\n", tsk->name, (tsk->_nextTrigger-millis())/1000);
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
                            Serial.printf("NetworkTask: Running task '%s'...\n", tsk->name);
                            bool res = tsk->callback();
                            if ( res ) {
                                tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                                tsk->_lastCheck = millis();
                            } else { 
                                failedTasks.push_back(tsk);
                                Serial.printf("NetworkTask: Task '%s' FAILED (wait a bit)\n", tsk->name);
                            }
                        }
                    }
                }
                if ( failedTasks.size() > 0 ) {
                    Serial.println("NetworkTask: Retrying failed tasks...");
                    delay(1000);
                    for (auto const& tsk : failedTasks) {
                        if ( -1 == tsk->_nextTrigger ) {
                            tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                        } else {
                            if ( millis() > tsk->_nextTrigger ) {
                                delay(250);
                                Serial.printf("NetworkTask: Running task (more slow) '%s'...\n", tsk->name);
                                bool res = tsk->callback();
                                if ( res ) {
                                    tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                                    tsk->_lastCheck = millis();
                                } else { 
                                    Serial.printf("NetworkTask: Task '%s' FAILED (retry on next network event)\n", tsk->name);
                                }
                            }
                        }
                    }
                }
                WiFi.disconnect();
                delay(100);
                WiFi.mode(WIFI_OFF);
                connectedMS = millis()-beginConnected;
                Serial.printf("Network: WiFi connection: %d sec\n", connectedMS/1000);
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
    xTaskCreate(NetworkHandlerTask, "wifi", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);
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
    bleServiceRunning=true;

    union RLEPackage {
        uint8_t bytes[5];
        struct {
            uint16_t color;
            uint8_t repeat;
            uint8_t x; // screen is 240x240, don't waste my time!
            uint8_t y;
        };
    };

    void *theScreenShotToSend=nullptr; // point to current screenshoot
    // temp values
    screenShootCurrentImageX=0;
    screenShootCurrentImageY=0;
    oldDeviceConnected = false;
    deviceConnected = false;
    while(bleEnabled) {
        if ( blePeer ) {
            if ( BLESendScreenShootCommand ) { // the client has sended "GETSCREENSHOOT" in the BLE UART
                // first iteration
                if ( false == screenShootInProgress ) {
                    theScreenShotToSend = (void*)screenShootCanvas;
                    screenShootInProgress = true;
                    Serial.println("Network: begin sending screenshoot via BLE UART");
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
                            Serial.printf("Network: Screenshot served by BLE (image: %d byte) upload: %d byte\n",realImageSize,imageUploadSize);
                            delay(100);
                            theScreenShotToSend=nullptr;
                            realImageSize=0;
                            imageUploadSize=0;
                            screenShootCurrentImageY=0;
                            screenShootCurrentImageX=0;
                            BLESendScreenShootCommand=false;
                            screenShootInProgress = false;
                            //DISCONNECT THE CLIENT
                            BLEKickAllPeers();
                            networkActivity=false;
                        }
                    }
                }
            }
        } else {
            networkActivity=false;
            delay(100);
        }
        // disconnecting
        if (!deviceConnected && oldDeviceConnected) {
            screenShootInProgress=false;
            blePeer=false;
            Serial.println("Network: BLE Device disconnected");
            delay(500); // give the bluetooth stack the chance to get things ready
            pServer->startAdvertising(); // restart advertising
            Serial.println("Network: Start BLE advertising");
            oldDeviceConnected = deviceConnected;
            networkActivity=false;
        }

        // connecting
        if (deviceConnected && !oldDeviceConnected) {
            // do stuff here on connecting
            blePeer=true;
            Serial.println("Network: BLE Device connected");
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
    Serial.println("BLE: Task ends here");
    vTaskDelete(NULL);
}

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */  
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
  /***************** New - Security handled here ********************
  ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest(){
      Serial.println("Server PassKeyRequest");
      return 123456; 
    }

    bool onConfirmPIN(uint32_t pass_key){
      Serial.print("The passkey YES/NO number: ");Serial.println(pass_key);
      return true; 
    }

    void onAuthenticationComplete(ble_gap_conn_desc desc){
      Serial.println("Starting BLE work!");
    }
  /*******************************************************************/
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            const char * receivedCommand = rxValue.c_str();
            const char * ScreenShootCommand = "GETSCREENSHOOT";

            if ( 0 == strncmp(ScreenShootCommand,receivedCommand, strlen(ScreenShootCommand))) {
                Serial.println("BLE: UART: 'GETSCREENSHOOT' command received");
                BLESendScreenShootCommand = true;
            } else {
                BLESendScreenShootCommand=false;
                Serial.printf("BLE: UART Received Value: %c",receivedCommand);
                for (int i = 0; i < rxValue.length(); i++) {
                    Serial.printf("%c",rxValue[i]);
                }
                Serial.println("");
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
        Serial.printf("Network: BLE kicking out client %d\n",client);
        pServer->disconnect(client,0x13); // remote close
        pServer->disconnect(client,0x16); // localhost close
    }
}


// https://h2zero.github.io/NimBLE-Arduino/index.html
// https://github.com/h2zero/NimBLE-Arduino
void StopBLE() {
    if ( false == bleEnabled ) { return; }
    Serial.println("BLE: Stopping...");
    BLEKickAllPeers();
    /*
    pServer->removeService(pService,true);
    pService = nullptr;
    if ( nullptr != pTxCharacteristic ) {
        delete pTxCharacteristic;
        pTxCharacteristic=nullptr;
    }
    */
    bleEnabled = false;
    if ( bleServiceRunning ) {
        Serial.println("BLE: Waiting service stops...");
        while(bleServiceRunning) { delay(1); }
    }
    //pService->removeCharacteristic(pTxCharacteristic, true);
    //pServer->removeService(pService, true);
    BLEDevice::deinit(true);
    /*
    pServer = nullptr;
    pService = nullptr;
    pTxCharacteristic = nullptr;
    */
}

void StartBLE() {
    if ( bleEnabled ) { return; }

    delay(3000); // wait 3 seconds to enable it
    if ( bleEnabled ) { vTaskDelete(NULL); }
    bleEnabled = true;
    Serial.println("BLE: Starting...");
    uint8_t BLEAddress[6];
    esp_read_mac(BLEAddress,ESP_MAC_BT);
    char BTName[30] = { 0 };
    sprintf(BTName,"lunokIoT_%02x%02x", BLEAddress[4], BLEAddress[5]);

    // Create the BLE Device
    BLEDevice::init(BTName);
    //if ( nullptr == pServer ) {
        // Create the BLE Server
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks());
    //}
    // Create the BLE Service
    //if ( nullptr == pService ) {
        pService = pServer->createService(SERVICE_UUID);
    //}

    //if ( nullptr == pTxCharacteristic ) {
        // Create a BLE Characteristic
        pTxCharacteristic = pService->createCharacteristic(
            CHARACTERISTIC_UUID_TX,
            /******* Enum Type NIMBLE_PROPERTY now *******      
            BLECharacteristic::PROPERTY_NOTIFY
            );
            **********************************************/  
            NIMBLE_PROPERTY::NOTIFY
        );
    //}
    /***************************************************   
     NOTE: DO NOT create a 2902 descriptor 
    it will be created automatically if notifications 
    or indications are enabled on a characteristic.

    pCharacteristic->addDescriptor(new BLE2902());
    ****************************************************/                  

    BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                                            CHARACTERISTIC_UUID_RX,
                                    /******* Enum Type NIMBLE_PROPERTY now *******       
                                            BLECharacteristic::PROPERTY_WRITE
                                            );
                                    *********************************************/  
                                            NIMBLE_PROPERTY::WRITE
                                            );

    pRxCharacteristic->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();

    xTaskCreate(BLELoopTask, "ble", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);

    //Serial.println("Waiting a client connection to notify...");
}
