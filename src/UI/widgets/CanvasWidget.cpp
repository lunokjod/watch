#include <Arduino.h>
#include <LilyGoWatch.h>
#include "CanvasWidget.hpp"
#include "../activator/ActiveRect.hpp"

extern TFT_eSprite *overlay;

CanvasWidget::CanvasWidget(int16_t h, int16_t w) {
    RebuildCanvas(h,w);
}

void CanvasWidget::RebuildCanvas(int16_t h, int16_t w) {
    if ( nullptr != canvas ) {
        canvas->deleteSprite();
    }
    canvas = new TFT_eSprite(ttgo->tft);
    canvas->setColorDepth(16);
    canvas->createSprite(w,h);
    canvas->fillSprite(MASK_COLOR);
}

TFT_eSprite *CanvasWidget::GetCanvas() { return this->canvas; }


CanvasWidget::~CanvasWidget() {
    if (nullptr != this->canvas ) {
        TFT_eSprite *tmp = this->canvas;
        this->canvas = nullptr;
        tmp->deleteSprite();
        delete tmp;
        Serial.printf("Destroyed canvas: %p\n", this);
    }
}

void CanvasWidget::DrawTo(TFT_eSprite * endCanvas, int16_t x, int16_t y, int32_t maskColor) {
    if ( nullptr == endCanvas) { return; }
    if ( nullptr == canvas ){ return; }
    endCanvas->setPivot(x,y);
    canvas->setPivot(0,0);
    // DEBUG canvas->fillSprite(TFT_YELLOW);
    canvas->pushRotated(endCanvas,0,maskColor);
    canvas->setPivot(this->canvas->width()/2,this->canvas->height()/2);
    endCanvas->setPivot(endCanvas->width()/2,endCanvas->height()/2);
    //overlay->drawRect(x,y,this->canvas->width(),this->canvas->height(),TFT_YELLOW);
}
