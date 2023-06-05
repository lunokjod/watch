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
extern TTGOClass *ttgo; // ttgo library shit ;)

#include "../lunokiot_config.hpp"

#include "Shutdown.hpp"
#include "../system/SystemEvents.hpp"

#include "resources.hpp"
#include "../system/Network/BLE.hpp"
#include "driver/uart.h"
#include "../app/LogView.hpp"
//@TODO https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-reference/system/system.html?highlight=memory#_CPPv429esp_register_shutdown_handler18shutdown_handler_t

extern uint32_t systemStatsRebootCounter;

ShutdownApplication::ShutdownApplication(bool restart, bool savedata): restart(restart), savedata(savedata) {

#ifdef LILYGO_WATCH_2020_V3
    ttgo->shake();
#endif
    //nextRedraw=0;
    //timeFromBegin=millis();
    //Serial.flush();

    this->canvas->setSwapBytes(true);
    this->canvas->pushImage(0,0,img_poweroff_fullscreen.width,img_poweroff_fullscreen.height, (uint16_t *)img_poweroff_fullscreen.pixel_data);
    this->canvas->setSwapBytes(false);

    char stepsString[128] = { 0 };
    //this->canvas->fillSprite(TFT_DARKGREY);
    this->canvas->setTextColor(TFT_WHITE);

    this->canvas->setTextSize(2);
    this->canvas->setTextWrap(false,false);
    this->canvas->setFreeFont(&FreeMonoBold24pt7b);
    this->canvas->setTextDatum(TL_DATUM);
    sprintf(stepsString,"See");
    this->canvas->setTextColor(TFT_BLACK);
    this->canvas->drawString(stepsString, 6, 6);
    this->canvas->setTextColor(TFT_WHITE);
    this->canvas->drawString(stepsString, 5, 5);

    int32_t posX = TFT_WIDTH/2;
    int32_t posY = TFT_HEIGHT-8;

    this->canvas->setTextSize(1);
    this->canvas->setTextDatum(BC_DATUM);
    this->canvas->setFreeFont(&FreeMonoBold18pt7b);
    sprintf(stepsString,"you soon :)");
    this->canvas->setTextColor(TFT_BLACK);
    this->canvas->drawString(stepsString, posX+1, posY+1);
    this->canvas->setTextColor(TFT_WHITE);
    this->canvas->drawString(stepsString, posX, posY);
}

bool ShutdownApplication::Tick() {
    // do a lightfade
    bright-=8;
    if ( bright > -1 ) { ttgo->setBrightness(bright); }
    else {
        // screen is off
        ttgo->setBrightness(0);
        tft->fillScreen(TFT_BLACK);
        // save system data and unmount filesystems gracefully
        if ( savedata ) { SaveDataBeforeShutdown(); }
        if ( restart ) {
            lEvLog("ESP32: System restart NOW!\n");
            uart_wait_tx_idle_polling(UART_NUM_0);
            ESP.restart();
        } else {
            lEvLog("ESP32: System shutdown NOW!\n");
            uart_wait_tx_idle_polling(UART_NUM_0);
            ttgo->shutdown();
        }
    }
    /*
    if ( nullptr == this->canvas ) { return false; }
    unsigned long milisFromBegin = millis()-timeFromBegin;
    if ( milisFromBegin > 2200 ) {
        systemStatsRebootCounter++;
        ttgo->setBrightness(0);
        tft->fillScreen(TFT_BLACK);
        if ( restart ) {
            lEvLog("ESP32: System restart NOW!\n");
            uart_wait_tx_idle_polling(UART_NUM_0);
            ESP.restart();
        } else {
            lEvLog("ESP32: System shutdown NOW!\n");
            uart_wait_tx_idle_polling(UART_NUM_0);
            ttgo->shutdown();
        }
    }*/
    return true;
}
