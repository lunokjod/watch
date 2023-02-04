
#include <Arduino.h>
#include <LilyGoWatch.h>

#include "Network.hpp"

#include <functional>
#include <list>
//#include <NimBLEDevice.h>

#include "lunokiot_config.hpp"
#include "SystemEvents.hpp"

#include <ArduinoNvs.h> // persistent values
#include <WiFi.h>
#include "UI/UI.hpp" // for rgb unions
#include <HTTPClient.h>

#include "Network/BLE.hpp"

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
extern TFT_eSprite *screenShootCanvas;

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

// Network scheduler list
std::list<NetworkTaskDescriptor *> networkPendingTasks = {};


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

    BaseType_t taskOK = xTaskCreatePinnedToCore(NetworkTaskRun,"",LUNOKIOT_TASK_STACK_SIZE,NULL,uxTaskPriorityGet(NULL), NULL,0);
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
