#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"
#include "Point2D.hpp"

#include "../UI.hpp"

Point2D::Point2D(int16_t x,int16_t y) : x(x), y(y) { }

void Point2D::Set(int16_t px, int16_t py) {
    if (( px != x )||( py != y )) {
        last_x=x;
        last_y=y;
        x=px;
        y=py;
        //Serial.printf("%p AAAAAAAAAAAAAAA: X: %d Y: %d AX: %d AY: %d\n", this, x, y, last_x, last_y);
        // notify to the UI loop
        _UINotifyPoint2DChange(this);
        // esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_READY,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
    }
}
void Point2D::GetDelta(int16_t &dx, int16_t &dy) {
    dx = last_x ;//last_x - x;
    dy = int16_t(last_y - y);
}

void Point2D::Get(int16_t &px, int16_t &py) {
    px = x;
    py = y;
}
