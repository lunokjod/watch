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

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "WiFi.hpp"
#include "../../app/LogView.hpp"
#include "../SystemEvents.hpp" 
#include <ArduinoNvs.h>
#include <Ticker.h>

#include <functional>
#include <list>
#include "BLE.hpp"
#include <esp_task_wdt.h>
#include "../../lunokIoT.hpp"

//void LoTWiFi::SuspendTasks() { xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS); disabled=true; }
//void LoTWiFi::ResumeTasks() { xSemaphoreGive( taskLock ); disabled=false; }
//extern bool provisioned;

void LoTWiFi::Enable() {
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    enabled=true;
    xSemaphoreGive( taskLock );
}

void LoTWiFi::Disable() {
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    enabled=false;
    xSemaphoreGive( taskLock );
}

bool LoTWiFi::RadioInUse() {
    wl_status_t current = WiFi.status();
    if ((WL_NO_SHIELD == current)||(WL_IDLE_STATUS == current)) { return false; }
    return true;
}

bool LoTWiFi::InUse() {
    bool response;
    BaseType_t res = xSemaphoreTake( taskLock, LUNOKIOT_EVENT_FAST_TIME_TICKS);
    if ( pdFALSE == res ) { return true; } // locked, ergo in use x'D
    response=running;
    xSemaphoreGive( taskLock );
    return response;
}

bool LoTWiFi::IsEnabled() {
    bool response;
    BaseType_t res = xSemaphoreTake( taskLock, LUNOKIOT_EVENT_FAST_TIME_TICKS);
    if ( pdFALSE == res ) { return true; } // locked, ergo in use x'D
    response=enabled;
    xSemaphoreGive( taskLock );
    return response;
}

bool LoTWiFi::AddTask(LoTWiFiTask * newTask) {
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    size_t offset = 0;
    for (auto const& tsk : tasks) {
        if ( tsk == newTask ) {
            lNetLog("WiFi: %p Task %p '%s' already on the list (offset: %d)\n",this,newTask,tsk->Name(), offset);
            xSemaphoreGive( taskLock );
            return false;
        }
        offset++;
    }
    tasks.push_back(newTask);
    lNetLog("WiFi: %p Task %p '%s' added (offset: %d)\n",this, newTask,newTask->Name(), offset);
    xSemaphoreGive( taskLock );
    return true;
}

bool LoTWiFi::RemoveTask(LoTWiFiTask * newTask) {
    lLog("@TODO REMOVETASK NOT IMPLEMENTED\n");
    return false;

}
size_t LoTWiFi::GetConnections() { return credentials.size(); }

//@TODO 
char * LoTWiFi::GetSSID(size_t offset) {
    return nullptr;
}

char * LoTWiFi::GetPWD(size_t offset) {
    return nullptr;
}

bool LoTWiFi::Provisioned() {
    if ( GetConnections() ) { return true; }
    return false;
}

void LoTWiFi::AddConnection(const char *SSID,const char *password) {
    LoTWiFiCredentials * cred = new LoTWiFiCredentials();
    char *nSSID = (char *)ps_malloc(strlen(SSID)+1);
    char *nPwd = (char *)ps_malloc(strlen(password)+1);
    sprintf(nSSID,"%s",SSID);
    sprintf(nPwd,"%s",password);
    cred->SSID=nSSID;
    cred->Password=nPwd;
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    credentials.push_back(cred);
    NVS.setInt(String(SystemSettings::KeysAsString[SystemSettings::SettingKey::WiFiCredentialsNumber]),int(GetConnections()),true);
    //LoT().GetSettings()->SetInt(SystemSettings::SettingKey::WiFiCredentialsNumber,int(GetConnections()));
    lNetLog("WiFi: %p Added connection %u SSID: '%s' Password: '%s'\n", this, GetConnections(), nSSID, nPwd);
    char labelName[255];
    sprintf(labelName,"SSID_%u",GetConnections());
    NVS.setString(String(labelName),String(nSSID));
    sprintf(labelName,"WIFIPWD_%u",GetConnections());
    NVS.setString(String(labelName),String(nPwd));
    wifiMulti.addAP(nSSID,nPwd);
    //provisioned=true;
    int shit = LoT().GetSettings()->GetInt(SystemSettings::SettingKey::WiFiCredentialsNumber);
    lLog("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA: %d\n",shit);
    xSemaphoreGive( taskLock );
}

