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
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "WatchfaceDot.hpp"
#include "LuIMainMenu.hpp"
#include "../lunokIoT.hpp"


#include "../static/img_weather_200.c"
#include "../static/img_weather_300.c"
#include "../static/img_weather_500.c"
#include "../static/img_weather_600.c"
#include "../static/img_weather_800.c"

extern bool weatherSyncDone;
extern int weatherId;
extern double weatherTemp;
extern char *weatherMain;
extern char *weatherDescription;
extern char *weatherIcon;
extern bool UILongTapOverride; // don't allow long tap for task switcher

WatchfaceDotApplication::WatchfaceDotApplication() {
    bottomRightButton = new ActiveRect(160, 160, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new LuIMainMenuApplication()); });
    if ( nullptr == bottomRightButton ) {
        lAppLog("MEMORY ERROR: Unable to allocate bottomRightButton\n");
    }
    displayBuffer = new CanvasWidget(48,48);

    locX=random(0,displayBuffer->canvas->width());
    locY=random(0,displayBuffer->canvas->height());
    locXDir=(bool)random(0,1);
    locYDir=(bool)random(0,1);
    displayBuffer->canvas->setTextFont(0);
    displayBuffer->canvas->setTextSize(1);
    displayBuffer->canvas->setTextDatum(CC_DATUM);
    displayBuffer->canvas->setTextColor(TFT_WHITE);
    /*
    dotSize = (uint8_t*)ps_malloc(displayBuffer->canvas->width()*displayBuffer->canvas->height());
    for(int off=0;off<48*48;off++) {
        dotSize[off]=2;
    }*/
    Tick();
    UILongTapOverride=true;
    UINextTimeout = millis()+UITimeout; // auto-sleep
}

WatchfaceDotApplication::~WatchfaceDotApplication() {
    if ( nullptr != bottomRightButton ) { delete bottomRightButton; }
    if ( nullptr != displayBuffer ) { delete displayBuffer; }
    //if ( nullptr != dotSize ) { free(dotSize); }
    UILongTapOverride=false;
}

