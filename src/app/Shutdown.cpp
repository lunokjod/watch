#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../lunokiot_config.hpp"

#include "Shutdown.hpp"
#include "../system/SystemEvents.hpp"

#include "../static/img_poweroff_fullscreen.c"

ShutdownApplication::ShutdownApplication(bool restart, bool savedata): restart(restart), savedata(savedata) {
    if ( savedata ) {
        SaveDataBeforeShutdown();
    }
#ifdef LILYGO_WATCH_2020_V3
    ttgo->shake();
#endif
    nextRedraw=0;
    timeFromBegin=millis();
    Serial.flush();

    this->GetCanvas()->setSwapBytes(true);
    this->GetCanvas()->pushImage(0,0,img_poweroff_fullscreen.width,img_poweroff_fullscreen.height, (uint16_t *)img_poweroff_fullscreen.pixel_data);
    this->GetCanvas()->setSwapBytes(false);

    char stepsString[128] = { 0 };
    //this->GetCanvas()->fillSprite(TFT_DARKGREY);
    this->GetCanvas()->setTextColor(TFT_WHITE);

    this->GetCanvas()->setTextSize(2);
    this->GetCanvas()->setTextWrap(false,false);
    this->GetCanvas()->setFreeFont(&FreeMonoBold24pt7b);
    this->GetCanvas()->setTextDatum(TL_DATUM);
    sprintf(stepsString,"See");
    this->GetCanvas()->setTextColor(TFT_BLACK);
    this->GetCanvas()->drawString(stepsString, 6, 6);
    this->GetCanvas()->setTextColor(TFT_WHITE);
    this->GetCanvas()->drawString(stepsString, 5, 5);

    int32_t posX = TFT_WIDTH/2;
    int32_t posY = TFT_HEIGHT-8;

    this->GetCanvas()->setTextSize(1);
    this->GetCanvas()->setTextDatum(BC_DATUM);
    this->GetCanvas()->setFreeFont(&FreeMonoBold18pt7b);
    sprintf(stepsString,"you soon :)");
    this->GetCanvas()->setTextColor(TFT_BLACK);
    this->GetCanvas()->drawString(stepsString, posX+1, posY+1);
    this->GetCanvas()->setTextColor(TFT_WHITE);
    this->GetCanvas()->drawString(stepsString, posX, posY);

}

bool ShutdownApplication::Tick() {
    if ( nullptr == this->GetCanvas() ) { return false; }
    unsigned long milisFromBegin = millis()-timeFromBegin;
    bright-=8;
    if ( bright > -1 ) { ttgo->setBrightness(bright); }
    if ( milisFromBegin > 2200 ) {
        ttgo->setBrightness(0);
        ttgo->tft->fillScreen(TFT_BLACK);
        if ( restart ) {
            Serial.println("System restart NOW!");
            Serial.flush();
            delay(10);
            ESP.restart();
        } else {
            Serial.println("System shutdown NOW!");
            Serial.flush();
            delay(10);
            ttgo->shutdown();
        }
    }
    return true;
}
