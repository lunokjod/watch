#include <functional>

#include "lunokiot_config.hpp"

#include "../UI.hpp"
#include "ActiveRect.hpp"

#include "../../app/LogView.hpp"

ActiveRect::ActiveRect(int16_t x,int16_t y, int16_t h, int16_t w, std::function<void ()> tapCallback):
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

void ActiveRect::Trigger() {
    BaseType_t res = xTaskCreate(ActiveRect::_LaunchCallbackTask, "", taskStackSize, &tapActivityCallback, uxTaskPriorityGet(NULL), NULL);
    if ( res != pdTRUE ) {
        lUILog("%s: %p ERROR: Unable to launch task!\n",__PRETTY_FUNCTION__,this);
        lUILog("ESP32: Free heap: %d KB\n", ESP.getFreeHeap()/1024);
        lUILog("ESP32: Free PSRAM: %d KB\n", ESP.getFreePsram()/1024);
    }
}

void ActiveRect::_LaunchCallbackTask(void * callbackData) {
    delay(5); // to get correct values (triggers taskdelay that triggers yeld)
    std::function<void ()> *runMe = reinterpret_cast<std::function<void ()>*>(callbackData);
    (*runMe)();
    vTaskDelete(NULL);
}

bool ActiveRect::Interact(bool touch, int16_t tx,int16_t ty) {
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
            Trigger();
        }
    }
    lastInteraction=false;
    currentTapTime = 0; // no tap, no count
    return true;
}
