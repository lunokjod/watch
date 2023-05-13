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

#include "Fade.hpp"
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "../../lunokiot_config.hpp"
#include "../UI.hpp"
#include "../../app/LogView.hpp"

extern SemaphoreHandle_t UISemaphore;

void FadeTransition(TFT_eSprite * curentView, TFT_eSprite * nextView) {
    // draw by fade
    //for(uint16_t balance=0;balance<256;balance+=(256/4)) {
        //TickType_t nextStep = xTaskGetTickCount();     // get the current ticks
        for(int32_t y=0;y<curentView->height();y+=2) {
            for(int32_t x=0;x<curentView->width();x+=2) {
                //uint16_t oldColor = curentView->readPixel(x,y);
                uint16_t newColor = nextView->readPixel(x,y);
                //uint16_t finalColor = tft->alphaBlend(balance,newColor,oldColor);
                //tft->drawPixel(x,y,finalColor);
                tft->drawPixel(x,y,newColor);
            }
        }
        //BaseType_t delayed = xTaskDelayUntil( &nextStep, (50 / portTICK_PERIOD_MS) ); // wait a ittle bit (freeRTOS must breath)
        //taskYIELD();
    //}
    // push full image
    nextView->pushSprite(0,0);
}
