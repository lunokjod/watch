#include "AppTemplate.hpp"

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../lunokiot_config.hpp"

#include "../app/Watchface.hpp" // for back button
#include "../app/LogView.hpp"   // for lLog functions

#include "../static/img_back_32.xbm"              // back button
#include "../UI/widgets/ButtonImageXBMWidget.hpp" // back button


TemplateApplication::~TemplateApplication() {
    if ( nullptr != btnBack ) { delete btnBack; }
}

TemplateApplication::TemplateApplication() {
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,ttgo->tft->color24to16(0x353e45),false);

    Tick(); // splash
}

bool TemplateApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        btnBack->DrawTo(canvas);
        nextRedraw=millis()+(1000/10);
        return true;
    }
    return false;
}
