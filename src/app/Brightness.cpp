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
    TemplateApplication();
    brightGauge = new GaugeWidget(20,10,200);

    uint8_t userBright = ttgo->bl->getLevel();
    // NVS.getInt("lBright");
    int16_t currentValue = userBright*(360.0/255);
    //ttgo->setBrightness(userBright);
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
                    ((TFT_HEIGHT/2)-(img_bright_48_height/2)-14),
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
