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

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../lunokIoT.hpp"
#include "../static/img_back_32.xbm"
#include "../static/img_wifi_32.xbm"
#include "../static/img_bluetooth_32.xbm"
#include "../static/img_help_32.xbm"

#include "../system/Network.hpp"
#include "LogView.hpp"

#include "../system/Network.hpp"
extern NetworkTaskDescriptor * NetworkNTPTask;
extern NetworkTaskDescriptor * NetworkWeatherTask;
extern NetworkTaskDescriptor * NetworkGeoIPTask;

extern void StopBLE();
extern void StartBLE(bool synched);

SettingsApplication::~SettingsApplication() {
    // on destroy
    // set the switch values
    NVS.setInt("BLEEnabled",bleCheck->switchEnabled,false);
    NVS.setInt("NTPBLEEnabled",ntpBLECheck->switchEnabled,false);

    NVS.setInt("WifiEnabled",wifiCheck->switchEnabled,false);

    NVS.setInt("NTPEnabled",ntpCheck->switchEnabled,false);
    if ( nullptr != NetworkNTPTask ) { 
        NetworkNTPTask->enabled = ntpCheck->switchEnabled;
    }
    NVS.setInt("OWeatherEnabled",openweatherCheck->switchEnabled,false);
    if ( nullptr != NetworkWeatherTask ) {
        NetworkWeatherTask->enabled = openweatherCheck->switchEnabled;
        NetworkGeoIPTask->enabled = openweatherCheck->switchEnabled;
    }
    if ( bleCheck->switchEnabled ) { StartBLE(); }
    else { StopBLE(); }
    if ( nullptr != ntpCheck) { delete ntpCheck; }
    if ( nullptr != openweatherCheck) { delete openweatherCheck; }
    if ( nullptr != wifiCheck) { delete wifiCheck; }
    if ( nullptr != bleCheck) { delete bleCheck; }
    if ( nullptr != ntpBLECheck ) { delete ntpBLECheck; }
}

SettingsApplication::SettingsApplication() {

    wifiCheck=new SwitchWidget(115,0,[&,this](IGNORE_PARAM){
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
        if ( false == wifiCheck->switchEnabled ) {
            ntpCheck->switchEnabled=false;
            openweatherCheck->switchEnabled=false;
        }
        ntpCheck->InternalRedraw();
        openweatherCheck->InternalRedraw();
    });
    wifiCheck->switchEnabled=(bool)NVS.getInt("WifiEnabled");
    wifiCheck->InternalRedraw();

    ntpCheck=new SwitchWidget(10,35);
    ntpCheck->switchEnabled= (wifiCheck->switchEnabled?NVS.getInt("NTPEnabled"):false);
    ntpCheck->enabled=wifiCheck->switchEnabled;
    ntpCheck->InternalRedraw();

    openweatherCheck=new SwitchWidget(10,80);
    openweatherCheck->switchEnabled=(wifiCheck->switchEnabled?NVS.getInt("OWeatherEnabled"):false);
    openweatherCheck->enabled=wifiCheck->switchEnabled;
    if ( 0 == strlen(openWeatherMapApiKey)) {
        openweatherCheck->SetEnabled(false);
    }
    openweatherCheck->InternalRedraw();

    bleCheck=new SwitchWidget(115,120,[&,this](IGNORE_PARAM){
        ntpBLECheck->SetEnabled(bleCheck->switchEnabled);
        if ( false == bleCheck->switchEnabled ) {
            ntpBLECheck->switchEnabled=false;
        }
        ntpBLECheck->InternalRedraw();
    });
    bleCheck->switchEnabled=NVS.getInt("BLEEnabled");
    bleCheck->InternalRedraw();

    ntpBLECheck=new SwitchWidget(20,160);
    ntpBLECheck->switchEnabled= (bleCheck->switchEnabled?NVS.getInt("NTPBLEEnabled"):false);
    ntpBLECheck->enabled=bleCheck->switchEnabled;
    ntpBLECheck->InternalRedraw();

    Tick();
}

bool SettingsApplication::Tick() {
    
    TemplateApplication::Tick();
    //TemplateApplication::btnBack->Interact(touched,touchX, touchY);
    //btnHelp->Interact(touched,touchX, touchY);
    wifiCheck->Interact(touched,touchX, touchY);    
    ntpCheck->Interact(touched,touchX, touchY);
    openweatherCheck->Interact(touched,touchX, touchY);

    bleCheck->Interact(touched,touchX, touchY);
    ntpBLECheck->Interact(touched,touchX, touchY);


    if (millis() > nextRefresh ) {
        canvas->fillSprite(ThCol(background));
        TemplateApplication::btnBack->DrawTo(canvas);

        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextDatum(CL_DATUM);

        ntpCheck->DrawTo(canvas);
        uint16_t textColor = ThCol(text);
        if ( false == ntpCheck->switchEnabled ) { textColor = ThCol(text_alt); }
        canvas->setTextColor(textColor);
        canvas->drawString("NTP",ntpCheck->GetX()+ntpCheck->GetW()+15,ntpCheck->GetY()+(ntpCheck->GetH()/2));

        openweatherCheck->DrawTo(canvas);
        textColor = ThCol(text);
        if ( false == openweatherCheck->switchEnabled ) { textColor = ThCol(text_alt); }
        canvas->setTextColor(textColor);
        canvas->drawString("OpenWeather",openweatherCheck->GetX()+openweatherCheck->GetW()+15,openweatherCheck->GetY()+(openweatherCheck->GetH()/2));

        wifiCheck->DrawTo(canvas);
        textColor = ThCol(text);
        if ( false == wifiCheck->switchEnabled ) { textColor = ThCol(text_alt); }
        canvas->drawXBitmap(wifiCheck->GetX()+wifiCheck->GetW()+15,wifiCheck->GetY()+(wifiCheck->GetH()/2)-(img_wifi_32_height/2),img_wifi_32_bits,img_wifi_32_width,img_wifi_32_height,textColor);

        bleCheck->DrawTo(canvas);
        textColor = ThCol(text);
        if ( false == bleCheck->switchEnabled ) { textColor = ThCol(text_alt); }
        canvas->drawXBitmap(bleCheck->GetX()+bleCheck->GetW()+15,bleCheck->GetY()+(bleCheck->GetH()/2)-(img_bluetooth_32_height/2),img_bluetooth_32_bits,img_bluetooth_32_width,img_bluetooth_32_height,textColor);

        ntpBLECheck->DrawTo(canvas);
        textColor = ThCol(text);
        if ( false == ntpBLECheck->switchEnabled ) { textColor = ThCol(text_alt); }
        canvas->setTextColor(textColor);
        canvas->drawString("NTP-BLE",ntpBLECheck->GetX()+ntpBLECheck->GetW()+15,ntpBLECheck->GetY()+(ntpBLECheck->GetH()/2));

        nextRefresh=millis()+(1000/6);
        return true;
    }
    return false;
}