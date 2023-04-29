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
#include "WatchfaceBasic.hpp"
#include "../UI/UI.hpp"

#include "LuIMainMenu.hpp"
#include "Steps.hpp"
#include "Battery.hpp"
#include "Settings.hpp"
#include <sys/param.h>

WatchfaceBasic::WatchfaceBasic() {
    bottomLeftButton = new ActiveRect(0, 160, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new BatteryApplication()); });
    topLeftButton = new ActiveRect(0, 0, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new SettingsApplication()); });
    topRightButton = new ActiveRect(160, 0, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new StepsApplication()); });
    bottomRightButton = new ActiveRect(160, 160, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new LuIMainMenuApplication()); });

    //displayBuffer = new CanvasWidget((TFT_WIDTH-dotSize)/dotSize,(TFT_HEIGHT-dotSize)/dotSize);
    displayBuffer = new CanvasWidget((TFT_WIDTH)/dotSize,(TFT_HEIGHT)/dotSize);
    displayBuffer->canvas->fillSprite(TFT_BLACK);
    Tick();
}

WatchfaceBasic::~WatchfaceBasic() {
    if ( nullptr != topLeftButton ) { delete topLeftButton; }
    if ( nullptr != topRightButton ) { delete topRightButton; }
    if ( nullptr != bottomRightButton ) { delete bottomRightButton; }
    if ( nullptr != bottomLeftButton ) { delete bottomLeftButton; }
    if ( nullptr != displayBuffer ) { delete displayBuffer; }
}

const char *DayString[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
const char *MonthString[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

bool WatchfaceBasic::Tick() {
    topRightButton->Interact(touched, touchX, touchY); // steps
    bottomRightButton->Interact(touched, touchX, touchY); // menu
    topLeftButton->Interact(touched, touchX, touchY); // settings
    bottomLeftButton->Interact(touched, touchX, touchY); // battery

    if ( millis() > dotRefresh) {
        showDot=(!showDot);
        dotRefresh=millis()+2000;
    }

    /* if ( millis() > refreshPointDelayMS ) {
        refreshPoint++;
        if ( refreshPoint > (displayBuffer->canvas->width()*displayBuffer->canvas->height()) ) { refreshPoint=0; }
        refreshPointDelayMS=millis()+20;
    }*/

    if ( millis() > nextRefresh ) { // redraw full canvas
        /*
        for (int y=0;y<displayBuffer->canvas->height();y++) {
            for (int x=0;x<displayBuffer->canvas->width();x++) {
                if ( 10 == random(0,20) ) {
                    uint16_t rcolor = random(0,65535);
                    displayBuffer->canvas->drawPixel(x,y,rcolor);
                }
            }
        }
        */
        time_t now;
        struct tm * tmpTime;
        struct tm timeinfo;
        time(&now);
        tmpTime = localtime(&now);
        memcpy(&timeinfo,tmpTime, sizeof(struct tm));
        char buffer[64] = { 0 };
        if ( showDot ) { sprintf(buffer,"%02d:%02d:%02d", tmpTime->tm_hour, timeinfo.tm_min, timeinfo.tm_sec); }
        else { sprintf(buffer,"%02d %02d %02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec); }
        displayBuffer->canvas->setFreeFont(&TomThumb);//Org_01);
        displayBuffer->canvas->setTextSize(1);
        displayBuffer->canvas->setTextDatum(TL_DATUM);
        displayBuffer->canvas->setTextColor(LedTimeColor);
        displayBuffer->canvas->drawString(buffer, 2,2);

        // update day of week
        int weekday=timeinfo.tm_wday;
        sprintf(buffer,"%s",DayString[weekday]);
        displayBuffer->canvas->setTextSize(1);
        displayBuffer->canvas->drawString(buffer, 3,9);

        // update day of month
        displayBuffer->canvas->setFreeFont(&Org_01);
        displayBuffer->canvas->setTextSize(1);
        sprintf(buffer,"%02d",timeinfo.tm_mday);
        displayBuffer->canvas->drawString(buffer, 16,9);

        // update month
        int month=timeinfo.tm_mon;
        displayBuffer->canvas->setFreeFont(&Org_01);
        sprintf(buffer,"%s",MonthString[month]);
        displayBuffer->canvas->setTextSize(1);
        displayBuffer->canvas->drawString(buffer, 8,16);

        // update year
        displayBuffer->canvas->setFreeFont(&Picopixel);
        displayBuffer->canvas->setTextSize(1);
        displayBuffer->canvas->setTextDatum(TL_DATUM);
        displayBuffer->canvas->setTextColor(LedTimeColor);
        sprintf(buffer,"%04d",1900+timeinfo.tm_year);
        displayBuffer->canvas->drawString(buffer, 8,23);


        int32_t currentPixel=0;
        // redraw whole watchface canvas
        canvas->fillSprite(TFT_BLACK); // clean canvas
        for (int y=0;y<displayBuffer->canvas->height();y++) {
            for (int x=0;x<displayBuffer->canvas->width();x++) {
                /*
                if ( currentPixel == refreshPoint ) { // flickering
                    displayBuffer->canvas->drawPixel(x,y,TFT_WHITE);
                    refreshPoint++;
                    if ( refreshPoint > (displayBuffer->canvas->width()*displayBuffer->canvas->height()) ) { refreshPoint=0; }
                }*/
    
                //led color
                uint16_t color = displayBuffer->canvas->readPixel(x,y);
                canvas->fillCircle((x*dotSize)+(dotSize/2),(y*dotSize)+(dotSize/2),(dotSize/2)-1,color);
                // get color
                uint8_t r,g,b; GetRGBFrom16Bit(color, r,g,b);
                uint8_t bright=(r+g+b)/3; // get bright
                if ( bright > 16 ) {
                    //add led bright light
                    uint16_t bcolor = tft->alphaBlend(128,TFT_WHITE,color);
                    canvas->fillCircle((x*dotSize)+(dotSize*0.6),(y*dotSize)+(dotSize*0.3),(dotSize/4),bcolor);
                    // color decay to black
                    uint16_t fcolor = tft->alphaBlend(72,TFT_BLACK,color);
                    displayBuffer->canvas->drawPixel(x,y,fcolor);
                } else {
                    // too low bright: erase pixel
                    displayBuffer->canvas->drawPixel(x,y,TFT_BLACK);
                }
                currentPixel++;
            }
        }
        nextRefresh=millis()+(1000/6);
        return true;
    }
    return false;
}
