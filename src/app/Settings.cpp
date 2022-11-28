#include "Settings.hpp"

#include <Arduino.h>
#include <ArduinoNvs.h>

#include <libraries/TFT_eSPI/TFT_eSPI.h>
extern TFT_eSPI *tft;
extern TFT_eSprite *overlay;

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
    delete btnHelp;
    delete ntpCheck;
    delete openweatherCheck;
    delete wifiCheck;
    delete bleCheck;
}

SettingsApplication::SettingsApplication() {

    wifiCheck=new SwitchWidget(80,110,[&,this](){
        delay(30);
        //@TODO the value is inverted due callback is called BEFORE change of bool
        bool inverVal = (!wifiCheck->switchEnabled);
        ntpCheck->SetEnabled(inverVal);
        openweatherCheck->SetEnabled(inverVal);
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
    
    btnHelp=new ButtonImageXBMWidget(TFT_WIDTH-69,5,64,64,[&,this](){
        showOverlay = (!showOverlay);
    },img_help_32_bits,img_help_32_height,img_help_32_width,ThCol(text),ThCol(button),false);

    ntpCheck=new SwitchWidget(10,10);
    ntpCheck->switchEnabled=NVS.getInt("NTPEnabled");
    ntpCheck->enabled=wifiCheck->switchEnabled;
    ntpCheck->InternalRedraw();

    openweatherCheck=new SwitchWidget(10,60);
    openweatherCheck->switchEnabled=NVS.getInt("OWeatherEnabled");
    openweatherCheck->enabled=wifiCheck->switchEnabled;
    openweatherCheck->InternalRedraw();

    // build overlay as help screen
    overlay->fillSprite(TFT_BLACK);
    const uint8_t margin = 32;
    const uint8_t radius = 50;
    // translucent
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
    /*
    overlay->setTextFont(1);
    overlay->setTextSize(2);
    overlay->setTextDatum(CL_DATUM);
    overlay->setTextColor(TFT_WHITE);
    overlay->drawString("When WIFi up, try to sync via NTP",ntpCheck->x+55,ntpCheck->y);
    */
    Tick();
}

bool SettingsApplication::Tick() {
    
    TemplateApplication::Tick();

    btnHelp->Interact(touched,touchX, touchY);
    wifiCheck->Interact(touched,touchX, touchY);
    bleCheck->Interact(touched,touchX, touchY);

    ntpCheck->Interact(touched,touchX, touchY);
    openweatherCheck->Interact(touched,touchX, touchY);


    if (millis() > nextRefresh ) {
        canvas->fillSprite(ThCol(background));
        btnHelp->DrawTo(canvas);
        btnBack->DrawTo(canvas);
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

        if ( showOverlay ) { overlay->pushRotated(canvas,0,TFT_TRANSPARENT); }

        nextRefresh=millis()+(1000/3);
        return true;
    }
    return false;
}