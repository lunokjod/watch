#include <Arduino.h>
#include <LilyGoWatch.h>
#include "SetTimeZone.hpp"
#include "../static/img_back_32.xbm"
#include "../static/img_timezone_32.xbm"
#include "Watchface.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include <ArduinoNvs.h>

SetTimeZoneApplication::~SetTimeZoneApplication() {
    if ( nullptr != btnBack ) { delete btnBack; }
    if ( nullptr != switchDaylight ) { delete switchDaylight; }
    if ( nullptr != timezoneGMTSelector ) { delete timezoneGMTSelector; }
    if ( nullptr != btnSetGMT ) { delete btnSetGMT; }
}

SetTimeZoneApplication::SetTimeZoneApplication() {
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,ttgo->tft->color24to16(0x353e45),false);

    switchDaylight=new SwitchWidget(10,10, [&,this]() { }, canvas->color24to16(0x555f68));

    btnSetGMT=new ButtonImageXBMWidget(70,TFT_HEIGHT-69,64,110,[&,this](){
        Serial.printf("Summer time: %s\n",(switchDaylight->switchEnabled?"true":"false"));
        Serial.printf("GMT: %d\n",timezoneGMTSelector->selectedValue);

        NVS.setInt("summerTime",switchDaylight->switchEnabled, false);
        NVS.setInt("timezoneTime",timezoneGMTSelector->selectedValue, false);

    },img_timezone_32_bits,img_timezone_32_height,img_timezone_32_width,TFT_WHITE, ttgo->tft->color24to16(0x353e45));

    timezoneGMTSelector = new ValueSelector(120,60,100,115,-12,12,
    canvas->color24to16(0x212121),true);

    int daylight = NVS.getInt("summerTime");
    long timezone = NVS.getInt("timezoneTime");
    Serial.printf("Summer time: %s\n",(daylight?"true":"false"));
    Serial.printf("GMT: %d\n",timezone);
    timezoneGMTSelector->selectedValue=timezone;
    switchDaylight->switchEnabled=daylight;
}

bool SetTimeZoneApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY);
    switchDaylight->Interact(touched,touchX, touchY);
    btnSetGMT->Interact(touched,touchX, touchY);
    if (millis() > nextSelectorTick ) {
        timezoneGMTSelector->Interact(touched,touchX,touchY);
        nextSelectorTick=millis()+(1000/4);
    }
    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        btnBack->DrawTo(canvas);
        switchDaylight->DrawTo(canvas);
        timezoneGMTSelector->DrawTo(canvas);
        btnSetGMT->DrawTo(canvas);
        canvas->setTextFont(0);
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextSize(2);
        canvas->setTextDatum(TL_DATUM);
        canvas->drawString("Daylight", 90, 36);

        canvas->setTextSize(4);
        canvas->setTextDatum(CR_DATUM);
        canvas->drawString("GMT", 120, 110);

        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}