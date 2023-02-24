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

#ifndef __LUNOKIOT__APPLICATION__BASE__
#define __LUNOKIOT__APPLICATION__BASE__

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <ArduinoNvs.h>
#include "LogView.hpp" // log capabilities
#include "../app/LogView.hpp" // for lLog functions
#include "../system/SystemEvents.hpp"
#include "../system/Datasources/database.hpp"
#include "../system/Network.hpp"
#include "../system/Application.hpp"
#include "../UI/activator/ActiveRect.hpp"
#include "../UI/widgets/CanvasZWidget.hpp"

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
#include "Steps.hpp"
#include "Calendar.hpp"
#include "Battery.hpp"
#include "Settings.hpp"

class WatchfaceSquare : public LunokIoTApplication {
    protected:
        ActiveRect *topRightButton = nullptr;
        ActiveRect *bottomRightButton = nullptr;
        ActiveRect *topLeftButton = nullptr;
        ActiveRect *bottomLeftButton = nullptr;
        void Handlers();

    private:
        char *weatherCity = nullptr;
        char *weatherCountry = nullptr;
        int weatherId = -1;
        char *weatherMain = nullptr;
        char *weatherDescription = nullptr;
        char *weatherIcon = nullptr;
        double weatherTemp = -1000;
        char *weatherReceivedData = nullptr;
        char *geoIPReceivedData = nullptr;
        String jsonBuffer;
        unsigned long nextRefresh=0;
        bool weatherSyncDone = false;
    public:
        const char *AppName() override { return "Square Watchface"; };
        static void FreeRTOSEventReceived(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
        bool wifiEnabled = false;
        bool GetSecureNetworkWeather();
        bool ParseWeatherData();
        String httpGETRequest(const char* serverName);
        static NetworkTaskDescriptor * geoIPTask;
        static NetworkTaskDescriptor * weatherTask;
        WatchfaceSquare();
        ~WatchfaceSquare();
        bool Tick();
};

#endif
