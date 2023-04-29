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

//#include <Arduino.h>
#include <LilyGoWatch.h>
#include "CanvasWidget.hpp"
#include <libraries/TFT_eSPI/TFT_eSPI.h>

//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include "../../app/LogView.hpp"

extern TTGOClass *ttgo; // ttgo lib

CanvasWidget::CanvasWidget(int16_t h, int16_t w,int8_t colorDepth) {
    lUIDeepLog("%s new %p\n",__PRETTY_FUNCTION__,this);
    RebuildCanvas(h,w,colorDepth);
}

bool CanvasWidget::RebuildCanvas(int16_t h, int16_t w, int8_t colorDepth) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    if ( nullptr != canvas ) {
        canvas->deleteSprite();
        delete canvas;
        canvas=nullptr;
    }
    canvas = new TFT_eSprite(tft);
    if ( nullptr == canvas ) { return false; }
    canvas->setColorDepth(colorDepth);
    this->colorDepth = colorDepth;
    if ( nullptr == canvas->createSprite(w,h) ) { return false; }
    
    if ( 4 == colorDepth ) {
        canvas->createPalette(default_4bit_palette);
    }
    canvas->fillSprite(MASK_COLOR); // by default use transparent
    return true;
}

CanvasWidget::~CanvasWidget() {
    if ( nullptr != canvas ) {
        canvas->deleteSprite();
        delete canvas;
        canvas=nullptr;
    }
    lUIDeepLog("%s delete %p\n",__PRETTY_FUNCTION__,this);
}

void CanvasWidget::DirectDraw(int16_t x,int16_t y) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    if ( nullptr != canvas ) {
        canvas->pushSprite(x,y,Drawable::MASK_COLOR);
    }
}

void CanvasWidget::DumpAlphaTo(TFT_eSprite * endCanvas, int16_t x, int16_t y, uint8_t alpha, uint16_t maskColor) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    for (int cy=0;cy<canvas->height();cy++) {
        for (int cx=0;cx<canvas->width();cx++) {
            uint16_t color = canvas->readPixel(cx,cy);
            if ( maskColor == color ) { continue; }
            uint16_t color2 = endCanvas->readPixel(cx+x,cy+y);
            uint16_t mColor = tft->alphaBlend(alpha,color,color2);
            endCanvas->drawPixel(cx+x,cy+y,mColor);
        }
    }
}

void CanvasWidget::DumpTo(TFT_eSprite * endCanvas, int16_t x, int16_t y, uint16_t maskColor) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    for (int cy=0;cy<canvas->height();cy++) {
        for (int cx=0;cx<canvas->width();cx++) {
            uint16_t color = canvas->readPixel(cx,cy);
            if ( maskColor == color ) { continue; }
            endCanvas->drawPixel(cx+x,cy+y,color);
        }
    }
}

void CanvasWidget::DrawTo(TFT_eSprite * endCanvas, int16_t x, int16_t y, uint16_t maskColor) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    if ( nullptr == endCanvas) { return; }
    if ( nullptr == canvas ) { return; }
    endCanvas->setPivot(x,y);
    canvas->setPivot(0,0);
    canvas->pushRotated(endCanvas,0,maskColor);
    canvas->setPivot(canvas->width()/2,canvas->height()/2);
    endCanvas->setPivot(endCanvas->width()/2,endCanvas->height()/2);
}
