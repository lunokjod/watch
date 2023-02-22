//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//
// LunokWatch is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later 
// version.
//
// LunokWatch is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
// details.
//
// You should have received a copy of the GNU General Public License along with 
// LunokWatch. If not, see <https://www.gnu.org/licenses/>. 
//

#include <Arduino.h>
#include "SetTime.hpp"
#include "SetDate.hpp"
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo library shit ;)
#include "../static/img_calendar_32.xbm"
#include "../static/img_back_32.xbm"
#include "../static/img_settime_32.xbm"
#include "LogView.hpp"

SetDateApplication::~SetDateApplication() {
    if ( nullptr != day) { delete day; }
    if ( nullptr != month ) { delete month; }
    if ( nullptr != backButton ) { delete backButton; }
    if ( nullptr != showTimeButton ) { delete showTimeButton; }
    if ( nullptr != setDateButton ) { delete setDateButton; }
}


extern int currentDay;
extern int currentMonth;
extern bool ntpSyncDone;

SetDateApplication::SetDateApplication() {
    day = new MujiValue(0,0,165,120,1,31,TFT_BLACK);
    month = new ValueSelector(120,0,165,120,1,12,TFT_BLACK);
    lAppLog("currentDay: %d currentMonth: %d\n",currentDay,currentMonth);
    day->selectedValue = currentDay;
    month->selectedValue = currentMonth+1;
    day->InternalRedraw();
    month->InternalRedraw();
    backButton=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](void *unused){
        LaunchWatchface();
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,canvas->color24to16(0x353e45),false);    
    setDateButton=new ButtonImageXBMWidget(5+64+15,TFT_HEIGHT-69,64,80,[&,this](void *unused){
        lAppLog("SetTime: RTC and localtime sync\n");
        RTC_Date test = ttgo->rtc->getDateTime();
        test.day = day->selectedValue;
        test.month = month->selectedValue;
        ttgo->rtc->setDateTime(test);
        ttgo->rtc->syncToSystem();
        currentDay = day->selectedValue;
        currentMonth = month->selectedValue -1;
        ntpSyncDone=true;

    },img_calendar_32_bits,img_calendar_32_height,img_calendar_32_width,TFT_WHITE,canvas->color24to16(0x353e45));    
    showTimeButton=new ButtonImageXBMWidget(TFT_WIDTH-69,TFT_HEIGHT-69,64,64,[&,this](void *unused){
        LaunchApplication(new SetTimeApplication());
    },img_settime_32_bits,img_settime_32_height,img_settime_32_width,TFT_WHITE,canvas->color24to16(0x353e45),false);    
    Tick();
}
bool SetDateApplication::Tick() {
    showTimeButton->Interact(touched, touchX, touchY);
    setDateButton->Interact(touched, touchX, touchY);
    backButton->Interact(touched, touchX, touchY);
    //if (millis() > nextSelectorTick ) {
        day->Interact(touched,touchX,touchY);
        month->Interact(touched,touchX,touchY);
    //    nextSelectorTick=millis()+(1000/4);
    //}

    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
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
        nextRedraw=millis()+(1000/4);
        return true;
    }
    return false;
}