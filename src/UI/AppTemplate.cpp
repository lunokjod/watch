#include "AppTemplate.hpp"

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../lunokiot_config.hpp"

#include "../app/Watchface.hpp" // for back button
#include "../app/LogView.hpp"   // for lLog functions

#include "../static/img_back_32.xbm"              // back button
#include "../UI/widgets/ButtonImageXBMWidget.hpp" // back button


TemplateApplication::~TemplateApplication() { delete btnBack; }

TemplateApplication::TemplateApplication() {
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,ThCol(text),ThCol(button),false);
}

bool TemplateApplication::Tick() {
    bool interacted = btnBack->Interact(touched,touchX, touchY);
    if ( millis() > nextRefresh ) {
        btnBack->DrawTo(canvas);
    }
    return interacted;
}
