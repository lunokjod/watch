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
#include "../widgets/CanvasWidget.hpp"

void UpDownTransition(TFT_eSprite * curentView, TFT_eSprite * nextView) {
        //CanvasWidget * leftPiece = new CanvasWidget(curentView->height(),curentView->width()/2);
        //CanvasWidget * rightPiece = new CanvasWidget(curentView->height(),curentView->width()/2);
        TFT_eSprite * leftPiece = GetSpriteRect(nextView,0,0,curentView->height(),curentView->width()/2);
        TFT_eSprite * rightPiece = GetSpriteRect(nextView,curentView->width()/2,0,curentView->height(),curentView->width()/2);
        /*
        leftPiece->canvas->setPivot(0,0);
        nextView->setPivot(0,0);
        nextView->pushRotated(leftPiece->canvas,0,0);
        rightPiece->canvas->setPivot(0,0);
        nextView->setPivot(curentView->width()/2,0);
        nextView->pushRotated(rightPiece->canvas,0);
        */
       int32_t y2=curentView->height()*-1;
        for(int32_t y=curentView->height();y>0;y-=16) {
            TickType_t nextStep = xTaskGetTickCount();     // get the current ticks
            leftPiece->pushSprite(0,y);
            rightPiece->pushSprite(curentView->width()/2,y2);
            BaseType_t delayed = xTaskDelayUntil( &nextStep, (30 / portTICK_PERIOD_MS) ); // wait a ittle bit (freeRTOS must breath)
            y2+=16;
        }
        //taskYIELD();
    //}
    // push full image
    nextView->pushSprite(0,0);
    leftPiece->deleteSprite();
    delete leftPiece;
    rightPiece->deleteSprite();
    delete rightPiece;
    //delete leftPiece;
    //delete rightPiece;
}
