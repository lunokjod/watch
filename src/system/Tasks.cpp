#include <Arduino.h>
#include "Tasks.hpp"
#include <list>
#include "../lunokiot_config.hpp"

extern bool ntpSyncDone;

// Task scheduler list
std::list<TimedTaskDescriptor *> pendingTasks = {};

/*
 * Stop desired task from network scheduler
 */
bool RemoveTimedTask(TimedTaskDescriptor *oldTsk) {
    Serial.printf("RemoveTimedTask: Task %p '%s' removed\n", oldTsk,oldTsk->name);
    pendingTasks.remove(oldTsk);
    return true;
}

/*
 * Add a timed network task to the network scheduler
 */
bool AddTimedTask(TimedTaskDescriptor *nuTsk) {
    size_t offset = 0;
    for (auto const& tsk : pendingTasks) {
        if ( tsk == nuTsk ) {
            Serial.printf("AddTimedTask: Task %p '%s' already on the list (offset: %d)\n", tsk,tsk->name, offset);
            return false;
        }
        offset++;
    }
    pendingTasks.push_back(nuTsk);
    Serial.printf("AddTimedTask: Task %p '%s' added (offset: %d)\n", nuTsk,nuTsk->name, offset);
    return true;
}

static void TimedHandlerTask(void* args) {
    unsigned long nextConnectMS = 0;
    unsigned long beginConnected = -1;
    unsigned long beginIdle = -1;
    while(true) {
        delay(2*1000); // allow other tasks to do their chance :)
        if ( systemSleep ) { continue; } // fuck off... giveup/sleep in progress... (shaded area)

        if ( false == ntpSyncDone ) {
            Serial.println("TimedTasks: Not runnig due no NTP sync or manual set timedate isn't done");
            delay(10*1000);
            continue;
        }

        for (auto const& tsk : pendingTasks) {
            if ( -1 == tsk->_nextTrigger ) {
                tsk->_nextTrigger = millis()+tsk->everyTimeMS;
            } else {
                if ( millis() > tsk->_nextTrigger ) {
                    delay(150);
                    Serial.printf("TimedTasks: Running task '%s'...\n", tsk->name);
                    bool res = tsk->callback();
                    if ( res ) {
                        tsk->_nextTrigger = millis()+tsk->everyTimeMS;
                        tsk->_lastCheck = millis();
                        Serial.printf("TimedTasks: Task '%s' done!\n", tsk->name);
                    } else { 
                        Serial.printf("TimedTasks: Task '%s' FAILED!\n", tsk->name);
                    }
                }
            }
        }        
    }
    vTaskDelete(NULL);
}
bool TimedTaskkHandler() {
    xTaskCreate(TimedHandlerTask, "ltask", LUNOKIOT_APP_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);
    return false;
}