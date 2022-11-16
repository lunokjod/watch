#include <Arduino.h>
#include <LilyGoWatch.h>
#include <ArduinoNvs.h>

#include "Brightness.hpp"

#include "../UI/widgets/GaugeWidget.hpp"
#include "../static/img_bright_48.xbm"

#include "LogView.hpp"

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
    brightGauge = new GaugeWidget(40,40,240-80);

    uint8_t userBright = ttgo->bl->getLevel();
    // NVS.getInt("lBright");
    int16_t currentValue = userBright*(360.0/255);
    //ttgo->setBrightness(userBright);
    brightGauge->selectedAngle = currentValue;
    Tick();
}

bool BrightnessApplication::Tick() {
    bool interacted = brightGauge->Interact(touched,touchX, touchY);
    if ( interacted ) {
        uint16_t currentValue = ((255/360.0)*brightGauge->selectedAngle);
        ttgo->setBrightness(currentValue);
    }

    if (millis() > nextRefresh ) {
        
        canvas->fillSprite(ThCol(background));
        TemplateApplication::Tick();

        // sun in the center
        canvas->drawXBitmap((TFT_WIDTH/2)-(img_bright_48_width/2),
                    (TFT_HEIGHT/2)-(img_bright_48_height/2),
                    img_bright_48_bits,
                    img_bright_48_width,
                    img_bright_48_height,
                    TFT_WHITE);

        brightGauge->DrawTo(canvas);
        TemplateApplication::Tick();
        nextRefresh=millis()+(1000/17);
        return true;
    }
    return false;
}
