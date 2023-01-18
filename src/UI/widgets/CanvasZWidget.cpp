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
    if ( directDraw ) { // if availaible, use it!
        scaledCopy->pushSprite(x,y,maskColor);
    }
    if ( nullptr != endCanvas ) {
        CanvasWidget::DrawTo(endCanvas,x,y,maskColor);
    }
    canvas = backup;
    scaledCopy->deleteSprite();
    delete scaledCopy;
}
