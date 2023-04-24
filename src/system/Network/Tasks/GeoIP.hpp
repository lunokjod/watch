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


#ifndef ___LUNOKIOT__GEOIP__WIFI_TASK___
#define ___LUNOKIOT__GEOIP__WIFI_TASK___

#include "../WiFi.hpp"
#include <LilyGoWatch.h>

class GeoIPWifiTask: public LoTWiFiTask {
    private:
        const unsigned long Every3H = 1000*60*60*3;
        unsigned long nextShoot = 0;
        const char *url = "http://www.geoplugin.net/json.gp";
        char *geoIPReceivedData = nullptr;
        //char *weatherCity = nullptr;
        //char *weatherCountry = nullptr;
    public:
        GeoIPWifiTask() {};
        ~GeoIPWifiTask() override {};
        const char * Name() override { return "GeoIP"; }
        void Launch() override;
        bool Check() override;
};

#endif
