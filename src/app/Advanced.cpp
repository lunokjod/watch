#include <Arduino.h>
#include <ArduinoNvs.h>
#include <LilyGoWatch.h>
#include "Advanced.hpp"
#include "LogView.hpp"
#include "../static/img_back_32.xbm"
#include "../static/img_trash_32.xbm"
#include "../static/img_log_32.xbm"
#include "../static/img_help_32.xbm"
#include "../UI/UI.hpp"
#include "Shutdown.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"

AdvancedSettingsApplication::~AdvancedSettingsApplication() {
    if ( nullptr != btnBack ) { delete btnBack; }
    if ( nullptr != btnErase ) { delete btnErase; }
    if ( nullptr != btnLog ) { delete btnLog; }
}

AdvancedSettingsApplication::AdvancedSettingsApplication() {
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](void *bah){
        LaunchWatchface();
    },img_back_32_bits,img_back_32_height,img_back_32_width,ThCol(text),ThCol(button),false);
    btnErase=new ButtonImageXBMWidget(5,5,64,64,[&,this](void *bah){
        NVS.eraseAll(true);
        LaunchApplication(new ShutdownApplication(true,false));
    },img_trash_32_bits,img_trash_32_height,img_trash_32_width,ThCol(text),ThCol(high));
    btnLog=new ButtonImageXBMWidget(20,80,64,200,[&,this](void *bah){
        LaunchApplication(new LogViewApplication());
    },img_log_32_bits,img_log_32_height,img_log_32_width,ThCol(text),ThCol(background_alt));

    Tick();
}

bool AdvancedSettingsApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY);
    btnErase->Interact(touched,touchX, touchY);
    btnLog->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        btnBack->DrawTo(canvas);
        btnErase->DrawTo(canvas);
        btnLog->DrawTo(canvas);
        nextRedraw=millis()+(1000/10);
        return true;
    }
    return false;
}
