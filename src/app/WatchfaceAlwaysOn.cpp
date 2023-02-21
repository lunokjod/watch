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
#include "LogView.hpp" // log capabilities
#include "WatchfaceAlwaysOn.hpp"

#define MINIMUM_BACKLIGHT 5

WatchfaceAlwaysOn::WatchfaceAlwaysOn() {
    Tick();
}

WatchfaceAlwaysOn::~WatchfaceAlwaysOn() {
}

bool WatchfaceAlwaysOn::Tick() {
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
        sprintf(buffer,"%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        canvas->setTextSize(2);
        canvas->setTextDatum(CC_DATUM);
        canvas->setFreeFont(&FreeMonoBold18pt7b);
        canvas->setTextColor(TFT_WHITE);
        canvas->fillSprite(TFT_BLACK);
        canvas->drawString(buffer, TFT_WIDTH/2, TFT_HEIGHT/2);

        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
