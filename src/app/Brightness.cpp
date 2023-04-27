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
#include <ArduinoNvs.h>

#include "Brightness.hpp"

#include "../UI/widgets/GaugeWidget.hpp"
#include "../static/img_bright_48.xbm"

#include "LogView.hpp"

#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo lib

BrightnessApplication::~BrightnessApplication() {
    uint16_t currentValue = ((255/360.0)*brightGauge->selectedAngle);
    if (currentValue != 0) {
        ttgo->setBrightness(currentValue);
        NVS.setInt("lBright",currentValue,false);
    };
    delete brightGauge;
}

BrightnessApplication::BrightnessApplication() {
    brightGauge = new GaugeWidget(10,10,220);

    uint8_t userBright = ttgo->bl->getLevel();
    int16_t currentValue = userBright*(360.0/255);
    brightGauge->selectedAngle = currentValue;
    Tick();
}

bool BrightnessApplication::Tick() {
    if ( brightGauge->Interact(touched,touchX, touchY) ) {
        uint16_t currentValue = ((255/360.0)*brightGauge->selectedAngle);
        ttgo->setBrightness(currentValue);
        brightGauge->DirectDraw();
        return false;
    }
    TemplateApplication::Tick();
    if (millis() > nextRefresh ) {
        canvas->fillSprite(ThCol(background));
        // sun in the center
        canvas->drawXBitmap((TFT_WIDTH/2)-(img_bright_48_width/2),
                    ((TFT_HEIGHT/2)-(img_bright_48_height/2)),
                    img_bright_48_bits,
                    img_bright_48_width,
                    img_bright_48_height,
                    TFT_WHITE);
        brightGauge->DrawTo(canvas);
        btnBack->DrawTo(canvas);
        nextRefresh=millis()+(1000/3);
        return true;
    }
    return false;
}
