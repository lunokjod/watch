#include <Arduino.h>
#include <ArduinoNvs.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <esp_wifi.h>
#include <WiFi.h>

#include "LocalCloud.hpp"
#include "Network.hpp"
#include "../lunokiot_config.hpp"
#include "UI/UI.hpp"

NetworkTaskDescriptor * localCloudNetworkTask = nullptr;
unsigned int localCloudNetworkTaskLastCheck = -1;
JSONVar localCloudNetworkHome;
char localCloudNetworkID[32];

void ParseLocalCloudHomeJSON() {
    // get info from my own profile
    if (localCloudNetworkHome.hasOwnProperty("profile")) {
        JSONVar profileJSON = localCloudNetworkHome["profile"];
        if ( profileJSON.hasOwnProperty("sessionID") ) {
            Serial.printf("LocalCloud: Identity: '%s'\n",(const char*)(profileJSON["sessionID"]));
            String shitNVSStringerSux = NVS.getString("lcSession");
            if ( 0 == shitNVSStringerSux.length() ) {
                Serial.printf("LocalCloud: Identity '%s' saved on NVS\n",(const char*)(profileJSON["sessionID"]));
                String sessionIDAsString = String((const char*)profileJSON["sessionID"]);
                NVS.setString("lcSession",sessionIDAsString); // get a copy of my "session cookie"
            }
        }
    }
    // get notifications from lunoKloud
    if ( localCloudNetworkHome.hasOwnProperty("notifications") ) {
        JSONVar notificationsJSON = localCloudNetworkHome["notifications"];
        pendingNotifications = notificationsJSON.length();
        Serial.printf("LocalCloud: Notifications received: %d\n",pendingNotifications);
    }
    Serial.println("");
    Serial.println("Received:");
    String jsonString = JSON.stringify(localCloudNetworkHome);
    Serial.printf("\n'%s'\n", jsonString.c_str());
    Serial.println("");
}

void StartLocalCloudClientTask(void * data) {
    delay(5000); // 5 seconds first delay
    if ( nullptr == localCloudNetworkTask ) {
        localCloudNetworkTask = new NetworkTaskDescriptor();
        localCloudNetworkTask->name = (char *)"LocalCloud";
        localCloudNetworkTask->everyTimeMS = ((1000*60)*29);
        localCloudNetworkTask->_lastCheck=millis();
        localCloudNetworkTask->_nextTrigger=0; // launch NOW if no synched never again
        localCloudNetworkTask->callback = [&]() {
            // check news and updates
            uint8_t eth_mac[6];
            const char *ssid_prefix = "lunokIoT_";
            esp_wifi_get_mac(WIFI_IF_AP, eth_mac);
            snprintf(localCloudNetworkID, 32, "%s%02X%02X%02X",
                    ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
            char *urlBuffer = (char*)ps_malloc(255);
            sprintf(urlBuffer,"%s/lunokIoT/lcHome.cgi",LUNOKIOT_LOCAL_CLOUD_URL);
            Serial.printf("LocalCloud: Trying to reach lunokIoT Local Cloud at '%s'\n",urlBuffer);
            String shitNVSStringerSux = NVS.getString("lcSession");
            HTTPClient localCloudHTTPClient;
            localCloudHTTPClient.begin(urlBuffer);
            if ( 0 != shitNVSStringerSux.length() ) {
                Serial.println("LocalCloud: Reusing NVS identity");
                String moreSHit = "lunokWatchSession=" + shitNVSStringerSux;
                localCloudHTTPClient.addHeader("Cookie",moreSHit);
            }
            localCloudHTTPClient.setUserAgent(localCloudNetworkID);
            int httpCode = localCloudHTTPClient.GET();
            free(urlBuffer);
            urlBuffer=nullptr;
            if(httpCode > 0) {
                String IHateTheStrings;
                if(httpCode == HTTP_CODE_OK) {
                    IHateTheStrings = localCloudHTTPClient.getString();
                    //Serial.printf("DEBUUUG: '%s'\n",IHateTheStrings.c_str());
                    localCloudNetworkHome = JSON.parse(IHateTheStrings);
                    if (JSON.typeof(localCloudNetworkHome) != "undefined") {
                        localCloudHTTPClient.end();
                        localCloudNetworkTaskLastCheck=millis();
                        ParseLocalCloudHomeJSON();
                        return true;
                    }
                    Serial.printf("LocalCloud: Unable to parse JSON: '%s'\n", IHateTheStrings.c_str());
                }
            } else {
                Serial.printf("LocalCloud: [HTTP] GET... failed, error: '%s'\n", localCloudHTTPClient.errorToString(httpCode).c_str());
            }
            localCloudHTTPClient.end();
            return false;
        };
        AddNetworkTask(localCloudNetworkTask);
    }
    vTaskDelete( NULL );
}

void StartLocalCloudClient() {
    xTaskCreatePinnedToCore(StartLocalCloudClientTask, "", LUNOKIOT_TASK_STACK_SIZE,nullptr, uxTaskPriorityGet(NULL), NULL,1);
}
