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
#include "../system/Application.hpp"
#include "TaskSwitcher.hpp"
#include <LilyGoWatch.h>

extern TTGOClass *ttgo; // access to ttgo specific libs
extern size_t lastAppsOffset;
extern const char  * lastAppsName[LUNOKIOT_MAX_LAST_APPS]; 
extern TFT_eSprite * lastApps[LUNOKIOT_MAX_LAST_APPS];
extern unsigned long lastAppsTimestamp[LUNOKIOT_MAX_LAST_APPS];
extern SemaphoreHandle_t lAppStack;
extern unsigned long touchDownTimeMS;
extern bool UILongTapOverride; // from UI.cpp
TaskSwitcher::TaskSwitcher() {
    // Init here any you need
    //lAppLog("Hello from LastApps!\n");
    //canvas->fillSprite(TFT_BLACK); // use theme colors

    // Remember to dump anything to canvas to get awesome splash screen
    // draw of TFT_eSPI canvas for splash here
    //UILongTapOverride=true;
    dirty=true;
    Tick(); // OR call this if no splash 
    //directDraw=true;
}

TaskSwitcher::~TaskSwitcher() {
    UILongTapOverride=false;

    /*
    int searchOffset = LUNOKIOT_MAX_LAST_APPS; // must be signed
    while ( searchOffset > 0 ) {
        searchOffset--;
        if ( nullptr != captures[searchOffset] ) {
            //lAppLog("Freeing low resolution of %u thumbnail at: %p\n",searchOffset,captures[searchOffset]);
            captures[searchOffset]->deleteSprite();
            delete captures[searchOffset];  
            captures[searchOffset]=nullptr;              
        }
    }*/
    //directDraw=false;
}
// https://blog.espressif.com/esp32-programmers-memory-model-259444d89387
bool TaskSwitcher::Tick() {

    if ( millis() > nextRefresh ) {
        dirty=true;
        nextRefresh=millis()+(1000/8);
    }
    if ( dirty ) {
        if ( touched ) {
            int offX=touchX/(canvas->width()/3);
            int offY=touchY/(canvas->height()/3);
            selectedOffset=(offY*3)+offX;
            //lAppLog("IMAGE OFFSET: %d\n", selectedOffset);
        } else { selectedOffset=-1; }

        // background fades
        if ( -1 == selectedOffset ) { canvas->fillSprite(ThCol(background_alt)); }
        else { canvas->fillSprite(TFT_BLACK); }

        uint8_t shadow=0;
        for(int y=0;y<canvas->height();y+=3) {
            uint32_t color= tft->color565(shadow,shadow,shadow);
            canvas->drawFastHLine(0,y,canvas->width(),color);
            shadow+=2;
        }
        int searchOffset = LUNOKIOT_MAX_LAST_APPS; // must be signed value
        bool alreadyExists=false;
        int16_t x=canvas->width()/6;
        int16_t y=canvas->height()/6;
        //lLog("@TODO ugly hardcoded list of hidden apps (use const string and share with the apps AppName())\n");
        if( xSemaphoreTake( lAppStack, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) == pdTRUE )  {
            int tileCount=0;
            while ( searchOffset > 0 ) {
                searchOffset--;
                if ( 0 == lastAppsTimestamp[searchOffset] )   { continue; }
                if ( nullptr == lastAppsName[searchOffset] )  { continue; }
                if ( nullptr == lastApps[searchOffset] )      { continue; }

                TFT_eSprite * scaledImg = nullptr;
                // wave effect
                if (-1 == selectedOffset ) {
                    // wave tile effect
                    if (( currentScale > MAXWAVE)||(currentScale < MINWAVE )) { scaleDirection=(!scaleDirection); }
                    if ( scaleDirection ) { currentScale+=(0.0013); }
                    else { currentScale-=(0.0013); }
                    scaledImg = ScaleSprite(lastApps[searchOffset], currentScale);
                } else { // thumb?
                    if ( selectedOffset == tileCount ) { // this is the selected one
                        scaledImg = ScaleSprite(lastApps[searchOffset], 0.50);
                    } else { // reduce others
                        scaledImg = ScaleSprite(lastApps[searchOffset], 0.18);
                    }
                }
                
                if ( nullptr != scaledImg ) {
                    canvas->setPivot(x,y);
                    scaledImg->setPivot(scaledImg->width()/2,scaledImg->height()/2);
                    scaledImg->pushRotated(canvas,0,CanvasWidget::MASK_COLOR);

                    scaledImg->deleteSprite();
                    delete scaledImg;
                    scaledImg=nullptr;

                }
                //lAppLog("RENDERING: %p '%s' %d at X: %d Y: %d\n",lastApps[searchOffset],lastAppsName[searchOffset],searchOffset,x,y);
                // build 3x3 grid
                x+=canvas->width()/3; //captures[searchOffset]->width();
                if ( x >= canvas->width() ) { x=canvas->width()/6; y+=(canvas->height()/3); } //captures[searchOffset]->height(); }
                if ( y >= canvas->height() ) { break; }
                tileCount++;

            }
            //lAppLog("----------------------------\n");
            xSemaphoreGive( lAppStack );
            delay(80);
        }
        dirty=false;
        return true;
    }
    return false;
}
