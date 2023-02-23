//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//    Watchface Author: @hpsaturn
//    License: GPLv3
//----------------------------------------------------------------

#include <Arduino.h>
#include "LogView.hpp" // log capabilities
#include "WatchfaceSquare.hpp"


#define MINIMUM_BACKLIGHT 10
#define MARGIN_TOP 3
#define MARGIN_LFT 2

char const *DAY[]={"SUN","MON","TUE","WED","THU","FRI","SAT"};

WatchfaceSquare::WatchfaceSquare() {
    // Init here any you need
    lAppLog("WatchfaceSquare On\n");

    // initalize here widgets
    //mywidget=new .....

    // Remember to dump anything to canvas to get awesome splash screen
    // draw of TFT_eSPI canvas for splash here

    bottomRightButton = new ActiveRect(160, 160, 80, 80, [](void *unused) {
        LaunchApplication(new MainMenuApplication());
    });
    if ( nullptr == bottomRightButton ) {
        lAppLog("Unable to allocate bottomRightButton\n");
        return;
    }

    Tick(); // OR call this if no splash 
}

WatchfaceSquare::~WatchfaceSquare() {
    // please remove/delete/free all to avoid leaks!
    //delete mywidget;
    delete bottomRightButton;
}

bool WatchfaceSquare::Tick() {

    bottomRightButton->Interact(touched, touchX, touchY);
    // low the brightness if system changes it
    if ( MINIMUM_BACKLIGHT != ttgo->bl->getLevel() ) { ttgo->setBrightness(MINIMUM_BACKLIGHT); }

    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(TFT_BLACK); // use theme colors

        time_t now;
        struct tm * tmpTime;
        struct tm timeinfo;
        time(&now);
        tmpTime = localtime(&now);
        memcpy(&timeinfo,tmpTime, sizeof(struct tm));
        char buffer[64] = { 0 };
        canvas->fillSprite(TFT_BLACK);
        // update day and month        
        canvas->setTextSize(4);
        canvas->setFreeFont(&TomThumb);
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(TL_DATUM);
        sprintf(buffer,"%02d.%02d", timeinfo.tm_mday, timeinfo.tm_mon+1);
        canvas->drawString(buffer, MARGIN_LFT, MARGIN_TOP);
        // update day of week
        int weekday=timeinfo.tm_wday;
        canvas->setTextColor(TFT_CYAN);
        canvas->setTextDatum(TC_DATUM);
        sprintf(buffer,"%s",DAY[weekday]);
        canvas->drawString(buffer, TFT_WIDTH/2+15, MARGIN_TOP);
        // update hours
        canvas->setTextSize(3);
        canvas->setFreeFont(&FreeMonoBold18pt7b);
        canvas->setTextColor(TFT_CYAN);
        canvas->setTextDatum(TR_DATUM);
        sprintf(buffer,"%02d", timeinfo.tm_hour);
        canvas->drawString(buffer, TFT_WIDTH+7, 30);
        // update minutes
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(CR_DATUM);
        sprintf(buffer,"%02d",timeinfo.tm_min);
        canvas->drawString(buffer, TFT_WIDTH+7, TFT_HEIGHT/2+17);
        // update battery 
        canvas->setFreeFont(&TomThumb);
        canvas->setTextSize(4);
        if (batteryPercent > -1) {
            uint32_t battColor = TFT_DARKGREY;
            bool battActivity = false;
            if (vbusPresent) battColor = TFT_GREEN;
            else if (batteryPercent < 10) battColor = ThCol(high);
            else if (batteryPercent < 35) battColor = TFT_YELLOW;
            else battColor = TFT_WHITE;
            canvas->setTextColor(battColor);
            int battFiltered = batteryPercent;
            if (battFiltered > 99) battFiltered = 99;
            sprintf(buffer, "%2d", battFiltered);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString(buffer, TFT_WIDTH-5, MARGIN_TOP);
            if(battFiltered == 0) battFiltered=1;
            canvas->fillRect(TFT_WIDTH-16,MARGIN_TOP,12,(int)(20/(100/battFiltered)),battColor);
        }
        else {
            canvas->setTextColor(TFT_DARKGREY);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString("NB", TFT_WIDTH-10, MARGIN_TOP);
        }

        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI

        return true;
    }
    return false;
}
