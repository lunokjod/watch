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
#include "Steps.hpp"
#include "Battery.hpp"
#include "Settings.hpp"
#include "../lunokIoT.hpp"
#include "../resources.hpp"
#include "../system/Network/BLE.hpp"
#include <WiFi.h>
#include "../system/Datasources/database.hpp"

// continue last status in re-launch
int16_t circleRadius[20] = { 0 };
int16_t circleX[20]= { 0 };
int16_t circleY[20]= { 0 };


extern bool weatherSyncDone;
extern int weatherId;
extern double weatherTemp;
extern char *weatherMain;
extern char *weatherDescription;
extern char *weatherIcon;
extern bool UILongTapOverride; // don't allow long tap for task switcher

WatchfaceDotApplication::WatchfaceDotApplication() {
    bottomLeftButton = new ActiveRect(0, 160, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new BatteryApplication()); });
    topLeftButton = new ActiveRect(0, 0, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new SettingsApplication()); });
    topRightButton = new ActiveRect(160, 0, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new StepsApplication()); });
    bottomRightButton = new ActiveRect(160, 160, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new LuIMainMenuApplication()); });


    displayBuffer = new CanvasWidget(48,48); // fill with status

    locX=random(0,displayBuffer->canvas->width());
    locY=random(0,displayBuffer->canvas->height());
    locXDir=(bool)random(0,1);
    locYDir=(bool)random(0,1);
    displayBuffer->canvas->setTextFont(0);
    displayBuffer->canvas->setTextSize(1);
    displayBuffer->canvas->setTextDatum(CC_DATUM);
    displayBuffer->canvas->setTextColor(TFT_WHITE);

    locationBuffer = new CanvasWidget(120,120);
    
    Tick();
    SetUserBrightness();

    UILongTapOverride=true;
}

WatchfaceDotApplication::~WatchfaceDotApplication() {
    if ( nullptr != topLeftButton ) { delete topLeftButton; }
    if ( nullptr != topRightButton ) { delete topRightButton; }
    if ( nullptr != bottomRightButton ) { delete bottomRightButton; }
    if ( nullptr != bottomLeftButton ) { delete bottomLeftButton; }
    
    if ( nullptr != displayBuffer ) { delete displayBuffer; }
    if ( nullptr != locationBuffer ) { delete locationBuffer; }
    UILongTapOverride=false;
}

bool WatchfaceDotApplication::Tick() {
    topRightButton->Interact(touched, touchX, touchY); // steps
    bottomRightButton->Interact(touched, touchX, touchY); // menu
    topLeftButton->Interact(touched, touchX, touchY); // settings
    bottomLeftButton->Interact(touched, touchX, touchY); // battery

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
        const uint8_t border=2;

        if ( locX > (displayBuffer->canvas->width()-(lenPxText/2))-border ) {
            locX = (displayBuffer->canvas->width()-(lenPxText/2))-border;
            locXDir=false;
        } else if ( locX < (lenPxText/2)+border ) {
            locX = (lenPxText/2)+border;
            locXDir=true;
        }
        if ( locY > (displayBuffer->canvas->height()-(heightPxText/2))-border ) {
            locY = displayBuffer->canvas->height()-(heightPxText/2)-border;
            locYDir=false;
        } else if ( locY < (heightPxText/2)+border ) {
            locY = (heightPxText/2)+border;
            locYDir=true;
        }
        displayBuffer->canvas->setTextColor(TFT_BLACK);
        displayBuffer->canvas->drawString(buffer,locX-1,locY-1);
        displayBuffer->canvas->drawString(buffer,locX+1,locY-1);
        displayBuffer->canvas->drawString(buffer,locX-1,locY+1);
        displayBuffer->canvas->drawString(buffer,locX+1,locY+1);
        displayBuffer->canvas->setTextColor(TFT_WHITE);
        displayBuffer->canvas->drawString(buffer,locX,locY);
        // 5x5 draw
        for (int y=0;y<displayBuffer->canvas->height();y++) {
            for (int x=0;x<displayBuffer->canvas->width();x++) {
                uint16_t color = displayBuffer->canvas->readPixel(x,y);
                canvas->fillCircle((x*5)+5,(y*5)+5,dotSize,color);
            }
        }

        if ( showDetails ) {
            locationBuffer->canvas->fillSprite(TFT_PINK);
            char textBuffer[100] = { 0 }; // much more than can be represented x'D
            locationBuffer->canvas->setTextFont(0);
            locationBuffer->canvas->setTextSize(1);
            locationBuffer->canvas->setTextDatum(TL_DATUM);

            // WiFi
            if ( LoT().GetWiFi()->RadioInUse() ) {
                uint16_t wifiColor = TFT_DARKGREY;
                wifiColor = TFT_GREEN;
                locationBuffer->canvas->setTextColor(wifiColor);
                sprintf(textBuffer,"WiFi");            
                locationBuffer->canvas->drawString(textBuffer,5,5);
            }

            // BLE
            uint16_t bleColor = TFT_DARKGREY;
            char * BLEstate = (char*)"";
            if ( LoT().GetBLE()->InUse() ) {
                if ( LoT().GetBLE()->IsAdvertising() ) {
                    bleColor = TFT_YELLOW;
                    BLEstate=(char*)"(adv)";
                } else if ( LoT().GetBLE()->IsScanning()) {
                    bleColor = TFT_CYAN;
                    BLEstate=(char*)"(scan)";
                } else if ( LoT().GetBLE()->Clients() > 0 ) {
                    bleColor = TFT_GREEN;
                    BLEstate=(char*)"(conn)";
                }
                locationBuffer->canvas->setTextColor(bleColor);
                sprintf(textBuffer,"BLE %s",BLEstate);
                locationBuffer->canvas->drawString(textBuffer,5,15);
                // ignore not-set and unset
                if ( BLELocationZone != BLEZoneLocations::UNKNOWN ) {
                    //sprintf(textBuffer,"Zone: %d", BLELocationZone);
                    sprintf(textBuffer,"%s", BLEZoneLocationsHumanReadable[BLELocationZone]);
                    locationBuffer->canvas->setTextDatum(CC_DATUM);
                    locationBuffer->canvas->setTextColor(TFT_WHITE);
                    locationBuffer->canvas->drawString(textBuffer,locationBuffer->canvas->width()/2,locationBuffer->canvas->height()/2);
                }
            }
            // SQL
            unsigned int pendSQLQueries = systemDatabase->Pending();
            if ( 0 != pendSQLQueries ) {
                locationBuffer->canvas->setTextDatum(TL_DATUM);
                sprintf(textBuffer,"SQL (%u)", pendSQLQueries );
                locationBuffer->canvas->setTextColor(TFT_WHITE);
                locationBuffer->canvas->drawString(textBuffer,5,25);
            }
        }

        // show raindrops
        for (int y=0;y<locationBuffer->canvas->height();y++) {
            for (int x=0;x<locationBuffer->canvas->width();x++) {
                uint16_t color = locationBuffer->canvas->readPixel(x,y);
                if ( TFT_PINK == color ) { continue; }
                canvas->fillCircle((x*2)+1,(y*2)+1,1,color);
            }
        }
        nextRefresh=millis()+(1000/12);
        return true;
    }
    return false;
}
