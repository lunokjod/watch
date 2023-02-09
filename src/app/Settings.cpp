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
#include "Watchface2.hpp"

SettingsApplication::~SettingsApplication() {
    // on destroy
    // set the switch values
    NVS.setInt("BLEEnabled",bleCheck->switchEnabled,false);
    NVS.setInt("WifiEnabled",wifiCheck->switchEnabled,false);

    NVS.setInt("NTPEnabled",ntpCheck->switchEnabled,false);
    if ( nullptr != Watchface2Application::ntpTask ) { 
        Watchface2Application::ntpTask->enabled = ntpCheck->switchEnabled;
    }
    NVS.setInt("OWeatherEnabled",openweatherCheck->switchEnabled,false);
    if ( nullptr != Watchface2Application::weatherTask ) {
        Watchface2Application::weatherTask->enabled = openweatherCheck->switchEnabled;
    }
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
        /*
        delay(30);
        //@TODO the value is inverted due callback is called BEFORE change of bool
        bool inverVal = (!wifiCheck->switchEnabled);
        ntpCheck->SetEnabled(inverVal);
        openweatherCheck->SetEnabled(inverVal);
        */
        ntpCheck->SetEnabled(wifiCheck->switchEnabled);
        if ( 0 == strlen(openWeatherMapApiKey)) {
            openweatherCheck->SetEnabled(false);
        } else {
            openweatherCheck->SetEnabled(wifiCheck->switchEnabled);
        }
    });
#ifdef LUNOKIOT_WIFI_ENABLED
    wifiCheck->switchEnabled=NVS.getInt("WifiEnabled");
#else
    wifiCheck->switchEnabled=false;
    wifiCheck->enabled=false;
#endif
    wifiCheck->InternalRedraw();

    bleCheck=new SwitchWidget(80,160);
#ifdef LUNOKIOT_BLE_ENABLED
    bleCheck->switchEnabled=NVS.getInt("BLEEnabled");
#else
    bleCheck->enabled=false;
    bleCheck->switchEnabled=false;
#endif
    bleCheck->InternalRedraw();

    ntpCheck=new SwitchWidget(10,10);
    ntpCheck->switchEnabled=NVS.getInt("NTPEnabled");
    ntpCheck->enabled=wifiCheck->switchEnabled;
    ntpCheck->InternalRedraw();

    openweatherCheck=new SwitchWidget(10,60);
    openweatherCheck->switchEnabled=NVS.getInt("OWeatherEnabled");
    openweatherCheck->enabled=wifiCheck->switchEnabled;
    openweatherCheck->InternalRedraw();
    if ( 0 == strlen(openWeatherMapApiKey)) {
        openweatherCheck->SetEnabled(false);
    }

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