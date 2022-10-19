#include <Arduino.h>
#include <LilyGoWatch.h>
#include "SetTime.hpp"
#include "../static/img_back_32.xbm"
#include "../static/img_settime_32.xbm"
#include "Watchface.hpp"

SetTimeApplication::~SetTimeApplication() {
    if ( nullptr != hour ) {
        delete hour;
        hour = nullptr;
    }
    if ( nullptr != minute ) {
        delete minute;
        minute = nullptr;
    }
    if ( nullptr != backButton ) {
        delete backButton;
        backButton = nullptr;
    }
    if ( nullptr != setTimeButton ) {
        delete setTimeButton;
        setTimeButton = nullptr;
    }
}

extern int currentSec;
extern int currentMin;
extern int currentHour;

SetTimeApplication::SetTimeApplication() {
    hour = new MujiValue(0,0,165,120,0,23,TFT_BLACK);
    minute = new ValueSelector(120,0,165,120,0,59,TFT_BLACK);
    hour->selectedValue = currentHour;
    minute->selectedValue = currentMin;
    backButton=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,canvas->color24to16(0x353e45),false);    
    setTimeButton=new ButtonImageXBMWidget(5+64+15,TFT_HEIGHT-69,64,150,[&,this](){
        Serial.println("SetTime: RTC and localtime sync");
        RTC_Date test;
        test.hour = hour->selectedValue+1;
        test.minute = minute->selectedValue;
        ttgo->rtc->setDateTime(test);
        ttgo->rtc->syncToSystem();
    },img_settime_32_bits,img_settime_32_height,img_settime_32_width,TFT_WHITE,canvas->color24to16(0x353e45));    

}
bool SetTimeApplication::Tick() {
    setTimeButton->Interact(touched, touchX, touchY);
    backButton->Interact(touched, touchX, touchY);
    if (millis() > nextBlinkTime ) {
        blinkTime=(!blinkTime);
        nextBlinkTime=millis()+1000;
    }
    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        hour->Interact(touched, touchX, touchY);
        minute->Interact(touched, touchX, touchY);
        hour->DrawTo(canvas);
        minute->DrawTo(canvas);
        setTimeButton->DrawTo(canvas);
        backButton->DrawTo(canvas);
        if ( blinkTime ) {
            canvas->fillRect((TFT_HEIGHT/2)-5,(TFT_WIDTH/2)-50,10,10,TFT_WHITE);
            canvas->fillRect((TFT_HEIGHT/2)-5,(TFT_WIDTH/2)-35,10,10,TFT_WHITE);
        }
        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}