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

#ifndef __LUNOKIOT__WATCHFACE2_APP__
#define __LUNOKIOT__WATCHFACE2_APP__

#include <Arduino.h>
#include "WatchfaceHandlers.hpp"
#include "../system/Application.hpp"
#include "../UI/activator/ActiveRect.hpp"
#include "../UI/widgets/CanvasZWidget.hpp"
#include "../app/LogView.hpp" // for lLog functions

#include "../static/img_hours_hand.c"
#include "../static/img_minutes_hand.c"
#include "../static/img_seconds_hand.c"
#include "../static/img_wifi_24.xbm"
#include "../static/img_bluetooth_24.xbm"
#include "../static/img_bluetooth_peer_24.xbm"
#include "../static/img_usb_24.xbm"

#include "../static/img_weather_200.c"
#include "../static/img_weather_300.c"
#include "../static/img_weather_500.c"
#include "../static/img_weather_600.c"
#include "../static/img_weather_800.c"

#include "MainMenu.hpp"
#include "Stopwatch.hpp"

class Watchface2Application: public LunokIoTApplication {
    protected:
        unsigned long nextRefresh=0;
        CanvasZWidget * colorBuffer = nullptr;
        CanvasZWidget * outherSphere = nullptr;
        CanvasZWidget * innerSphere = nullptr;
        CanvasZWidget * hourHandCanvas = nullptr;
        CanvasZWidget * minuteHandCanvas = nullptr;
        ActiveRect * bottomRightButton = nullptr;
        int16_t middleX;
        int16_t middleY;
        int16_t radius;
        const int16_t margin = 5;
        const float DEGREE_HRS = (360/24);
        int markAngle=0;
        // void Handlers();
    public:
        const char *AppName() override { return "Analogic watchface"; };

        Watchface2Application();
        virtual ~Watchface2Application();
        bool Tick();
        const bool isWatchface() override { return true; }
        const bool mustShowAsTask() override { return false; }
        void LowMemory();

};

#endif
