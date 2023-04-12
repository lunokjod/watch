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
    lUILog("Control %p attached to %p\n",this,control);
    dirty=true; // mark myself as dirty to be refreshed as child
    return true;
}

Control::~Control() {
    lUILog("Control %p destroyed!\n",this);
    if ( nullptr != canvas ) {
        if ( canvas->created() ) { canvas->deleteSprite(); }
        delete canvas;
        canvas=nullptr;
    }
}

Control::Control(uint32_t w, uint32_t h): width(w),height(h) {
    lUILog("Control created on %p\n",this);
}

void Control::SetSize(uint32_t w, uint32_t h) {
    if ( ( w != this->width ) || ( h != this->height ) ) {
        this->width=w;
        this->height=h;
        dirty=true;
        Rebuild();
    }
}

void Control::SetWidth(uint32_t w) {
    if ( w != this->width ) {
        this->width=w;
        dirty=true;
        Rebuild();
    }
}

void Control::SetHeight(uint32_t h) {
    if ( ( h != this->height ) ) {
        this->height=h;
        dirty=true;
        Rebuild();
    }
}

void Control::EventHandler() {
    //tft->drawRect(clipX+3,clipY+3,width-6,height-6,TFT_RED); //@DEBUG
    bool sensitive=false;
    if ( nullptr != tapCallback ) { sensitive=true; }
    else if ( nullptr != inTouchCallback ) { sensitive=true; }
    else if ( nullptr != outTouchCallback ) { sensitive=true; }
    else if ( nullptr != touchCallback ) { sensitive=true; }
    else if ( nullptr != dragCallback ) { sensitive=true; }
    if ( false == sensitive ) { return; } // no callbacks, no interaction, no party :(
    if ( touched ) {
        // check area
        if ( ( touchX > clipX ) && ( touchX < clipX+width ) 
                && ( touchY > clipY ) && ( touchY < clipY+height )  ) {
            // check touch
            if ( nullptr != touchCallback ) { (touchCallback)(touchCallbackParam); }

            // check drag
            if ( nullptr != dragCallback ) {
                if ( touchDragDistance > 0.0 ) {
                    //lLog("Control %p: Drag\n",this);
                    (dragCallback)(dragCallbackParam);
                }
            }
            // check in
            if ( false == lastTouched ) { // check tap
                if ( nullptr != inTouchCallback ) {
                    lUILog("Control %p: Thumb down\n",this);
                    (inTouchCallback)(inTouchCallbackParam);
                }
                dirty=true;
                lastTouched=true;
            }
        } else { // out of area
            if (lastTouched) { // was touched
                if ( nullptr != outTouchCallback ) {
                    lUILog("Control %p: Thumb leave\n",this);
                    (outTouchCallback)(outTouchCallbackParam);
                }
                dirty=true;
                lastTouched=false;
            }
        }
    } else { // no touch
        if ( lastTouched )  { // pushed but now out of touch
            if ( nullptr != outTouchCallback ) {
                lUILog("Control %p: Thumb up\n",this);
                (outTouchCallback)(outTouchCallbackParam);
                dirty=true;
            }
            if ( nullptr != tapCallback ) { // great! is pushed :D
                if ( touchDragDistance == 0 ) {
                    lUILog("Control %p: Tap callback\n",this);
                    (tapCallback)(tapCallbackParam);
                    dirty=true;
                }
            }
            lastTouched=false;
        }
    }
}

void Control::Rebuild() {
    lUILog("Control %p Rebuild (w:%u h:%u)\n",this,width,height);
    if ( nullptr != canvas ) {
        if ( canvas->created() ) { canvas->deleteSprite(); }
        delete canvas;
    }
    // create new with current size
    canvas = new TFT_eSprite(tft);
    canvas->setColorDepth(16);
    canvas->createSprite(width,height);
}

void Control::Refresh(bool direct) {
    if ( false == dirty ) { return; }
    //lUILog("Control %p Refresh direct: %s\n",this,(direct?"true":"false"));
    //lUILog("Control %p Refresh\n",this);
    if ( nullptr == canvas ) { Rebuild(); }
    else {
        if ( ( width != canvas->width() ) || ( height != canvas->height() )) {
            lUILog("Control %p viewport changed\n",this);
            Rebuild();
        }
    }
    uint16_t color=Drawable::MASK_COLOR;
    //if ( false == direct ) { color=ByteSwap(color); }
    canvas->fillSprite(color);
    dirty=false;
}

TFT_eSprite * Control::GetCanvas() {
    if ( nullptr == canvas ) { Rebuild(); }
    return canvas;
}
