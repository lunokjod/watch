#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Settings.hpp"
#include "../static/img_back_32.xbm"
#include "Watchface.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../static/img_wifi_32.xbm"
#include "../static/img_bluetooth_32.xbm"
#include <ArduinoNvs.h>
#include "Watchface.hpp"

#include "../system/Network.hpp"

//extern NetworkTaskDescriptor * WatchfaceApplication::ntpTask;
//extern NetworkTaskDescriptor * WatchfaceApplication::weatherTask;

SettingsApplication::~SettingsApplication() {
    if ( nullptr != btnBack ) { delete btnBack; }
    if ( nullptr != ntpCheck ) { delete ntpCheck; }
    if ( nullptr != openweatherCheck ) { delete openweatherCheck; }
    if ( nullptr != wifiCheck ) { delete wifiCheck; }
    if ( nullptr != bleCheck ) { delete bleCheck; }
}

SettingsApplication::SettingsApplication() {
    wifiCheck=new SwitchWidget(80,110, [&,this]() {
        openweatherCheck->enabled=wifiCheck->switchEnabled;
        ntpCheck->enabled=wifiCheck->switchEnabled;
        NVS.setInt("WifiEnabled",wifiCheck->switchEnabled,false);
    }, canvas->color24to16(0x555f68));
    wifiCheck->switchEnabled=NVS.getInt("WifiEnabled");
#ifndef LUNOKIOT_WIFI_ENABLED
    wifiCheck->switchEnabled=false;
    wifiCheck->enabled=false;
#endif
    bleCheck=new SwitchWidget(80,160, [&,this]() {
        NVS.setInt("BLEEnabled",bleCheck->switchEnabled,false);
    }, canvas->color24to16(0x555f68));
    bleCheck->switchEnabled=NVS.getInt("BLEEnabled");
#ifndef LUNOKIOT_BLE_ENABLED
    bleCheck->switchEnabled=false;
    bleCheck->enabled=false;
#endif

    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,ttgo->tft->color24to16(0x353e45),false);

    ntpCheck=new SwitchWidget(10,10, [&,this]() {
        NVS.setInt("NTPEnabled",ntpCheck->switchEnabled,false);
        if ( nullptr != WatchfaceApplication::ntpTask ) {
            WatchfaceApplication::ntpTask->enabled = ntpCheck->switchEnabled;
        }
    }, canvas->color24to16(0x555f68));
    ntpCheck->switchEnabled=NVS.getInt("NTPEnabled");
    ntpCheck->enabled=wifiCheck->switchEnabled;

    openweatherCheck=new SwitchWidget(10,60, [&,this]() {
        NVS.setInt("OWeatherEnabled",openweatherCheck->switchEnabled,false);
        if ( nullptr != WatchfaceApplication::weatherTask ) {
            WatchfaceApplication::weatherTask->enabled = openweatherCheck->switchEnabled;
        }
    }, canvas->color24to16(0x555f68));
    openweatherCheck->switchEnabled=NVS.getInt("OWeatherEnabled");
    openweatherCheck->enabled=wifiCheck->switchEnabled;
}

bool SettingsApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY);
    ntpCheck->Interact(touched,touchX, touchY);
    openweatherCheck->Interact(touched,touchX, touchY);
    wifiCheck->Interact(touched,touchX, touchY);
    bleCheck->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        btnBack->DrawTo(canvas);
        ntpCheck->DrawTo(canvas);
        openweatherCheck->DrawTo(canvas);
        wifiCheck->DrawTo(canvas);
        bleCheck->DrawTo(canvas);
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextDatum(TL_DATUM);
        canvas->setTextColor(TFT_WHITE);
        canvas->drawString("NTP Sync",95,35);
        canvas->drawString("OpenWeather",95,85);

        canvas->drawXBitmap(160,130,img_wifi_32_bits,img_wifi_32_width,img_wifi_32_height,TFT_WHITE);
        canvas->drawXBitmap(165,176,img_bluetooth_32_bits,img_bluetooth_32_width,img_bluetooth_32_height,TFT_WHITE);

        nextRedraw=millis()+(1000/10);
        return true;
    }
    return false;
}