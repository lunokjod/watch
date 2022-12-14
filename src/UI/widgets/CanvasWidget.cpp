#include <Arduino.h>
#include "CanvasWidget.hpp"
#include <libraries/TFT_eSPI/TFT_eSPI.h>
extern TFT_eSPI *tft;
#include "../../app/LogView.hpp"

CanvasWidget::CanvasWidget(int16_t h, int16_t w,int8_t colorDepth) {
    lUIDeepLog("%s new %p\n",__PRETTY_FUNCTION__,this);
    RebuildCanvas(h,w,colorDepth);
}
bool CanvasWidget::RebuildCanvas(int16_t h, int16_t w, int8_t colorDepth) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    if ( nullptr != canvas ) {
        canvas->deleteSprite();
        delete canvas;
        return false;
    }
    canvas = new TFT_eSprite(tft);
    canvas->setColorDepth(colorDepth);
    this->colorDepth = colorDepth;
    canvas->createSprite(w,h);
    if ( 4 == colorDepth ) {
        canvas->createPalette(default_4bit_palette);
    }
    canvas->fillSprite(MASK_COLOR); // by default use transparent
    return true;
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
