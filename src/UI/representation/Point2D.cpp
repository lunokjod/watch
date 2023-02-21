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
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"
#include "Point2D.hpp"
#include "../../app/LogView.hpp"
#include "../UI.hpp"
#include <Ticker.h>

Point2D::Point2D(int16_t x,int16_t y) : x(x), y(y) { }

void Point2D::Set(int16_t px, int16_t py) {
    if (( px != x )||( py != y )) {
        last_x=x;
        last_y=y;
        x=px;
        y=py;
        //Serial.printf("%p: X: %d Y: %d AX: %d AY: %d\n", this, x, y, last_x, last_y);
        // notify to the UI loop
        _UINotifyPoint2DChange(this);
        // esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_READY,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
    }
}
void Point2D::GetDelta(int16_t &dx, int16_t &dy) {
    dx = last_x - x; //last_x ;//last_x - x;
    dy = last_y - y;
}

void Point2D::Get(int16_t &px, int16_t &py) {
    px = x;
    py = y;
}

double Point2D::Distance(double x1, double y1, double x2, double y2) {
    double square_difference_x = (x2 - x1) * (x2 - x1);
    double square_difference_y = (y2 - y1) * (y2 - y1);
    double sum = square_difference_x + square_difference_y;
    double value = sqrt(sum);
    return value;
}

void Point2D::Goto(int16_t nx,int16_t ny,unsigned long time) {
    if ( 0 == time ) { // no time for do any animation: move instantly!
        Set(nx,ny);
        return;
    }

    // obtain steps between two points
    //double steps = sqrt(pow(nx - x, 2) + pow(ny - y, 2) * 1.0);
    steps = Distance(x,y,nx,ny);
    int16_t vx = x - nx;
    int16_t vy = y - ny;
    float incX = vx/steps;
    float incY = vy/steps;
    uint32_t stepMs = time/steps;
    lUILog("STEPS: %g vectorX: %d vectorY: %d incX: %0.2f incY: %0.2f\n", steps, vx, vy, incX, incY);
    animStepper.attach_ms(stepMs,[](){
        //Point2D * from =(Point2D *)fromPtr;
        lUILog("LOOOL\n");

        //steps--;
    });
}
