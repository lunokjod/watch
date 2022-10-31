#include <Arduino.h>
#include <LilyGoWatch.h>
#include "SetTime.hpp"
#include "SetDate.hpp"

#include "../static/img_calendar_32.xbm"
#include "../static/img_back_32.xbm"
#include "../static/img_settime_32.xbm"
#include "Watchface.hpp"

SetDateApplication::~SetDateApplication() {
    if ( nullptr != day) { delete day; }
    if ( nullptr != month ) { delete month; }
    if ( nullptr != backButton ) { delete backButton; }
    if ( nullptr != showTimeButton ) { delete showTimeButton; }
    if ( nullptr != setDateButton ) { delete setDateButton; }
}

extern int currentDay;
extern int currentMonth;

SetDateApplication::SetDateApplication() {
    day = new MujiValue(0,0,165,120,1,31,TFT_BLACK);
    month = new ValueSelector(120,0,165,120,1,12,TFT_BLACK);
    day->selectedValue = currentDay+1;
    month->selectedValue = currentMonth+1;
    backButton=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,canvas->color24to16(0x353e45),false);    
    setDateButton=new ButtonImageXBMWidget(5+64+15,TFT_HEIGHT-69,64,80,[&,this](){
        Serial.println("SetTime: RTC and localtime sync");
        RTC_Date test;
        test.day = day->selectedValue-1;
        test.month = month->selectedValue-1;
        ttgo->rtc->setDateTime(test);
        ttgo->rtc->syncToSystem();
    },img_calendar_32_bits,img_calendar_32_height,img_calendar_32_width,TFT_WHITE,canvas->color24to16(0x353e45));    
    showTimeButton=new ButtonImageXBMWidget(TFT_WIDTH-69,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new SetTimeApplication());
    },img_settime_32_bits,img_settime_32_height,img_settime_32_width,TFT_WHITE,canvas->color24to16(0x353e45),false);    
}
bool SetDateApplication::Tick() {
    showTimeButton->Interact(touched, touchX, touchY);
    setDateButton->Interact(touched, touchX, touchY);
    backButton->Interact(touched, touchX, touchY);

    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        day->Interact(touched, touchX, touchY);
        month->Interact(touched, touchX, touchY);
        day->DrawTo(canvas);
        month->DrawTo(canvas);
        setDateButton->DrawTo(canvas);
        backButton->DrawTo(canvas);
        showTimeButton->DrawTo(canvas);
        canvas->setTextFont(0);
        canvas->setTextSize(6);
        canvas->setTextDatum(CC_DATUM);
        canvas->setTextColor(TFT_WHITE);
        canvas->drawString("/",TFT_WIDTH/2,80);
        canvas->setTextSize(2);
        canvas->setTextDatum(TL_DATUM);
        canvas->drawString("Day",20, 10);
        canvas->setTextDatum(TR_DATUM);
        canvas->drawString("Month",TFT_WIDTH-20,10);
        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}