#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Settings.hpp"
#include "../static/img_back_32.xbm"
#include <ArduinoNvs.h>
#include "Steps.hpp"
#include "StepsSetup.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../lunokiot_config.hpp"

StepsSetupApplication::~StepsSetupApplication() {
    userTall = tallValue->selectedValue;
    NVS.setInt("UserTall",userTall, false);
    userMaleFemale = genderSelect->switchEnabled;
    NVS.setInt("UserSex",userMaleFemale, false);
    if ( nullptr != btnBack ) { delete btnBack; }
    if ( nullptr != genderSelect ) { delete genderSelect; }
    if ( nullptr != tallValue ) { delete tallValue; }
}

StepsSetupApplication::StepsSetupApplication() {
    btnBack=new ButtonImageXBMWidget(TFT_WIDTH-69,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new StepsApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,ttgo->tft->color24to16(0x353e45),false);
    genderSelect = new SwitchWidget(142,20,[&,this](){ });
    tallValue = new ValueSelector(10,10,220,110,120,220,canvas->color24to16(0x212121));
    tallValue->selectedValue = userTall;
    genderSelect->switchEnabled = userMaleFemale;
}

bool StepsSetupApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY);
    genderSelect->Interact(touched,touchX, touchY);
    if (millis() > nextSpinStep ) {
        tallValue->Interact(touched,touchX, touchY);
        nextSpinStep=millis()+(1000/4);
    }
    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        btnBack->DrawTo(canvas);
        genderSelect->DrawTo(canvas);
        tallValue->DrawTo(canvas);

        canvas->setTextFont(0);
        canvas->setTextSize(3);
        canvas->setTextDatum(TL_DATUM);
        canvas->setTextWrap(false,false);
        canvas->setTextColor(TFT_WHITE);
        canvas->drawString("M", 120,40);
        canvas->drawString("F", 210,40);

        canvas->setTextSize(4);
        canvas->setTextDatum(BL_DATUM);
        canvas->drawString("CM", 120,120);
        canvas->setTextSize(3);
        canvas->setTextDatum(TL_DATUM);
        canvas->drawString("Tall", 120,120);

        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}
