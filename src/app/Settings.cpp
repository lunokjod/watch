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

#include "Settings.hpp"

#include <Arduino.h>
#include <ArduinoNvs.h>

#include <libraries/TFT_eSPI/TFT_eSPI.h>

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"

#include "../static/img_back_32.xbm"
#include "../static/img_wifi_32.xbm"
#include "../static/img_bluetooth_32.xbm"
#include "../static/img_help_32.xbm"

#include "../system/Network.hpp"
#include "LogView.hpp"
// #include "Watchface2.hpp"
#include "WatchfaceSquare.hpp"
extern void StopBLE();
extern void StartBLE(bool synched=false);

SettingsApplication::~SettingsApplication() {
    // on destroy
    // set the switch values
    NVS.setInt("BLEEnabled",bleCheck->switchEnabled,false);
    NVS.setInt("WifiEnabled",wifiCheck->switchEnabled,false);

    NVS.setInt("NTPEnabled",ntpCheck->switchEnabled,false);

    // if ( nullptr != WatchfaceSquare::ntpTask ) { 
    //     WatchfaceSquare::ntpTask->enabled = ntpCheck->switchEnabled;
    // }
    NVS.setInt("OWeatherEnabled",openweatherCheck->switchEnabled,false);
    if ( nullptr != WatchfaceSquare::weatherTask ) {
        WatchfaceSquare::weatherTask->enabled = openweatherCheck->switchEnabled;
    }
    if ( bleCheck->switchEnabled ) { StartBLE(); }
    else { StopBLE(); }
    //delete btnHelp;
    delete ntpCheck;
    delete openweatherCheck;
    delete wifiCheck;
    delete bleCheck;
}

SettingsApplication::SettingsApplication() {

    wifiCheck=new SwitchWidget(80,110,[&,this](void *unused){
        // this delay is to get the current value of switch :( due multitasking env
        TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
        BaseType_t isDelayed = xTaskDelayUntil( &nextCheck, (100 / portTICK_PERIOD_MS) ); // wait a ittle bit
    
        ntpCheck->SetEnabled(wifiCheck->switchEnabled);
        // key defined?
        if ( 0 == strlen(openWeatherMapApiKey)) {
            openweatherCheck->SetEnabled(false);
        } else {
            openweatherCheck->SetEnabled(wifiCheck->switchEnabled);
        }
    });
    wifiCheck->switchEnabled=(bool)NVS.getInt("WifiEnabled");
    wifiCheck->InternalRedraw();

    bleCheck=new SwitchWidget(80,160);
    bleCheck->switchEnabled=NVS.getInt("BLEEnabled");
    bleCheck->InternalRedraw();

    ntpCheck=new SwitchWidget(10,10);
    ntpCheck->switchEnabled=NVS.getInt("NTPEnabled");
    ntpCheck->enabled=wifiCheck->switchEnabled;
    ntpCheck->InternalRedraw();

    openweatherCheck=new SwitchWidget(10,60);
    openweatherCheck->switchEnabled=NVS.getInt("OWeatherEnabled");
    openweatherCheck->enabled=wifiCheck->switchEnabled;
    if ( 0 == strlen(openWeatherMapApiKey)) {
        openweatherCheck->SetEnabled(false);
    }
    openweatherCheck->InternalRedraw();

    Tick();
}

bool SettingsApplication::Tick() {
    
    //TemplateApplication::Tick();
    TemplateApplication::btnBack->Interact(touched,touchX, touchY);
    //btnHelp->Interact(touched,touchX, touchY);
    wifiCheck->Interact(touched,touchX, touchY);
    bleCheck->Interact(touched,touchX, touchY);

    ntpCheck->Interact(touched,touchX, touchY);
    openweatherCheck->Interact(touched,touchX, touchY);


    if (millis() > nextRefresh ) {
        canvas->fillSprite(ThCol(background));
        //btnHelp->DrawTo(canvas);
        TemplateApplication::Tick();
        //btnBack->DrawTo(canvas);
        ntpCheck->DrawTo(canvas);
        openweatherCheck->DrawTo(canvas);
        wifiCheck->DrawTo(canvas);
        bleCheck->DrawTo(canvas);

        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextDatum(TL_DATUM);

        uint16_t textColor = ThCol(text);
        if ( false == wifiCheck->switchEnabled ) { textColor = ThCol(text_alt); }
        canvas->setTextColor(textColor);
        canvas->drawString("NTP",95,35);
        canvas->drawString("OpenWeather",95,85);

#ifdef LUNOKIOT_WIFI_ENABLED
        textColor = ThCol(text);
#else
        textColor = ThCol(text_alt);
#endif
        canvas->drawXBitmap(160,130,img_wifi_32_bits,img_wifi_32_width,img_wifi_32_height,textColor);
#ifdef LUNOKIOT_BLE_ENABLED
        textColor = ThCol(text);
#else
        textColor = ThCol(text_alt);
#endif
        canvas->drawXBitmap(165,176,img_bluetooth_32_bits,img_bluetooth_32_width,img_bluetooth_32_height,textColor);
        nextRefresh=millis()+(1000/6);
        return true;
    }
    return false;
}