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
#include "Stopwatch.hpp"
#include "../lunokiot_config.hpp"

#include "../static/img_play_48.xbm"
#include "../static/img_reload_48.xbm"
#include "../static/img_pause_48.xbm"
#include <ArduinoNvs.h>
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo lib

unsigned long StopwatchApplication::pauseTime=0;
unsigned long StopwatchApplication::starTime=0;

StopwatchApplication::~StopwatchApplication() {
    delete resetBtn;
    delete pauseBtn;
    delete startBtn;
}

StopwatchApplication::StopwatchApplication() {
    canvas->setTextFont(0);
    canvas->setTextSize(3);
    canvas->setTextColor(TFT_WHITE);
    canvas->setTextDatum(CC_DATUM);

    startBtn = new ButtonImageXBMWidget(10,120,70,70,[this](void *unused) {
        starTime=millis();
        pauseTime=0;
        //NVS.setInt("sw_start",uint64_t(starTime),false);
        //NVS.setInt("sw_pause",uint64_t(pauseTime),false);
    },img_play_48_bits,img_play_48_height,img_play_48_width,TFT_GREEN,Drawable::MASK_COLOR,false);

    resetBtn = new ButtonImageXBMWidget(10+74,120,70,70,[this](void *unused) {
        starTime=0;
        pauseTime=0;
        //NVS.setInt("sw_start",uint64_t(starTime),false);
        //NVS.setInt("sw_pause",uint64_t(pauseTime),false);
    },img_reload_48_bits,img_reload_48_height,img_reload_48_width,TFT_RED,Drawable::MASK_COLOR,false);

    pauseBtn = new ButtonImageXBMWidget(10+(74*2),120,70,70,[this](void *unused) {
        if ( 0 == starTime ) { return; }
        if ( 0 == pauseTime ) {
            pauseTime=millis();
            return;
        }
        unsigned long diff=pauseTime-starTime;
        starTime=millis()-diff;
        pauseTime=0;
        //NVS.setInt("sw_start",uint64_t(starTime),false);
        //NVS.setInt("sw_pause",uint64_t(pauseTime),false);

    },img_pause_48_bits,img_pause_48_height,img_pause_48_width,TFT_YELLOW,Drawable::MASK_COLOR,false);

    TemplateApplication::btnBack->xbmColor=ThCol(text); // more visible back button
    TemplateApplication::btnBack->InternalRedraw();
    Tick();
}
bool StopwatchApplication::Tick() {
    startBtn->Interact(touched,touchX,touchY);
    resetBtn->Interact(touched,touchX,touchY);
    pauseBtn->Interact(touched,touchX,touchY);
     // dont sleep when is in use
    if ( 0 != starTime ) { UINextTimeout = millis()+UITimeout; }
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        char displayText[16] = { 0 };
        if ( starTime > 0 ) {
            unsigned long diff= millis()-starTime;
            if ( pauseTime > 0 ) {
                diff=pauseTime-starTime;
                if ( millis() > nextBlink ) {
                    blink=(!blink);
                    nextBlink=millis()+300;
                }
            }
            else { blink = false; }
            int seconds = diff / 1000;
            diff %= 1000;
            int minutes = seconds / 60;
            seconds %= 60;
            int hours = minutes / 60;
            minutes %= 60;
            if ( false == blink ) {
                sprintf(displayText,"%02d:%02d:%02d.%03d",hours,minutes,seconds,diff);
            }
        } else {
            sprintf(displayText,"00:00:00.000");
        }
        canvas->fillRoundRect(0,40,TFT_WIDTH,40,10,TFT_BLACK);
        canvas->drawString(displayText,TFT_WIDTH/2,60);
        TemplateApplication::Tick();

        canvas->fillRoundRect(15,122,210,68,30,TFT_BLACK);
        startBtn->DrawTo(canvas);
        resetBtn->DrawTo(canvas);
        pauseBtn->DrawTo(canvas);
        nextRedraw=millis()+(1000/12);
        return true;
    }
    return false;
}
