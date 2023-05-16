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

#include "Stripes.hpp"
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "../../lunokiot_config.hpp"
#include "../UI.hpp"
#include "../../app/LogView.hpp"
#include "lunokIoT.hpp"

void StripeTransition(TFT_eSprite * curentView, TFT_eSprite * nextView) {
    const int16_t numStripes = 8;
    const int16_t sizeStripe = nextView->height()/numStripes;
    TFT_eSprite * stripes[numStripes];
    // build sprites
    for (int16_t curr=0;curr<numStripes;curr++) {
        stripes[curr] = GetSpriteRect(nextView,0,sizeStripe*curr,sizeStripe,nextView->width());
    }

    // up do down 
    for (int16_t curr=0;curr<numStripes;curr+=2) {
        TickType_t nextStep = xTaskGetTickCount();     // get the current ticks
        stripes[curr]->pushSprite(0,sizeStripe*curr);
        BaseType_t delayed = xTaskDelayUntil( &nextStep, (30 / portTICK_PERIOD_MS) ); // wait a ittle bit (freeRTOS must breath)
    }
    TickType_t nextStep = xTaskGetTickCount();     // get the current ticks
    // pause
    BaseType_t delayed = xTaskDelayUntil( &nextStep, (60 / portTICK_PERIOD_MS) ); // wait a ittle bit (freeRTOS must breath)
    // down to up
    for (int16_t curr=numStripes-1;curr>=0;curr-=2) {
        TickType_t nextStep = xTaskGetTickCount();     // get the current ticks
        stripes[curr]->pushSprite(0,sizeStripe*curr);
        BaseType_t delayed = xTaskDelayUntil( &nextStep, (30 / portTICK_PERIOD_MS) ); // wait a ittle bit (freeRTOS must breath)
    }

    // push full image
    //nextView->pushSprite(0,0);

    // cleanup
    for (int16_t curr=0;curr<numStripes;curr++) {
        if ( nullptr == stripes[curr] ) { continue; }
        stripes[curr]->deleteSprite();
        delete stripes[curr];
    }

}
