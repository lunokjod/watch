#include <Arduino.h>
#include "CanvasWidget.hpp"
#include <libraries/TFT_eSPI/TFT_eSPI.h>
extern TFT_eSPI *tft;

CanvasWidget::CanvasWidget(int16_t h, int16_t w) { RebuildCanvas(h,w); }

void CanvasWidget::RebuildCanvas(int16_t h, int16_t w) {
    if ( nullptr != canvas ) {
        canvas->deleteSprite();
        delete canvas;
    }
    canvas = new TFT_eSprite(tft);
    canvas->setColorDepth(16);
    canvas->createSprite(w,h);
    canvas->fillSprite(MASK_COLOR); // by default use transparent
}

CanvasWidget::~CanvasWidget() {
    canvas->deleteSprite();
    delete canvas;
}

void CanvasWidget::DirectDraw(int16_t x=0,int16_t y=0) {
    //tft->setSwapBytes(false);
    //canvas->setSwapBytes(true);
    canvas->pushSprite(x,y);
    //canvas->setSwapBytes(false);
    //tft->setSwapBytes(true);
}

void CanvasWidget::DrawTo(TFT_eSprite * endCanvas, int16_t x, int16_t y, int32_t maskColor) {
    if ( nullptr == endCanvas) { return; }
    endCanvas->setPivot(x,y);
    canvas->setPivot(0,0);
    canvas->pushRotated(endCanvas,0,maskColor);
    canvas->setPivot(canvas->width()/2,canvas->height()/2);
    endCanvas->setPivot(endCanvas->width()/2,endCanvas->height()/2);
}
