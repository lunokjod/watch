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
//#include <LilyGoWatch.h>
#include "Playground6.hpp"
#include "../UI/UI.hpp"
#include <list>
#include <libraries/TFT_eSPI/TFT_eSPI.h>
extern std::list<TFT_eSprite *>ScreenShots;

PlaygroundApplication6::~PlaygroundApplication6() {
    if ( nullptr != buffer ) { delete buffer; }
    while(Thumbnails.size() > 0) {
        TFT_eSprite * current = Thumbnails.back();
        Thumbnails.pop_back();
        current->deleteSprite();
        delete current;
    }
}
PlaygroundApplication6::PlaygroundApplication6() {
    this->buffer = new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    // generate thumbnails
    for (auto const& capture : ScreenShots) {
        Serial.printf("Playground6: ScreenShoot %p\n",capture);
        TFT_eSprite * reduced = ScaleSprite(capture,thumnailScale);
        TFT_eSprite *buffer = new TFT_eSprite(tft);
        buffer->createSprite(reduced->width(),reduced->height());
        reduced->pushRotated(buffer,0);
        Thumbnails.push_back(buffer);
        reduced->deleteSprite();
        delete reduced;
    }
}
bool PlaygroundApplication6::Tick() {
    if (millis() > nextRedraw ) {
        buffer->canvas->fillSprite(TFT_BLACK);
        int16_t displace=0;
        int16_t x=0;
        int16_t y=0;
        int16_t imageWidth = TFT_WIDTH*thumnailScale;
        int16_t maxImagesWidth =TFT_WIDTH/imageWidth;
        int16_t currImageNum = 0;
        for (auto const& capture : Thumbnails) {
            buffer->canvas->setPivot(x,y);
            capture->setPivot(0,0);
            capture->pushRotated(buffer->canvas,0);
            x+=imageWidth;
            currImageNum++;
            if (currImageNum >= maxImagesWidth ) {
                x = 0;
                y+=TFT_HEIGHT*thumnailScale;
                currImageNum=0;
                //if ( y > 238 ) { break; }
            }
        }
        nextRedraw=millis()+(1000/1); // tune your required refresh
        buffer->DrawTo(canvas,0,0);
        return true;
    }
    return false;
}