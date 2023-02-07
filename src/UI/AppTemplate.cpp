//#include <Arduino.h>
#include "AppTemplate.hpp"
//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include "../lunokiot_config.hpp"

#include "../app/LogView.hpp"   // for lLog functions
#include "../static/img_backscreen_24.xbm" // system back screen image
//#include "../static/img_back_32.xbm"              // old back button
#include "../UI/widgets/ButtonImageXBMWidget.hpp" // back button

TemplateApplication::~TemplateApplication() { delete btnBack; }

TemplateApplication::TemplateApplication() {
    btnBack=new ButtonImageXBMWidget(0,canvas->height()-32,32,32,[&,this](void *unused){
        LaunchWatchface();
    },img_backscreen_24_bits,img_backscreen_24_height,img_backscreen_24_width,ThCol(background_alt),0,false);
}

bool TemplateApplication::Tick() {
    bool backTap = btnBack->Interact(touched,touchX, touchY); 
    if ( backTap ) {
        btnBack->DirectDraw();
    } else {
    //if ( millis() > nextRefresh ) {
        btnBack->DrawTo(canvas);
    }
    return backTap;
}
