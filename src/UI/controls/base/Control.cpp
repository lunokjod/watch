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
    //lLog("Control destroyed on %p\n",this);
}

Control::Control(uint32_t w, uint32_t h): width(w),height(h) {
    //lLog("Control created on %p\n",this);
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

void Control::EventHandler() {
    if ( false == touchEnabled ) { return; }
    if ( touched ) {
        if ( ( touchX > clipX ) && ( touchX < clipX+width ) 
            && ( touchY > clipY ) && ( touchY < clipY+height )  ) {
            //lLog("%p RECT: X: %d Y: %d W: %d H: %d\n",this,nCX,nCY,width,height);
            if ( false == lastTouched ) { // yeah! IN!!!
                lastTouched=true;
                dirty=true;
            }
        } else {
            if (lastTouched) { // was touched, not now
                lastTouched=false;
                dirty=true;
            }
        }
    } else {
        if ( lastTouched )  { // launch callback
            if ( nullptr != callback ) { (callback)(callbackParam); }
            lastTouched=false;
            dirty=true;
        }
    }
}

void Control::Refresh(bool swap) {
    if ( nullptr != canvas ) {
        canvas->deleteSprite();
        delete canvas;
        canvas=nullptr;
    }
    canvas = new TFT_eSprite(ttgo->tft);
    canvas->setColorDepth(16);
    canvas->createSprite(width,height);
    canvas->fillSprite(Drawable::MASK_COLOR);
    //lLog("Control %p refresh canvas at %p swap: %s\n",this,canvas,(swap?"true":"false"));
}

TFT_eSprite * Control::GetCanvas() { return canvas; }
