#include <Arduino.h>
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo library shit ;)



#include "SetTime.hpp"
#include "SetDate.hpp"
#include "LogView.hpp"

#include "../static/img_calendar_32.xbm"
#include "../static/img_back_32.xbm"
#include "../static/img_settime_32.xbm"


SetTimeApplication::~SetTimeApplication() {
    if ( nullptr != hour ) { delete hour; }
    if ( nullptr != minute ) { delete minute; }
    if ( nullptr != backButton ) { delete backButton; }
    if ( nullptr != showDateButton ) { delete showDateButton; }
    if ( nullptr != setTimeButton ) { delete setTimeButton; }
}

extern int currentMin;
extern int currentHour;
extern bool ntpSyncDone;

SetTimeApplication::SetTimeApplication() {
    hour = new MujiValue(0,0,165,120,0,23,TFT_BLACK);
    minute = new ValueSelector(120,0,165,120,0,59,TFT_BLACK);
    lAppLog("currentMin: %d currentHour: %d\n",currentMin,currentHour);
    hour->selectedValue = currentHour;
    hour->InternalRedraw();
    minute->selectedValue = currentMin;
    minute->InternalRedraw();
    backButton=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchWatchface();
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,canvas->color24to16(0x353e45),false);    
    setTimeButton=new ButtonImageXBMWidget(5+64+15,TFT_HEIGHT-69,64,80,[&,this](){
        //Serial.println("SetTime: RTC and localtime sync");
        RTC_Date test = ttgo->rtc->getDateTime();
        test.hour = hour->selectedValue;
        test.minute = minute->selectedValue;
        ttgo->rtc->setDateTime(test);
        ttgo->rtc->syncToSystem();
        currentHour = hour->selectedValue;
        currentMin = minute->selectedValue;
        ntpSyncDone = true;

    },img_settime_32_bits,img_settime_32_height,img_settime_32_width,TFT_WHITE,canvas->color24to16(0x353e45));    
    showDateButton=new ButtonImageXBMWidget(TFT_WIDTH-69,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new SetDateApplication());
    },img_calendar_32_bits,img_calendar_32_height,img_calendar_32_width,TFT_WHITE,canvas->color24to16(0x353e45),false);
    Tick();
}
bool SetTimeApplication::Tick() {
    bool change=false;
    showDateButton->Interact(touched, touchX, touchY);
    setTimeButton->Interact(touched, touchX, touchY);
    backButton->Interact(touched, touchX, touchY);
    if (millis() > nextBlinkTime ) {
        blinkTime=(!blinkTime);
        nextBlinkTime=millis()+1000;
    }
    //if (millis() > nextSelectorTick ) {
        hour->Interact(touched, touchX, touchY);
        minute->Interact(touched, touchX, touchY);
    //    nextSelectorTick=millis()+(1000/4);
    //}
    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        hour->DrawTo(canvas);
        minute->DrawTo(canvas);
        setTimeButton->DrawTo(canvas);
        backButton->DrawTo(canvas);
        showDateButton->DrawTo(canvas);
        if ( blinkTime ) {
            canvas->fillRect((TFT_HEIGHT/2)-5,(TFT_WIDTH/2)-50,10,10,TFT_WHITE);
            canvas->fillRect((TFT_HEIGHT/2)-5,(TFT_WIDTH/2)-35,10,10,TFT_WHITE);
        }
        canvas->setTextFont(0);
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextSize(2);
        canvas->setTextDatum(TL_DATUM);
        canvas->drawString("Hour",20, 10);
        canvas->setTextDatum(TR_DATUM);
        canvas->drawString("Minute",TFT_WIDTH-20,10);

        nextRedraw=millis()+(1000/4);
        return true;
    }
    return false;
}