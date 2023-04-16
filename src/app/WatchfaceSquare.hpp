//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//    Watchface Author: @hpsaturn
//    License: GPLv3
//----------------------------------------------------------------

#ifndef __LUNOKIOT__WATCHFACESQUARE_APP__
#define __LUNOKIOT__WATCHFACESQUARE_APP__

#include <Arduino.h>
#include <WiFi.h>
#include "../system/Application.hpp"
#include "../system/SystemEvents.hpp"
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

#include "../resources.hpp"
/*
#include "../static/img_weather_200.c"
#include "../static/img_weather_300.c"
#include "../static/img_weather_500.c"
#include "../static/img_weather_600.c"
#include "../static/img_weather_800.c"
*/
#include "../static/img_step_32.xbm"
#include "../static/img_distance_32.xbm"

#include "LuIMainMenu.hpp"
#include "Steps.hpp"
#include "Calendar.hpp"
#include "Battery.hpp"
#include "Settings.hpp"

class WatchfaceSquare : public LunokIoTApplication {
    protected:
        unsigned long nextRefresh=0;
        ActiveRect *topRightButton = nullptr;
        ActiveRect *bottomRightButton = nullptr;
        ActiveRect *topLeftButton = nullptr;
        ActiveRect *bottomLeftButton = nullptr;
    public:
        const char *AppName() override { return "Square watchface"; };

        WatchfaceSquare();
        virtual ~WatchfaceSquare();
        bool Tick();
        const bool isWatchface() override { return true; }
        const bool mustShowAsTask() override { return false; }
        void LowMemory();
};

#endif
