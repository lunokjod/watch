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

#ifndef __LUNOKIOT__BLEMONITORDEVICE_APP__
#define __LUNOKIOT__BLEMONITORDEVICE_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>

#include "../system/Network.hpp"
#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonTextWidget.hpp"

class BLEDeviceMonitorApplication: public TemplateApplication {
    private:
        unsigned long nextRedraw=0;
        int rotateVal = 0;
        uint16_t backgroundColor;
        char *deviceMAC=nullptr;
        ButtonTextWidget *pairBtn=nullptr;
        // copy of lBLEDevice
        char * devName = nullptr;
        unsigned long firstSeen = 0; // @TODO use time_t instead
        unsigned long lastSeen = 0;
        size_t seenCount = 0;
        int rssi = 0;
        int8_t txPower = 0;
        double distance = -1;

    public:
        const char *AppName() override { return "BLE device inspector"; };
        BLEDeviceMonitorApplication(uint16_t baseColor, char *devMAC);
        ~BLEDeviceMonitorApplication();
        bool Tick();
};

#endif
