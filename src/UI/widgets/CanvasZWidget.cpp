#include <Arduino.h>
#include <LilyGoWatch.h>
#include "CanvasZWidget.hpp"
#include "CanvasWidget.hpp"
#include "../activator/ActiveRect.hpp"

extern TFT_eSprite *overlay;

CanvasZWidget::CanvasZWidget(int16_t h, int16_t w, float z):z(z) {
    RebuildCanvas(h,w);
}

void CanvasZWidget::RebuildCanvas(int16_t h, int16_t w) {
    if ( nullptr != canvas ) {
        canvas->deleteSprite();
        delete canvas;
        canvas = nullptr;
    }
    canvas = new TFT_eSprite(ttgo->tft);
    canvas->setColorDepth(16);
    canvas->createSprite(w,h);
    canvas->fillSprite(MASK_COLOR);
}

TFT_eSprite *CanvasZWidget::GetCanvas() { return this->canvas; }


CanvasZWidget::~CanvasZWidget() {
    if (nullptr != this->canvas ) {
        TFT_eSprite *tmp = this->canvas;
        this->canvas = nullptr;
        tmp->deleteSprite();
        delete tmp;
        Serial.printf("Destroyed canvas: %p\n", this);
    }
}

void CanvasZWidget::DrawTo(TFT_eSprite * endCanvas, int16_t x, int16_t y, float z,int32_t maskColor) {
    if ( nullptr == endCanvas) { return; }
    if ( nullptr == canvas ){ return; }
    TFT_eSprite * scaledCopy = SimplifyScreenShootFrom(canvas, z);
    this->z=z;

    endCanvas->setPivot(x,y);
    scaledCopy->setPivot(x,y);
    scaledCopy->pushRotated(endCanvas,0,maskColor);

    scaledCopy->setPivot(x,y);
    endCanvas->setPivot(endCanvas->width()/2,endCanvas->height()/2);

    scaledCopy->deleteSprite();
    delete scaledCopy;
}
