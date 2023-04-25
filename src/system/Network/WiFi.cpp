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
#include "WiFi.hpp"
#include "../../app/LogView.hpp"
#include "../SystemEvents.hpp" 
#include <ArduinoNvs.h>
#include <Ticker.h>

#include <functional>
#include <list>
#include "BLE.hpp"

//void LoTWiFi::SuspendTasks() { xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS); disabled=true; }
//void LoTWiFi::ResumeTasks() { xSemaphoreGive( taskLock ); disabled=false; }

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
    if ( ( WL_IDLE_STATUS != current )
            &&( WL_NO_SHIELD !=  current )
            &&( WL_DISCONNECTED !=  current ) ) {
                return true;
    }
    return false;
}

bool LoTWiFi::InUse() {
    bool response;
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    response=running;
    xSemaphoreGive( taskLock );
    return response;
}

bool LoTWiFi::IsEnabled() {
    bool response;
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    response=enabled;
    xSemaphoreGive( taskLock );
    return response;
}

bool LoTWiFi::AddTask(LoTWiFiTask * newTask) {
    // @note if change the wait ticks if you want reject by timeout
    Disable();

    size_t offset = 0;
    for (auto const& tsk : tasks) {
        if ( tsk == newTask ) {
            lNetLog("WiFi: %p Task %p '%s' already on the list (offset: %d)\n",this,newTask,tsk->Name(), offset);
            Enable();
            return false;
        }
        offset++;
    }
    tasks.push_back(newTask);
    lNetLog("WiFi: %p Task %p '%s' added (offset: %d)\n",this, newTask,newTask->Name(), offset);
    Enable();
    return true;
}

bool LoTWiFi::RemoveTask(LoTWiFiTask * newTask) {
    lLog("@TODO REMOVETASK NOT IMPLEMENTED\n");
    return false;

}

void LoTWiFi::PerformTasks() {
    lNetLog("WiFi: %p Tasks check\n", this);
    // check user permission
    if ( false == NVS.getInt("WifiEnabled") ) {
        lNetLog("WiFi: %p User settings don't agree\n",this);
        return;
    }
    if ( IsDisabled() ) {
        lNetLog("WiFi: %p Instance disabled\n",this);
        return;
    }
    //check provisioning
    String WSSID = NVS.getString("provSSID");
    String WPass = NVS.getString("provPWD");
    if (( 0 == WSSID.length() )||( 0 == WPass.length() )) {
        lNetLog("WiFi: %p WARNING: Device not provisioned\n",this);
        return;
    }
    // set lower priority
    //UBaseType_t myPriority = uxTaskPriorityGet(NULL);
    //vTaskPrioritySet(NULL,tskIDLE_PRIORITY);
    //taskYIELD();
    //check tasks
    lNetLog("WiFi: %p Checking pending tasks....\n",this);
    xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
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
        xSemaphoreGive( taskLock );
        return;
    }
    lNetLog("WiFi: %p Tasks pending: %u\n",this,tasksMustRun);
    running=true;
    // perform connection
    wl_status_t currStat;
    lNetLog("WiFi: %p trying to connect to '%s'...\n",this,WSSID.c_str());

    currStat = WiFi.begin(WSSID.c_str(),WPass.c_str());
    bool connected=false;
    unsigned long timeout = millis()+ConnectionTimeoutMs;
    while(timeout>millis()) {
        currStat = WiFi.status();
        lNetLog("WiFi: %p Trying to connect (status: %d)...\n",this,currStat);
        if ( WL_CONNECTED == currStat) { connected=true; break; }
        delay(2000);
        taskYIELD();
    }
    if (false == connected) {
        xSemaphoreTake( taskLock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        running=false;
        xSemaphoreGive( taskLock );
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
    delete tasksNeedToRun;

    // disconnect
    WiFi.disconnect(true);
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
        // wait until system brings up
        unsigned long timeOut=millis()+30000; // 30 secs
        while ( systemSleep ) {
            // timeout
            if ( millis() > timeOut ) { return; }
            TickType_t nextCheck = xTaskGetTickCount();
            xTaskDelayUntil( &nextCheck, (2000 / portTICK_PERIOD_MS) );
        }
        xTaskCreatePinnedToCore([](void *obj) { // in core 0 please
            LoTWiFi* instance = (LoTWiFi*)obj;
            //instance->NetworkHeartbeatTicker.detach(); // stop the timer
            instance->PerformTasks();
            //instance->InstallTimer(); // isn't recursive :P
            vTaskDelete(NULL);
        },"lwifi",LUNOKIOT_TASK_STACK_SIZE,instance,tskIDLE_PRIORITY, NULL,0);
    },this);
}

LoTWiFi::LoTWiFi() {
    lNetLog("WiFi: %p start\n", this);
    // @TODO warn if NVS user preference don't allow
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
