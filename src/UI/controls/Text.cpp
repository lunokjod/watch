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

#include "Text.hpp"
#include "../../app/LogView.hpp"
#include "../UI.hpp"

using namespace LuI;

Text::~Text() {
    lLog("Text %p destroyed!\n",this);
    if ( nullptr != imageTextCanvas ) {
        imageTextCanvas->deleteSprite();
        delete imageTextCanvas;
        imageTextCanvas=nullptr;
    }
}

Text::Text(IN char * what, IN uint16_t color,IN bool swap, IN uint8_t tsize, IN GFXfont *font) : text(what),color(color),swapColor(swap),font(font) {
    lLog("Created Text on %p swap: %s\n",this,(swap?"true":"false"));
    imageTextCanvas=new TFT_eSprite(tft);
    imageTextCanvas->setColorDepth(1);
    imageTextCanvas->setFreeFont(font);
    imageTextCanvas->setTextColor(TFT_WHITE);
    imageTextCanvas->setTextSize(tsize);
    imageTextCanvas->setTextDatum(TL_DATUM);
    int16_t width = imageTextCanvas->textWidth(text);
    int16_t height = imageTextCanvas->fontHeight()*tsize;
    imageTextCanvas->createSprite(width,height);
    imageTextCanvas->fillSprite(TFT_BLACK);
    imageTextCanvas->drawString(text,0,0);
}

void Text::Refresh(bool direct,bool swap) {
    //lLog("Text %p Refresh swap: %s\n",this,(swap?"true":"false"));
    Control::Refresh(direct,swap);

    uint16_t fcolor = color;
    if ( swap ) { fcolor = ByteSwap(color);
        /*
        double r = ((color >> 11) & 0x1F) / 31.0; // red   0.0 .. 1.0
        double g = ((color >> 5) & 0x3F) / 63.0;  // green 0.0 .. 1.0
        double b = (color & 0x1F) / 31.0;         // blue  0.0 .. 1.0
        fcolor = tft->color565(255*b,255*r,255*g);
        */
    }
    // center text
    int cXOff=(width-imageTextCanvas->width())/2;
    int cYOff=(height-imageTextCanvas->height())/2;
    for(int y=0;y<imageTextCanvas->height();y++) {
        for(int x=0;x<imageTextCanvas->width();x++) {
            bool isTextPart = imageTextCanvas->readPixel(x,y);
            if ( false == isTextPart ) { continue; } 
            canvas->drawPixel(cXOff+x,cYOff+y,fcolor);
        }
    }
}
