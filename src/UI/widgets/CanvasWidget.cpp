#include <Arduino.h>
#include "CanvasWidget.hpp"
#include <libraries/TFT_eSPI/TFT_eSPI.h>
extern TFT_eSPI *tft;
#include "../../app/LogView.hpp"

CanvasWidget::CanvasWidget(int16_t h, int16_t w) {
    lUIDeepLog("%s new %p\n",__PRETTY_FUNCTION__,this);
    RebuildCanvas(h,w);
}

void CanvasWidget::RebuildCanvas(int16_t h, int16_t w) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
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
    lUIDeepLog("%s delete %p\n",__PRETTY_FUNCTION__,this);
}

void CanvasWidget::DirectDraw(int16_t x,int16_t y) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    canvas->pushSprite(x,y,Drawable::MASK_COLOR);
}

void CanvasWidget::DrawTo(TFT_eSprite * endCanvas, int16_t x, int16_t y, int32_t maskColor) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    if ( nullptr == endCanvas) { return; }
    endCanvas->setPivot(x,y);
    canvas->setPivot(0,0);
    canvas->pushRotated(endCanvas,0,maskColor);
    canvas->setPivot(canvas->width()/2,canvas->height()/2);
    endCanvas->setPivot(endCanvas->width()/2,endCanvas->height()/2);
}
