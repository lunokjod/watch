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

#include "Rotation.hpp"

#include "../UI/UI.hpp"
#include "../static/img_happy_48.xbm" // center smiley to see the orientation
#include <ArduinoNvs.h> // persistent values
#include "../system/SystemEvents.hpp"
#include <TFT_eSPI.h>

// better save only on exit to save flash writes
RotationApplication::~RotationApplication() {
    NVS.setInt("ScreenRot",tft->getRotation(),false);
    delete up;
    delete right;
    delete down;
    delete left;
}

RotationApplication::RotationApplication() {
    down = new ButtonWidget(60,(TFT_HEIGHT-10)-60,60,TFT_WIDTH-120,[](void *unused) { tft->setRotation(0); });
    left = new ButtonWidget(10,60,TFT_HEIGHT-120,60,[](void *unused) { tft->setRotation(1); });
    up = new ButtonWidget(60,10,60,TFT_WIDTH-120,[](void *unused) { tft->setRotation(2); });
    right = new ButtonWidget((TFT_WIDTH-10)-60,60,TFT_HEIGHT-120,60,[](void *unused) { tft->setRotation(3); });
    Tick();
}

bool RotationApplication::Tick() {
    up->Interact(touched,touchX,touchY);
    right->Interact(touched,touchX,touchY);
    down->Interact(touched,touchX,touchY);
    left->Interact(touched,touchX,touchY);

    if (millis() > nextRedraw ) {
        // draw code here
        canvas->fillSprite(ThCol(background));

        canvas->drawXBitmap((TFT_WIDTH/2)-(img_happy_48_width/2),
                (TFT_HEIGHT/2)-(img_happy_48_height/2),
                img_happy_48_bits,img_happy_48_width,img_happy_48_height,TFT_WHITE);

        uint8_t rot = tft->getRotation();
        up->DrawTo(canvas);
        right->DrawTo(canvas);
        down->DrawTo(canvas);
        left->DrawTo(canvas);
        
        nextRedraw=millis()+(1000/3); // tune your required refresh
        return true;
    }
    return false;
}
