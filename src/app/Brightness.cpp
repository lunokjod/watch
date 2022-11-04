#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Settings.hpp"
#include "../static/img_back_32.xbm"
#include "Brightness.hpp"
#include <ArduinoNvs.h>
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../static/img_bright_48.xbm"

BrightnessApplication::~BrightnessApplication() {
    if ( nullptr != btnBack ) { delete btnBack; }
    if ( nullptr != brightGauge ) { delete brightGauge; }
    NVS.setInt("lBright",ttgo->bl->getLevel(),false);    
}

BrightnessApplication::BrightnessApplication() {
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,ttgo->tft->color24to16(0x353e45),false);
    brightGauge = new GaugeWidget(40,40,240-80);
    uint8_t currentValue = ttgo->bl->getLevel()*(360/255);
    brightGauge->selectedAngle = currentValue;

}

bool BrightnessApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY);
    bool change = brightGauge->Interact(touched,touchX, touchY);
    if ( change ) {
        uint8_t currentValue = ((360.0/255)*brightGauge->selectedAngle);
        //Serial.printf("CURENT: %u ANGLE: %d\n", currentValue,brightGauge->selectedAngle);
        ttgo->setBrightness(currentValue);
    }
    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        canvas->drawXBitmap((TFT_WIDTH/2)-(img_bright_48_width/2),
                    (TFT_HEIGHT/2)-(img_bright_48_height/2),
                    img_bright_48_bits,
                    img_bright_48_width,
                    img_bright_48_height,
                    TFT_WHITE);

        btnBack->DrawTo(canvas);
        brightGauge->DrawTo(canvas);
        nextRedraw=millis()+(1000/10);
        return true;
    }
    return false;
}