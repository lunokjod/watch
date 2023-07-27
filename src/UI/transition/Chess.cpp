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

#include "Displace.hpp"
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "../../lunokiot_config.hpp"
#include "../UI.hpp"
#include "../../app/LogView.hpp"
#include "lunokIoT.hpp"

void ChessTransition(TFT_eSprite * curentView, TFT_eSprite * nextView) {
        TFT_eSprite * leftTopPiece = GetSpriteRect(nextView,0,0,curentView->height()/2,curentView->width()/2);
        TFT_eSprite * rightTopPiece = GetSpriteRect(nextView,curentView->width()/2,0,curentView->height()/2,curentView->width()/2);
        TFT_eSprite * leftDownPiece = GetSpriteRect(nextView,0,curentView->height()/2,curentView->height()/2,curentView->width()/2);
        TFT_eSprite * rightDownPiece = GetSpriteRect(nextView,curentView->width()/2,curentView->height()/2,curentView->height()/2,curentView->width()/2);

        int32_t x=(curentView->width()/2)*-1;
        int32_t x2=curentView->width();
        int32_t y=(curentView->height()/2)*-1;
        int32_t y2=curentView->height();
        int8_t steps=15;
        while(true) {
            TickType_t nextStep = xTaskGetTickCount();     // get the current ticks
            leftTopPiece->pushSprite(0,y);
            rightTopPiece->pushSprite(x2,0);
            leftDownPiece->pushSprite(x,y2);
            rightDownPiece->pushSprite(curentView->width()/2,y2);

            BaseType_t delayed = xTaskDelayUntil( &nextStep, (30 / portTICK_PERIOD_MS) ); // wait a ittle bit (freeRTOS must breath)
            //BaseType_t delayed = xTaskDelayUntil( &nextStep, (80 / portTICK_PERIOD_MS) );
            x+=8;
            x2-=8;
            y+=8;
            y2-=8;
            steps--;
            if ( steps < 0 ) { break; }
        }
        //taskYIELD();
    //}
    // push full image
    nextView->pushSprite(0,0);
    leftTopPiece->deleteSprite();
    delete leftTopPiece;
    rightTopPiece->deleteSprite();
    delete rightTopPiece;
    leftDownPiece->deleteSprite();
    delete leftDownPiece;
    rightDownPiece->deleteSprite();
    delete rightDownPiece;
}
