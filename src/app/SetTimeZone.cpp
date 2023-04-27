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
#include <LilyGoWatch.h>
#include "SetTimeZone.hpp"
#include "../static/img_back_32.xbm"
#include "../static/img_timezone_32.xbm"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
//#include <ArduinoNvs.h>
#include "LogView.hpp"
#include "../lunokIoT.hpp"

SetTimeZoneApplication::~SetTimeZoneApplication() {
    LoT().GetSettings()->SetInt(SystemSettings::SettingKey::Daylight,switchDaylight->switchEnabled);
    //NVS.setInt("summerTime",switchDaylight->switchEnabled, false);
    LoT().GetSettings()->SetInt(SystemSettings::SettingKey::TimeZone,timezoneGMTSelector->selectedValue);
    //NVS.setInt("timezoneTime",timezoneGMTSelector->selectedValue, false);
    if ( nullptr != btnBack ) { delete btnBack; }
    if ( nullptr != switchDaylight ) { delete switchDaylight; }
    if ( nullptr != timezoneGMTSelector ) { delete timezoneGMTSelector; }
    if ( nullptr != btnSetGMT ) { delete btnSetGMT; }
}

SetTimeZoneApplication::SetTimeZoneApplication() {
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](void *unused){
        LaunchWatchface();
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,canvas->color24to16(0x353e45),false);

    switchDaylight=new SwitchWidget(10,10, [&,this](void *unused) { }, canvas->color24to16(0x555f68));

    btnSetGMT=new ButtonImageXBMWidget(70,TFT_HEIGHT-69,64,110,[&,this](void *unused){
        lAppLog("Summer time: '%s', GMT: %+d\n",(switchDaylight->switchEnabled?"yes":"no"),timezoneGMTSelector->selectedValue);
    },img_timezone_32_bits,img_timezone_32_height,img_timezone_32_width,TFT_WHITE, canvas->color24to16(0x353e45));

    timezoneGMTSelector = new ValueSelector(120,60,100,115,-12,12,
    canvas->color24to16(0x212121),true);
    bool daylight = LoT().GetSettings()->GetInt(SystemSettings::SettingKey::Daylight);
    long timezone = LoT().GetSettings()->GetInt(SystemSettings::SettingKey::TimeZone);
    lAppLog("Summer time: '%s', GMT: %+d\n",(daylight?"yes":"no"),timezone);
    timezoneGMTSelector->selectedValue=timezone;
    switchDaylight->switchEnabled=daylight;
    switchDaylight->InternalRedraw();
    timezoneGMTSelector->InternalRedraw();
    Tick();
}

bool SetTimeZoneApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY);
    switchDaylight->Interact(touched,touchX, touchY);
    btnSetGMT->Interact(touched,touchX, touchY);
    //if (millis() > nextSelectorTick ) {
        timezoneGMTSelector->Interact(touched,touchX,touchY);
    //    nextSelectorTick=millis()+(1000/4);
    //}
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

        nextRedraw=millis()+(1000/4);
        return true;
    }
    return false;
}