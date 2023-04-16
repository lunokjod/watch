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

#include "IconMenu.hpp"
#include "../../app/LogView.hpp"
#include <Ticker.h>

using namespace LuI;

//Ticker _bufferFadeAnim;
//Buffer * _bufferFadeAnimRelatedBuffer=nullptr;

IconMenu::~IconMenu() {
    lLog("IconMenu %p destroyed!\n",this);
    if ( nullptr != bufferCanvas ) {
        bufferCanvas->deleteSprite();
        delete bufferCanvas;
        bufferCanvas=nullptr;
    }
}

IconMenu::IconMenu(LuI_Layout layout, int menuEntries,const IconMenuEntry *optionList) 
                        : layout(layout), numEntries(menuEntries), entries(optionList) {
    lLog("IconMenu %p created!\n",this);

    inTouchCallbackParam=this;
    inTouchCallback = [](void * obj){
        IconMenu * self = (IconMenu *)obj;
        self->alreadyChanged=false;
        //lLog("IN\n");
    };
    outTouchCallbackParam=this;
    outTouchCallback = [](void * obj){
        IconMenu * self = (IconMenu *)obj;
        self->alreadyChanged=false;
        //lLog("OUT\n");
    };

    tapCallbackParam=this;
    tapCallback = [](void * obj){
        IconMenu * self = (IconMenu *)obj;
        self->Launch();
    };
    dragCallbackParam=this;
    dragCallback = [&](void * obj){
        //lLog("DRAG\n");
        IconMenu * self = (IconMenu *)obj;
        if ( self->alreadyChanged ) { return; }
        int32_t originalDisplace=self->displacement;
        int32_t maxDrag = self->width *0.5;//-(self->width/3);
        self->displacement = touchDragVectorX;
        if ( LuI_Vertical_Layout == self->layout ) {
            self->displacement = touchDragVectorY;
            maxDrag = self->height *0.5;//-(self->height/3);
        }

        if ( originalDisplace == self->displacement ) { return; }
        int selectedBefore=self->selectedEntry;
        if (self->displacement > maxDrag ) {
            self->selectedEntry--;
            self->displacement=0;
        } else if (self->displacement < maxDrag*-1 ) {
            self->selectedEntry++;
            self->displacement=0;
        }
        if ( self->selectedEntry > self->numEntries ) { self->selectedEntry=self->numEntries; }
        else if ( self->selectedEntry < 0 ) { self->selectedEntry=0; }

        if ( selectedBefore != self->selectedEntry) {
            // stops the drag
            self->alreadyChanged=true;
            if ( nullptr != pageCallback ) { (pageCallback)(pageCallbackParam); }
        }
        self->dirty=true;
    };
}

void IconMenu::Launch() {
    IconMenuEntry currentEntry = entries[selectedEntry];
    (currentEntry.callback)(this);
}

void IconMenu::Refresh(bool direct) {
    //lLog("IconMenu %p refresh direct: %s dirty: %s\n",this,(direct?"true":"false"),(dirty?"true":"false"));
    if ( false == dirty ) { return; } // I'm clean!!
    Control::Refresh(false);

    IconMenuEntry currentEntry = entries[selectedEntry];
    if ( false == touched ) { displacement=0; }

    canvas->fillSprite(backgroundColor);
    int16_t offX=((width-currentEntry.width)/2);
    int16_t offY=((height-currentEntry.height)/2);
    if ( LuI_Horizontal_Layout == layout ) { offX+=displacement; }
    else { offY+=displacement; }
    float colourPercent;
    if ( LuI_Horizontal_Layout == layout ) { colourPercent=width/2; }
    else { colourPercent=height/2; }
    colourPercent=(abs(displacement)/colourPercent);
    if ( colourPercent > 1.0 ) { colourPercent = 1.0; }
    else if ( colourPercent < 0.1 ) { colourPercent = 0.1; }
    uint16_t iconColor=tft->alphaBlend(255*colourPercent,backgroundColor,TFT_WHITE);

    uint16_t iconNewColor=tft->alphaBlend(255*colourPercent,TFT_WHITE,backgroundColor);

    if ( displacement > 0 ) {
        // show last?
        if ( selectedEntry-1 > -1 ) {
            IconMenuEntry nextEntry = entries[selectedEntry-1];
            int16_t noffX=((width-nextEntry.width)/2);
            int16_t noffY=((height-nextEntry.height)/2);
            canvas->drawXBitmap(noffX,noffY,nextEntry.imagebits,nextEntry.width,nextEntry.height,iconNewColor);
        }
    } else if ( displacement < 0 ) {
        // show next?
        if ( selectedEntry+1 < numEntries ) {
            IconMenuEntry nextEntry = entries[selectedEntry+1];
            int16_t noffX=((width-nextEntry.width)/2);
            int16_t noffY=((height-nextEntry.height)/2);
            canvas->drawXBitmap(noffX,noffY,nextEntry.imagebits,nextEntry.width,nextEntry.height,iconNewColor);
        }
    } else {
        if ( showCatalogue ) { //@TODO show other icons alongside like a "tabs"
            // horizontal
            /*
            if ( selectedEntry-1 > -1 ) {
                IconMenuEntry nextEntry = entries[selectedEntry-1];
                int16_t noffX=((width-nextEntry.width)/2);
                int16_t noffY=((height-nextEntry.height)/2);
                canvas->drawXBitmap(noffX,noffY,nextEntry.imagebits,nextEntry.width,nextEntry.height,iconColor);
            }*/
        }
    }
    canvas->drawXBitmap(offX,offY,currentEntry.imagebits,currentEntry.width,currentEntry.height,iconColor);
    if ( direct ) { canvas->pushSprite(clipX,clipY); }
    //lLog("currentEntry: %u/%u '%s' displace: %d WIDTH: %d\n", selectedEntry,numEntries,currentEntry.name,displacement,width);
}
