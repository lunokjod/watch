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

#include "../../../lunokIoT.hpp"
#include "Control.hpp"
#include "../../../app/LogView.hpp"

using namespace LuI;


bool Control::Attach(INOUT Control *control) {
    if ( parent ) { return false; }
    parent=control;
    lLog("Control %p attached to %p\n",this,control);
    dirty=true; // mark myself as dirty to be refreshed
    return true;
}

Control::~Control() {
    lLog("Control %p destroyed!\n",this);
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
    dirty=true;
}

void Control::SetWidth(uint32_t w) {
    this->width=w;
    dirty=true;
}

void Control::SetHeight(uint32_t h) {
    this->height=h;
    dirty=true;
}

void Control::EventHandler() {
    //tft->drawRect(clipX+3,clipY+3,width-6,height-6,TFT_RED); //@DEBUG

    if ( touched ) {
        // check drag
        if ( ( touchX > clipX ) && ( touchX < clipX+width ) 
                && ( touchY > clipY ) && ( touchY < clipY+height )  ) {
            if ( touchDragDistance > 0.0 ) {
                if ( nullptr != dragCallback ) {
                    (dragCallback)(dragCallbackParam);
                }
            }
        }
        // check touch
        if ( ( touchX > clipX ) && ( touchX < clipX+width ) 
                && ( touchY > clipY ) && ( touchY < clipY+height )  ) {
            //  lLog("%p RECT: X: %d Y: %d W: %d H: %d\n",this,nCX,nCY,width,height);
            if ( false == lastTouched ) { // yeah! IN!!!
                //lLog("BEGIN TOUCH\n");
                lastTouched=true;
                dirty=true;
            }
        } else {
            if (lastTouched) { // was touched, not now
                //lLog("OUT TOUCH\n");
                lastTouched=false;
                dirty=true;
            }
        }
    } else {
        if ( lastTouched )  { // launch callback on tap
            if ( nullptr != tapCallback ) {
                lLog("LAUNCH TOUCH\n");
                (tapCallback)(tapCallbackParam);
            }
            lastTouched=false;
            dirty=true;
        }
    }
}

void Control::Refresh(bool direct, bool swap) {
    // this base control don't support direct, only buffered
    //lLog("Control %p refresh canvas at %p swap: %s dirty: %s\n",this,canvas,(swap?"true":"false"),(dirty?"true":"false"));
    if ( dirty ) {
        if ( nullptr != canvas ) {
            canvas->deleteSprite();
            delete canvas;
            canvas=nullptr;
        }
        canvas = new TFT_eSprite(tft);
        canvas->setColorDepth(16);
        canvas->createSprite(width,height);
        canvas->fillSprite(Drawable::MASK_COLOR);
        dirty=false;
    }
}

TFT_eSprite * Control::GetCanvas() { return canvas; }
