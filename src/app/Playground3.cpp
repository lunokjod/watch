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
#include "Playground3.hpp"

PlaygroundApplication3::~PlaygroundApplication3() {
    if ( nullptr != hourGauge ) {
        delete hourGauge;
        hourGauge = nullptr;
    }
    
}

extern int currentHour;
extern int currentMin;

PlaygroundApplication3::PlaygroundApplication3() {
    hourGauge = new GaugeWidget(TFT_WIDTH-100,10,220);
    //hourGauge = new GaugeWidget((TFT_WIDTH/2)-80,TFT_HEIGHT-80,240-80);
    minuteGauge = new GaugeWidget(-110,10,220);

    // common font set
    canvas->setTextFont(0);
    canvas->setTextSize(4);
    canvas->setTextWrap(false,false);

    // convert hours to degrees to be coherent with current time
    //hour = currentHour;
    //minute = currentMin;
    hourGauge->selectedAngle   = (currentHour*15);
    minuteGauge->selectedAngle = ((currentMin  * 360 ) / 120);
}
bool PlaygroundApplication3::Tick() {
    bool moved = hourGauge->Interact(touched,touchX, touchY,10);
    if ( false == moved ) { // only rotate one gauge at a time
        moved = minuteGauge->Interact(touched,touchX, touchY,10);
    }

    if ( (-1 != fixTimeTimeout ) && ( millis() > fixTimeTimeout ) ) {
        fixTimeTimeout = -1;
        timeIsSet=true;
        Serial.println("@TODO SetTime: RTC and localtime sync");
        /*
        RTC_Date test;
        test.hour = hour;
        test.minute = minute;
        ttgo->rtc->setDateTime(test);
        ttgo->rtc->syncToSystem();
        */
    }

    if ( moved ) { // manipulte the blink state to force show time
        fixTimeTimeout = millis()+SetTimeTimeout;
        timeIsSet = false;
        showTime = true;
        timeTick = millis()+ 1000; // 1 second blink
    } // force re-show on gauge change


    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        hourGauge->DrawTo(canvas);
        minuteGauge->DrawTo(canvas);
        if ( millis() > timeTick) {
            showTime=(!showTime);
            timeTick = millis()+ 1000; // 1 second blink
        }
        if (( showTime ) || ( timeIsSet ) ) {
            char textBuffer[32] = { 0 };
            hour = hourGauge->selectedAngle/(360/48)-26;
            minute = minuteGauge->selectedAngle/(360/120);
            if ( hour < 0 ) { hour = 0; }
            else if ( hour > 23 ) { hour = 23; }
            if ( minute < 0 ) { minute = 0; }
            else if ( minute > 59 ) { minute = 59; }
            sprintf(textBuffer,"%02d:%02d", hour, minute);
            int posX = TFT_WIDTH/2;
            int posY = 20; //TFT_HEIGHT/2;

            canvas->setTextDatum(TC_DATUM);
            canvas->setTextColor(TFT_BLACK);
            canvas->drawString(textBuffer, posX+2, posY+2);
            canvas->setTextColor(TFT_WHITE);
            canvas->drawString(textBuffer, posX, posY);
        }

        canvas->setTextDatum(CL_DATUM);
        int posX = 10;
        int posY = TFT_HEIGHT/2;
        canvas->setTextColor(TFT_BLACK);
        canvas->drawString("Min",posX+2, posY+2);
        canvas->setTextColor(TFT_WHITE);
        canvas->drawString("Min",posX, posY);



        canvas->setTextDatum(CR_DATUM);
        posX = TFT_WIDTH-10;
        posY = TFT_HEIGHT/2;
        canvas->setTextColor(TFT_BLACK);
        canvas->drawString("Hr",posX+2, posY+2);
        canvas->setTextColor(TFT_WHITE);
        canvas->drawString("Hr",posX, posY);

        nextRedraw=millis()+(1000/16);
        return true;
    }
    return false;
}