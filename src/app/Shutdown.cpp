#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../lunokiot_config.hpp"

#include "Shutdown.hpp"

#include "../static/img_poweroff_fullscreen.c"

ShutdownApplication::ShutdownApplication() {
    LunokIoTApplication();
    ttgo->shake();

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
    unsigned long secsFromBegin = ((millis()-timeFromBegin)/1000);

    if ( secsFromBegin == 3 ) {
        Serial.println("System shutdown!");
        Serial.flush();
        delay(10);
        ttgo->shutdown();
    }
    if ( 1 == secsFromBegin ) {
        ttgo->shake();
    }
    //if ( nextRedraw < millis() ) {

    //    nextRedraw = millis()+1000;
    //}
    return true;
}
