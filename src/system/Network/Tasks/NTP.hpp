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


#ifndef ___LUNOKIOT__NTP__WIFI_TASK___
#define ___LUNOKIOT__NTP__WIFI_TASK___

#include "../WiFi.hpp"
#include <LilyGoWatch.h>

class NTPWifiTask: public LoTWiFiTask {
    private:
        const unsigned long Every24H = 1000*60*60*24;
        unsigned long nextShoot = 0;
        PCF8563_Class * rtc = nullptr;
    public:
        const static char * ntpServer;
        NTPWifiTask(PCF8563_Class * rtc=nullptr);
        ~NTPWifiTask() override;
        const char * Name() override { return "NTP"; }
        void Launch() override;
        bool Check() override;
};

#endif
