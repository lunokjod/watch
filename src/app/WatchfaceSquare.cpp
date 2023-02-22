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
        canvas->setTextSize(4);
        canvas->setFreeFont(&FreeMonoBold18pt7b);
        canvas->setTextColor(TFT_CYAN);
        canvas->fillSprite(TFT_BLACK);
        canvas->setTextDatum(TR_DATUM);
        sprintf(buffer,"%02d", timeinfo.tm_hour);
        canvas->drawString(buffer, TFT_WIDTH, 15);
        sprintf(buffer,"%02d",timeinfo.tm_min);
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(BR_DATUM);
        canvas->drawString(buffer, TFT_WIDTH, TFT_HEIGHT);
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
