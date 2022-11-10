#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Settings.hpp"
#include "../static/img_back_32.xbm"
#include "Watchface.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../static/img_wifi_32.xbm"
#include "../static/img_bluetooth_32.xbm"
#include "../static/img_help_32.xbm"

#include <ArduinoNvs.h>
#include "Watchface.hpp"

#include "../system/Network.hpp"
extern TFT_eSprite *overlay;

//extern NetworkTaskDescriptor * WatchfaceApplication::ntpTask;
//extern NetworkTaskDescriptor * WatchfaceApplication::weatherTask;

SettingsApplication::~SettingsApplication() {
    if ( nullptr != btnHelp ) { delete btnHelp; }
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

    btnHelp=new ButtonImageXBMWidget(TFT_WIDTH-69,5,64,64,[&,this](){
        showOverlay = (!showOverlay);
    },img_help_32_bits,img_help_32_height,img_help_32_width,TFT_WHITE,ttgo->tft->color24to16(0x353e45),false);


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


    // build overlay as help screen
    overlay->fillSprite(TFT_BLACK);
    const uint8_t margin = 32;
    const uint8_t radius = 50;

    for(int32_t y=0;y<TFT_HEIGHT;y+=2) {
        for(int32_t x=0;x<TFT_WIDTH;x+=2) {
            overlay->drawPixel(x,y,TFT_TRANSPARENT);
            //overlay->drawPixel(x+1,y,TFT_TRANSPARENT);
        }
    }

    overlay->fillCircle(ntpCheck->x+margin,ntpCheck->y+margin,radius+2,TFT_WHITE);
    overlay->fillCircle(openweatherCheck->x+margin,openweatherCheck->y+margin,radius+2,TFT_WHITE);
    overlay->fillCircle(wifiCheck->x+margin,wifiCheck->y+margin,radius+2,TFT_WHITE);
    overlay->fillCircle(bleCheck->x+margin,bleCheck->y+margin,radius+2,TFT_WHITE);
    overlay->fillCircle(btnHelp->x+margin,btnHelp->y+margin,radius+2,TFT_WHITE);
    overlay->fillCircle(btnBack->x+margin,btnBack->y+margin,radius+2,TFT_WHITE);

    overlay->fillCircle(ntpCheck->x+margin,ntpCheck->y+margin,radius,TFT_TRANSPARENT);
    overlay->fillCircle(openweatherCheck->x+margin,openweatherCheck->y+margin,radius,TFT_TRANSPARENT);
    overlay->fillCircle(wifiCheck->x+margin,wifiCheck->y+margin,radius,TFT_TRANSPARENT);
    overlay->fillCircle(bleCheck->x+margin,bleCheck->y+margin,radius,TFT_TRANSPARENT);
    overlay->fillCircle(btnHelp->x+margin,btnHelp->y+margin,radius,TFT_TRANSPARENT);
    overlay->fillCircle(btnBack->x+margin,btnBack->y+margin,radius,TFT_TRANSPARENT);

    canvas->setTextFont(1);
    canvas->setTextSize(2);
    canvas->setTextDatum(CL_DATUM);
    canvas->setTextColor(TFT_WHITE);
    canvas->drawString("When WIFi up, try to sync via NTP",ntpCheck->x+55,ntpCheck->y);

    Tick();
}

bool SettingsApplication::Tick() {

    bool interacted=false;
    bool ret;
    ret = btnHelp->Interact(touched,touchX, touchY);
    if ( ret ) { interacted = true; }
    ret = btnBack->Interact(touched,touchX, touchY);
    if ( ret ) { interacted = true; }
    ret = ntpCheck->Interact(touched,touchX, touchY);
    if ( ret ) { interacted = true; }
    ret = openweatherCheck->Interact(touched,touchX, touchY);
    if ( ret ) { interacted = true; }
    ret = wifiCheck->Interact(touched,touchX, touchY);
    if ( ret ) { interacted = true; }
    ret = bleCheck->Interact(touched,touchX, touchY);
    if ( ret ) { interacted = true; }

    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        btnHelp->DrawTo(canvas);
        btnBack->DrawTo(canvas);
        ntpCheck->DrawTo(canvas);
        openweatherCheck->DrawTo(canvas);
        wifiCheck->DrawTo(canvas);
        bleCheck->DrawTo(canvas);
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextDatum(TL_DATUM);
        canvas->setTextColor(TFT_WHITE);
        canvas->drawString("NTP",95,35);
        canvas->drawString("OpenWeather",95,85);

        canvas->drawXBitmap(160,130,img_wifi_32_bits,img_wifi_32_width,img_wifi_32_height,TFT_WHITE);
        canvas->drawXBitmap(165,176,img_bluetooth_32_bits,img_bluetooth_32_width,img_bluetooth_32_height,TFT_WHITE);

        //if ( ( false == interacted ) && ( touched ) ) { showOverlay=(!showOverlay); }
        if ( showOverlay ) { overlay->pushRotated(canvas,0,TFT_TRANSPARENT); }

        nextRedraw=millis()+(1000/10);
        return true;
    }
    return false;
}