void LoTWiFi::PerformTasks() {
    // check user permission
    if ( false == NVS.getInt("WifiEnabled") ) {
        //lNetLog("WiFi: %p User settings don't agree\n",this);
        if ( IsEnabled() ) { Disable(); }
        return;
    }
    lNetLog("WiFi: %p Tasks check\n", this);
    if ( IsDisabled() ) {
        lNetLog("WiFi: %p WARNING: Instance disabled\n",this);
        return;
    }
    /*
    //check provisioning
    if ( false == Provisioned() ) {
        lNetLog("WiFi: not provisioned\n");
        return;
    }
    
    size_t currentWiFiOffset=0;
    while ( currentWiFiOffset < GetConnections()) {
        lLog("@DEBUG TRYING TO CONNECT TO OFFSET: %u\n", currentWiFiOffset);
        currentWiFiOffset++;
    }
    return;
    // @TODO    
    String WSSID = NVS.getString("provSSID");
    String WPass = NVS.getString("provPWD");
     */
    /*
    String WSSID = NVS.getString("provSSID");
    String WPass = NVS.getString("provPWD");
    if (( 0 == WSSID.length() )||( 0 == WPass.length() )) {
        lNetLog("WiFi: %p WARNING: Device not provisioned\n",this);
        return;
    }
    */
    // set lower priority
    //UBaseType_t myPriority = uxTaskPriorityGet(NULL);
    //vTaskPrioritySet(NULL,tskIDLE_PRIORITY);
    //taskYIELD();
    //check tasks
    //lNetLog("WiFi: %p Checking pending tasks....\n",this);
    if ( pdFALSE == xSemaphoreTake( taskLock, LUNOKIOT_EVENT_TIME_TICKS) ) { return; }
    size_t tasksMustRun=0;
    std::list<LoTWiFiTask *> * tasksNeedToRun = new std::list<LoTWiFiTask *>();
    for (auto const& tsk : tasks) {
        //launch if need
        if ( false == tsk->Check() ) { continue; }
        tasksNeedToRun->push_back(tsk);
        tasksMustRun++;
    }
    if ( 0 == tasksMustRun ) {
        running=false;
        xSemaphoreGive( taskLock );
        delete tasksNeedToRun;
        //vTaskPrioritySet(NULL,myPriority);
        //taskYIELD();
        // trigger next call
        return;
    }
    lNetLog("WiFi: %p Tasks pending: %u\n",this,tasksMustRun);
    running=true;
    xSemaphoreGive( taskLock );
    // perform connection
    wl_status_t currStat;
    bool connected = false;
    lNetLog("WiFi: %p trying to connect ...\n",this);
    if ( WL_CONNECTED == wifiMulti.run() ) { connected = true; }
    /*
    currStat = WiFi.begin(WSSID.c_str(),WPass.c_str());
    bool connected=false;
    unsigned long timeout = millis()+ConnectionTimeoutMs;
    while(timeout>millis()) {
        currStat = WiFi.status();
        lNetLog("WiFi: %p Trying to connect (status: %d)...\n",this,currStat);
        if ( WL_CONNECTED == currStat) { connected=true; break; }
        delay(2000);
        //taskYIELD();
    }*/
    if (false == connected) {
        xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        esp_task_wdt_reset();
        WiFi.disconnect(true);
        delete tasksNeedToRun;
        delay(100);
        WiFi.mode(WIFI_MODE_NULL);
        //vTaskPrioritySet(NULL,myPriority);
        //taskYIELD();
        running=false;
        xSemaphoreGive( taskLock );
        return;
    }

    // run pending tasks
    size_t tasksRunned=0;
    for (auto const& tsk : *tasksNeedToRun) {
        lNetLog("WiFi: %p Task '%s' began\n",this,tsk->Name());
        tsk->Launch();
        tasksRunned++;
    }
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    // disconnect
    WiFi.disconnect(true);
    delete tasksNeedToRun;
    delay(100);
    WiFi.mode(WIFI_MODE_NULL);
    running=false;
    xSemaphoreGive( taskLock );

    lNetLog("WiFi: %p Tasks performed: %u/%u\n", this,tasksMustRun,tasksRunned);
    // return to my priority
    //vTaskPrioritySet(NULL,myPriority);
    //taskYIELD();
    FreeSpace();
}

void LoTWiFi::InstallTimer() {
    NetworkHeartbeatTicker.attach<LoTWiFi*>(HeartBeatTime,[](LoTWiFi* instance){
        if ( systemSleep ) { return; }
        /*
        // wait until system brings up
        unsigned long timeOut=millis()+30000; // 30 secs
        while ( systemSleep ) {
            // timeout
            if ( millis() > timeOut ) { return; }
            TickType_t nextCheck = xTaskGetTickCount();
            xTaskDelayUntil( &nextCheck, (2000 / portTICK_PERIOD_MS) );
        }*/
        xTaskCreatePinnedToCore([](void *obj) { // in core 0 please
            LoTWiFi* instance = (LoTWiFi*)obj;
            //instance->NetworkHeartbeatTicker.detach(); // stop the timer
            instance->PerformTasks();
            //instance->InstallTimer(); // isn't recursive :P
            vTaskDelete(NULL);
        },"lwifi",LUNOKIOT_TASK_STACK_SIZE,instance,tskIDLE_PRIORITY, NULL,WIFICORE);
    },this);
}

LoTWiFi::LoTWiFi() {
    lNetLog("WiFi: %p start\n", this);
    bool canUse = NVS.getInt("WifiEnabled");
    if ( canUse ) { Enable(); }
    else { Disable(); }
    int maxWifis = LoT().GetSettings()->GetInt(SystemSettings::SettingKey::WiFiCredentialsNumber);
    /*
    if ( -1 == maxWifis ) {
        lNetLog("WiFi: not provisioned!\n");
        return;
    }*/
    if ( 0 == maxWifis ) {
        lNetLog("WiFi: not provisioned!\n");
        return;
    }
    size_t currentWiFiOffset=0;
    while ( currentWiFiOffset <= maxWifis ) {
        char labelName[255];
        sprintf(labelName,"SSID_%u",currentWiFiOffset);
        String nSSID = NVS.getString(String(labelName));
        sprintf(labelName,"WIFIPWD_%u",currentWiFiOffset);
        String nPwd = NVS.getString(String(labelName));
        lNetLog("WiFi: %p Add connection: %u SSID: '%s' Pwd: '%s'\n", this, currentWiFiOffset,nSSID.c_str(),nPwd.c_str());
        wifiMulti.addAP(nSSID.c_str(),nPwd.c_str());
        //AddConnection(nSSID.c_str(),nPwd.c_str());
        currentWiFiOffset++;
    }
    InstallTimer();
}

LoTWiFi::~LoTWiFi() {
    Disable();
    NetworkHeartbeatTicker.detach();
    tasks.remove_if([&](LoTWiFiTask *tsk){
        lNetLog("WiFi: %p Destroy task: '%s'\n", this, tsk->Name());
        return true;
    });
    lNetLog("WiFi: %p die\n", this);
}
