#include "../../../lunokIoT.hpp"
#include "Control.hpp"
#include "../../../app/LogView.hpp"

using namespace LuI;


bool Control::Attach(INOUT Control *control) {
    parent=control;
    lLog("Control %p attached to %p\n",this,control);
    return true;
}

Control::~Control() {
    if ( nullptr != canvas ) {
        canvas->deleteSprite();
        delete canvas;
        canvas=nullptr;
    }
    lLog("Control destroyed on %p\n",this);
}

Control::Control(uint32_t w, uint32_t h): width(w),height(h) {
    lLog("Control created on %p\n",this);
}

void Control::SetSize(uint32_t w, uint32_t h) {
    this->width=w;
    this->height=h;
}

void Control::SetWidth(uint32_t w) {
    this->width=w;
}

void Control::SetHeight(uint32_t h) {
    this->height=h;
}

void Control::Refresh() {
    if ( nullptr != canvas ) {
        canvas->deleteSprite();
        delete canvas;
        canvas=nullptr;
    }
    canvas = new TFT_eSprite(ttgo->tft);
    canvas->setColorDepth(16);
    canvas->createSprite(width,height);
    lLog("Control refresh on %p canvas: %p\n",this,canvas);
    //@TODO fill the canvas here
    canvas->fillSprite(Drawable::MASK_COLOR);
}

TFT_eSprite * Control::GetCanvas() { return canvas; }
