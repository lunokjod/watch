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
#include "Playground0.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/UI.hpp"

PlaygroundApplication0::~PlaygroundApplication0() {
}

PlaygroundApplication0::PlaygroundApplication0() {
}

typedef struct {
    PlaygroundApplication0* app;
    float angle;
    uint32_t color;
} useAsPayload;

bool DrawCircleCallback(int x,int y, int cx, int cy, int angle, int step, void* payload) {
    useAsPayload * data = (useAsPayload *)payload;
    PlaygroundApplication0 * self = (PlaygroundApplication0 *)data->app;
    if ( angle < data->angle ) {
        self->canvas->fillCircle(x,y,8,data->color);
    }
    return true;
}
bool PlaygroundApplication0::Tick() {
    if (millis() > nextRedraw ) {
        canvas->fillSprite(TFT_BLACK);
        useAsPayload payload = { this, degX, TFT_BLUE };
        DescribeCircle(120,120,80,DrawCircleCallback,&payload);
        payload = { this, degY, TFT_GREEN };
        DescribeCircle(120,120,60,DrawCircleCallback,&payload);
        payload = { this, degZ, TFT_RED };
        DescribeCircle(120,120,40,DrawCircleCallback,&payload);
        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}
