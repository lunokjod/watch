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

#include "lunokiot_config.hpp"

#include "../UI.hpp"
#include "ActiveRect.hpp"

#include "../../app/LogView.hpp"
#include "../../system/SystemEvents.hpp"

ActiveRect::ActiveRect(int16_t x,int16_t y, int16_t h, int16_t w, UICallback tapCallback):
                                    x(x),y(y),h(h),w(w),tapActivityCallback(tapCallback) {
    taskStackSize = LUNOKIOT_TASK_STACK_SIZE;
}


bool ActiveRect::InRadius(int16_t px, int16_t py, int16_t x,int16_t y,int16_t r) {
    // obtain vector from center
    int16_t tdvX = px-(x+(r/2));
    int16_t tdvY = py-(y+(r/2));
    // obtain distance between two points (radial active area)
    int16_t dist = sqrt(pow(tdvX, 2) + pow(tdvY, 2) * 1.0);
    if ( dist > (r/2) ) {
        return false;
    }
    //Serial.printf("ActiveRect: Event: Thumb PX: %d PY: %d INRADIUS between X: %d Y: %d H: %d W: %d\n",px,py,x,y,x+h,y+w);
    return true;
}

bool ActiveRect::InRect(int16_t px, int16_t py, int16_t x,int16_t y,int16_t h,int16_t w) {
    if ( px < x ) { return false; }
    if ( py < y ) { return false; }
    if ( px > (x+w) ) { return false; }
    if ( py > (y+h) ) { return false; }
    //Serial.printf("ActiveRect: Event: Thumb PX: %d PY: %d INRECT between X: %d Y: %d W: %d H: %d\n",px,py,x,y,x+w,y+h);
    return true;
}
// used when call to callback
struct LaunchDescriptor {
    UICallback callback;
    void * param; // the caller object as void*
};

void ActiveRect::Trigger() {
    if ( nullptr == tapActivityCallback ) { return; }
    LaunchDescriptor * comm = new LaunchDescriptor();
    if ( nullptr == comm ) {
        lUILog("%s: %p ERROR: Unable to create launch descriptor!\n",__PRETTY_FUNCTION__,this);
        return;
    }
    comm->callback=tapActivityCallback;
    comm->param=paramCallback;
    BaseType_t res = xTaskCreatePinnedToCore(ActiveRect::_LaunchCallbackTask, "", taskStackSize, comm, uxTaskPriorityGet(NULL), NULL,1);
    if ( res != pdTRUE ) {
        delete(comm);
        lUILog("%s: %p ERROR: Unable to launch task!\n",__PRETTY_FUNCTION__,this);
        FreeSpace();
        //lUILog("ESP32: Free heap: %d KB\n", ESP.getFreeHeap()/1024);
        //lUILog("ESP32: Free PSRAM: %d KB\n", ESP.getFreePsram()/1024);
    }
}

void ActiveRect::_LaunchCallbackTask(void * callbackData) {
    if ( nullptr == callbackData ) {
        lUILog("ActiveRect unable to launch callback!\n");
        vTaskDelete(NULL);
    }
    delay(5); // to get correct values (triggers taskdelay that triggers yeld)
    //EventKVO *self = reinterpret_cast<EventKVO*>(handler_args);
    LaunchDescriptor *desc = (LaunchDescriptor *)callbackData; // risky
    (desc->callback)(desc->param);
    delete desc;
    vTaskDelete(NULL);
}

bool ActiveRect::Interact(bool touch, int16_t tx,int16_t ty, bool trigger) {
    if ( false == enabled ) { return false; } // no sensitive
    if ( touch ) {
        pushed = ActiveRect::InRect(tx,ty,x,y,h,w); // inside?
        if ( pushed ) {
            if ( 0 == currentTapTime ) {
                currentTapTime = millis();
                lastInteraction = true;
            }
        }
        return pushed; // don't trigger Interact
    } else { pushed = false; }
    if ( false == lastInteraction ) { return false; } // get out fast!
    // check here the time pushed
    unsigned long tapTime=millis()-currentTapTime;
    
    if ( tapTime > 10 ) {
        // no callback, nothing to do
        if ( nullptr != tapActivityCallback ) {
            // launch the callback!
            pushed = ActiveRect::InRect(tx,ty,x,y,h,w); // inside?
            if ( ( trigger ) && ( pushed )) { Trigger(); }
        }
    }
    lastInteraction=false;
    currentTapTime = 0; // no tap, no count
    return true;
}
