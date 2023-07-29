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

#include "Flip.hpp"
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "../../lunokiot_config.hpp"
#include "../UI.hpp"
#include "../../app/LogView.hpp"

void FlipTransition(TFT_eSprite * curentView, TFT_eSprite * nextView) {
    // scale screen buffer
    const int images=4;
    TFT_eSprite *scaledImgs[images];
    scaledImgs[0] = ShearSprite(nextView,{1,-1,0,1});
    scaledImgs[1] = ShearSprite(nextView,{1,-0.75,0,1});
    scaledImgs[2] = ShearSprite(nextView,{1,-0.5,0,1});
    scaledImgs[3] = ShearSprite(nextView,{1,-0.25,0,1});
    for(int scale=0;scale<images;scale++) {
        TickType_t nextStep = xTaskGetTickCount();     // get the current ticks
        TFT_eSprite *scaledImg = scaledImgs[scale];
        if ( nullptr != scaledImg ) {
            scaledImg->pushSprite((TFT_WIDTH-scaledImg->width())/2,(TFT_HEIGHT-scaledImg->height())/2,TFT_PINK);
        }
        BaseType_t delayed = xTaskDelayUntil( &nextStep, (60 / portTICK_PERIOD_MS) ); // wait a ittle bit (freeRTOS must breath)
    }
    // push full image
    nextView->pushSprite(0,0);
    // cleanup
    for(int i=0;i<images;i++) {
        if ( nullptr == scaledImgs[i] ) { continue; }
        scaledImgs[i]->deleteSprite();
        delete scaledImgs[i];
    }
}
