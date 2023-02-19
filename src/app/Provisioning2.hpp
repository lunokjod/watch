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

#ifndef __LUNOKIOT__PROVISIONING2_APP__
#define __LUNOKIOT__PROVISIONING2_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include <qrcode.h>

class Provisioning2Application: public TemplateApplication {
    public:
        const char *AppName() override { return "WiFi provisioning"; };
        unsigned long nextRedraw=0;
        char *pop=nullptr;
        char *service_key=nullptr;
        char service_name[32];
        bool provisioningStarted = false;
        bool qrVisible = false;
        bool useBluetooth = false;

        TFT_eSprite * currentQRRendered = nullptr;
        QRCode currentQR;
        uint8_t *currentQRData = nullptr;;

        void GenerateQRCode();
        SwitchWidget * wifiOrBLE = nullptr;
        ButtonImageXBMWidget * clearProvBtn=nullptr;
        ButtonImageXBMWidget * startProvBtn=nullptr;
        Provisioning2Application();
        ~Provisioning2Application();
        bool Tick();
        void GenerateCredentials();
};

#endif
