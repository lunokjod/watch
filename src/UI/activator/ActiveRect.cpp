#include <Arduino.h>
#include <LilyGoWatch.h>
#include <functional>

#include "lunokiot_config.hpp"

#include "../UI.hpp"
#include "ActiveRect.hpp"


ActiveRect::ActiveRect(int16_t x,int16_t y, int16_t h, int16_t w, std::function<void ()> tapCallback):
                                    x(x),y(y),h(h),w(w),tapActivityCallback(tapCallback) {
    taskStackSize = LUNOKIOT_TASK_STACK_SIZE;
    enabled=true;
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
    if ( px > (x+w) ) { return false; } // @TODO this maybe is wrong!!!
    if ( py > (y+h) ) { return false; }
    //Serial.printf("ActiveRect: Event: Thumb PX: %d PY: %d INRECT between X: %d Y: %d H: %d W: %d\n",px,py,x,y,x+h,y+w);
    return true;
}

void ActiveRect::_LaunchCallbackTask(void * callbackData) {
    //delay(5);
    std::function<void ()> *runMe = reinterpret_cast<std::function<void ()>*>(callbackData);
    (*runMe)();
    vTaskDelete(NULL);
}
void ActiveRect::SetEnabled(bool state) {
    enabled = state;
    if ( false == state ) {
        lastInteraction=false; 
    }
}
bool ActiveRect::Interact(bool touch, int16_t tx,int16_t ty) {
    if ( false == enabled ) { return false; }

    if ( false == touch ) {
        if ( lastInteraction ) {
            lastInteraction=false;
            if ( nullptr != tapActivityCallback ) {
                // launch the callback!
                xTaskCreate(ActiveRect::_LaunchCallbackTask, "", taskStackSize, &tapActivityCallback, uxTaskPriorityGet(NULL), NULL);
            }
        }
        return false;
    }
    if ( nullptr == tapActivityCallback ) { return false; }
    if ( lastInteraction ) { return false; }
    //Serial.printf("ActiveArea::Interact(%s, %d, %d) on %p",(touch?"True":"False"),tx,ty, this);
    bool collision = ActiveRect::InRect(tx,ty,this->x,this->y,this->h,this->w);
    if ( false == collision ) { return false; }
    lastTapTime = millis();
    lastInteraction = true;
    return true;
}
