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

/*
extern TFT_eSprite *overlay;

CanvasZWidget::CanvasZWidget(int16_t h, int16_t w, float z):z(z) {
    RebuildCanvas(w,h);
}

void CanvasZWidget::RebuildCanvas(int16_t w, int16_t h) {
    if ( nullptr != canvas ) {
        canvas->deleteSprite();
        delete canvas;
        canvas = nullptr;
    }
    canvas = new TFT_eSprite(ttgo->tft);
    canvas->setColorDepth(16);
    canvas->createSprite(h,w);
    canvas->fillSprite(MASK_COLOR);
}

TFT_eSprite *CanvasZWidget::canvas { return this->canvas; }


CanvasZWidget::~CanvasZWidget() {
    if (nullptr != this->canvas ) {
        TFT_eSprite *tmp = this->canvas;
        this->canvas = nullptr;
        tmp->deleteSprite();
        delete tmp;
        Serial.printf("Destroyed canvas: %p\n", this);
    }
}

void CanvasZWidget::DrawTo(TFT_eSprite * endCanvas, int16_t x, int16_t y, float z, bool centered,int32_t maskColor) {
    int16_t originalPivotX = endCanvas->getPivotX();
    int16_t originalPivotY = endCanvas->getPivotY();
    if ( centered ) {
        int16_t px = (scaledCopy->width()/2);
        int16_t py = (scaledCopy->height()/2);
        endCanvas->setPivot(x-px,y-py);
        scaledCopy->setPivot(0,0);        
    } else {
        endCanvas->setPivot(x,y);
        scaledCopy->setPivot(0,0);
    }
    scaledCopy->pushRotated(endCanvas,0,maskColor);
    // restore caller pivot
    endCanvas->setPivot(originalPivotX,originalPivotY);


}
*/