bool WatchfaceDotApplication::Tick() {
    bottomRightButton->Interact(touched, touchX, touchY); // menu
    bool launchDrop = touched;
    // create touch circles
    if ( millis() > nextDrop ) {
        // fake data
        touchX=random(0,240);
        touchY=random(0,240);
        nextDrop=millis()+1000;
        launchDrop=true;
    }
    if ( launchDrop ) {
        //nextDrop=millis()+10000;
        UILongTapOverride=true;
        bool foundSlot=false;
        for(int cc=0;cc<20;cc++) {
            if ( 0 == circleRadius[cc] ) { // get this slot!!!
                circleRadius[cc]=1; // start!!!
                circleX[cc]=touchX;
                circleY[cc]=touchY;
                foundSlot=true;
                break;
            }
        }
    }
    if ( millis() > nextRefresh ) {
        canvas->fillSprite(TFT_BLACK); // cleanup
        displayBuffer->canvas->fillSprite(TFT_BLACK);

        if (weatherSyncDone) {
            int16_t weatherX=displayBuffer->canvas->width()/2;
            int16_t weatherY=displayBuffer->canvas->height()/2;
            if (-1 != weatherId) {
                if ((200 <= weatherId) && (300 > weatherId)) {
                    displayBuffer->canvas->pushImage(weatherX-(img_weather_200.width/2), weatherY-(img_weather_200.height/2), img_weather_200.width, img_weather_200.height, (uint16_t *)img_weather_200.pixel_data);
                }
                else if ((300 <= weatherId) && (400 > weatherId)) {
                    displayBuffer->canvas->pushImage(weatherX-(img_weather_300.width/2), weatherY-(img_weather_300.height/2), img_weather_300.width, img_weather_300.height, (uint16_t *)img_weather_300.pixel_data);
                }
                else if ((500 <= weatherId) && (600 > weatherId)) {
                    displayBuffer->canvas->pushImage(weatherX-(img_weather_500.width/2), weatherY-(img_weather_500.height/2), img_weather_500.width, img_weather_500.height, (uint16_t *)img_weather_500.pixel_data);
                }
                else if ((600 <= weatherId) && (700 > weatherId)) {
                    displayBuffer->canvas->pushImage(weatherX-(img_weather_600.width/2), weatherY-(img_weather_600.height/2), img_weather_600.width, img_weather_600.height, (uint16_t *)img_weather_600.pixel_data);
                }
                else if ((700 <= weatherId) && (800 > weatherId)) {
                    displayBuffer->canvas->pushImage(weatherX-(img_weather_800.width/2), weatherY-(img_weather_800.height/2), img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
                }
                else if ((800 <= weatherId) && (900 > weatherId)) {
                    displayBuffer->canvas->pushImage(weatherX-(img_weather_800.width/2), weatherY-(img_weather_800.height/2), img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
                }
            }
        }
        // draw circles
        for(int cc=0;cc<20;cc++) {
            if ( circleRadius[cc] > 0 ) {
                displayBuffer->canvas->drawCircle(circleX[cc]/5,circleY[cc]/5,circleRadius[cc],TFT_SKYBLUE);
                circleRadius[cc]++;
                if ( circleRadius[cc] > 54 ) { circleRadius[cc] = 0; }
            }
        }

        time_t now;
        struct tm * tmpTime;
        struct tm timeinfo;
        time(&now);
        tmpTime = localtime(&now);
        memcpy(&timeinfo,tmpTime, sizeof(struct tm));
        char buffer[8] = { 0 };
        if ( millis() > nextBlink ) { drawDot=(!drawDot); nextBlink=millis()+1000; } 
        sprintf(buffer,"%02d%c%02d", timeinfo.tm_hour,(drawDot?':':' '), timeinfo.tm_min);
        int16_t lenPxText = displayBuffer->canvas->textWidth(buffer);
        int16_t heightPxText = displayBuffer->canvas->fontHeight();
        locX=locX+(locXDir?+1:-1);
        locY=locY+(locYDir?+1:-1);

        if ( locX > (displayBuffer->canvas->width()-(lenPxText/2)) ) {
            locX = (displayBuffer->canvas->width()-(lenPxText/2));
            locXDir=false;
        } else if ( locX < (lenPxText/2) ) {
            locX = (lenPxText/2);
            locXDir=true;
        }
        if ( locY > displayBuffer->canvas->height()-(heightPxText/2) ) {
            locY = displayBuffer->canvas->height()-(heightPxText/2);
            locYDir=false;
        } else if ( locY < (heightPxText/2) ) {
            locY = (heightPxText/2);
            locYDir=true;
        }
        displayBuffer->canvas->setTextColor(TFT_BLACK);
        displayBuffer->canvas->drawString(buffer,locX-1,locY-1);
        displayBuffer->canvas->drawString(buffer,locX+1,locY-1);
        displayBuffer->canvas->drawString(buffer,locX-1,locY+1);
        displayBuffer->canvas->drawString(buffer,locX+1,locY+1);
        displayBuffer->canvas->setTextColor(TFT_WHITE);
        displayBuffer->canvas->drawString(buffer,locX,locY);

        //size_t dotOffset=0;
        for (int y=0;y<displayBuffer->canvas->height();y++) {
            for (int x=0;x<displayBuffer->canvas->width();x++) {
                uint16_t color = displayBuffer->canvas->readPixel(x,y);
                /*
                if ( TFT_WHITE == color ) { dotSize[dotOffset]=6; }
                dotSize[dotOffset]--;
                if ( dotSize[dotOffset] < 1 ) { dotSize[dotOffset]=1; }
                canvas->fillCircle((x*5)+5,(y*5)+5,dotSize[dotOffset],color);
                dotOffset++;
                */
                canvas->fillCircle((x*5)+5,(y*5)+5,dotSize,color);
            }
        }
        nextRefresh=millis()+(1000/12);
        return true;
    }
    return false;
}
