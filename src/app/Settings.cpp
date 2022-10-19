#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Settings.hpp"
#include "../static/img_back_32.xbm"
#include "Watchface.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"

SettingsApplication::~SettingsApplication() {
    if ( nullptr != btnBack ) {
        delete btnBack;
        btnBack = nullptr;
    }
    if ( nullptr != test ) {
        delete test;
        test = nullptr;
    }
}

SettingsApplication::SettingsApplication() {
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,canvas->color24to16(0x353e45));
    // int16_t x, int16_t y, std::function<void ()> notifyTo, uint32_t switchColor=TFT_DARKGREY
    test=new SwitchWidget(10,10, [&,this]() {
        Serial.printf("@TODO Value: %s\n",(test->switchEnabled?"true":"false"));
    }, canvas->color24to16(0x555f68));
}

bool SettingsApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY);
    test->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        btnBack->DrawTo(canvas);
        test->DrawTo(canvas);
        nextRedraw=millis()+(1000/3);
    }
    return true;
}