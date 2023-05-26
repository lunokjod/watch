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
#include "CanvasZWidget.hpp"
#include "../UI.hpp"
#include "../../app/LogView.hpp"

CanvasZWidget::CanvasZWidget(int16_t h, int16_t w, float z, int8_t colorDeepth): CanvasWidget(h,w,colorDeepth),z(z){
    // parent do all the work
}

CanvasZWidget::~CanvasZWidget() {
    /*
    if (nullptr != this->canvas ) {
        TFT_eSprite *tmp = this->canvas;
        this->canvas = nullptr;
        tmp->deleteSprite();
        delete tmp;
        lUILog("Destroyed canvas: %p\n", this);
    }*/
}
TFT_eSprite * CanvasZWidget::GetScaledImageClipped(float scale,int16_t maxW, int16_t maxH) {
    this->z=scale;
    TFT_eSprite * scaledCopy = ScaleSpriteMAX(canvas, scale,maxW,maxH);
    return scaledCopy;
}

TFT_eSprite * CanvasZWidget::GetScaledImage(float scale) {
    this->z=scale;
    TFT_eSprite * scaledCopy = ScaleSprite(canvas, scale);
    return scaledCopy;
}
void CanvasZWidget::DrawTo(int16_t x, int16_t y, float z, bool centered, int32_t maskColor) {
    DrawTo(nullptr,x,y,z,centered,maskColor);
}

bool CanvasZWidget::SetScale(float z) {
    if ( this->z != z ) {
        this->z=z;
        if ( ( nullptr != canvas ) && ( NULL != canvas ) ) {
            if ( true == canvas->created() ) {
                return CanvasWidget::RebuildCanvas(canvas->height(),canvas->width(), colorDepth);
            }
        }
    }
    return false;
}

void CanvasZWidget::DrawTo(TFT_eSprite * endCanvas, int16_t x, int16_t y, float z, bool centered, int32_t maskColor) {
    if ( nullptr == canvas ){ return; }
    TFT_eSprite * scaledCopy = GetScaledImage(z);
    scaledCopy->setPivot(0,0);
    if ( centered ) {
        x-=(scaledCopy->width()/2);
        y-=(scaledCopy->height()/2);
    }
    TFT_eSprite * backup = canvas;
    canvas = scaledCopy;
    if ( nullptr == endCanvas ) {
        if ( directDraw ) { // if availaible, use it!
            scaledCopy->pushSprite(x,y,maskColor);
        }
    } else {
        CanvasWidget::DrawTo(endCanvas,x,y,maskColor);
    }
    canvas = backup;
    scaledCopy->deleteSprite();
    delete scaledCopy;
}
