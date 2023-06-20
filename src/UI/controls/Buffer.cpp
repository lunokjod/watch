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

#include "Buffer.hpp"
#include "../../app/LogView.hpp"
#include <Ticker.h>

using namespace LuI;

//Ticker _bufferFadeAnim;
//Buffer * _bufferFadeAnimRelatedBuffer=nullptr;

Buffer::~Buffer() {
    lLog("Buffer %p destroyed!\n",this);
    if ( nullptr != imageCanvas ) {
        imageCanvas->deleteSprite();
        delete imageCanvas;
        imageCanvas=nullptr;
    }
    if ( nullptr != barHorizontal ) {
        barHorizontal->deleteSprite();
        delete barHorizontal;
        barHorizontal=nullptr;
    }
    if ( nullptr != barVertical ) {
        barVertical->deleteSprite();
        delete barVertical;
        barVertical=nullptr;
    }
}
Buffer::Buffer(IN uint32_t width, IN uint32_t height)//,LuI_Layout layout, size_t childs)
                                : imageWidth(width),imageHeight(height) {//,Container(layout,childs) {
    lLog("Buffer %p created!\n",this);
    imageCanvas=new TFT_eSprite(tft);
    imageCanvas->setColorDepth(16);
    imageCanvas->createSprite(width,height);
    dragCallbackParam=this;
    dragCallback = [](void * obj){
        Buffer * self = (Buffer *)obj;
        TFT_eSprite * buff = self->GetBuffer();
        int32_t originalOffX = self->offsetX;
        int32_t originalOffY = self->offsetY;
        int16_t relDragX = touchDragVectorX;
        int16_t relDragY = touchDragVectorY;

        // James Bruton smooth https://hackaday.com/2021/09/03/smooth-servo-motion-for-lifelike-animatronics/
        int32_t nOffX = self->offsetX+(relDragX*-1);
        int32_t nOffY = self->offsetY+(relDragY*-1);
        self->offsetX=(self->offsetX*0.8)+(nOffX*0.2);
        self->offsetY=(self->offsetY*0.8)+(nOffY*0.2);

        const int32_t maxX = buff->width()-self->width;
        const int32_t maxY = buff->height()-self->height;
        if ( self->offsetX < 0 ) { self->offsetX=0; }
        else if ( self->offsetX > maxX ) { self->offsetX=maxX; }
        if ( self->offsetY < 0 ) { self->offsetY=0; }
        else if ( self->offsetY > maxY ) { self->offsetY=maxY; }
        self->fadeDownBarsValue=255;
        // redraw only if offset changes
        if ( ( originalOffX != self->offsetX ) || ( originalOffY != self->offsetY) ) {
            self->dirty=true;
        }
    };
}

void Buffer::DrawBars() {
    //if ( hideBars ) { hideBars=false; return; } // hide bars received
    if ( fadeDownBarsValue < 1 ) { return; } // ready fade-down
    if ( nullptr == barHorizontal ) { // create bar buffer H
        barHorizontal=new TFT_eSprite(tft);
        barHorizontal->setColorDepth(1);
        barHorizontal->createSprite(width-30,10);
    }
    int32_t percentX=0;
    int32_t percentY=0;
    int32_t maxX = imageCanvas->width()-int(width);
    // dont divide by zero
    if (maxX != 0) {
        percentX = (offsetX*(int(width)-50))/maxX;
    }
    int32_t maxY = imageCanvas->height()-int(height);
    if (maxY != 0) {
        percentY = (offsetY*(int(height)-50))/maxY;
    }

    barHorizontal->fillSprite(TFT_BLACK);
    barHorizontal->drawRoundRect(0,0,width-30,10,5,TFT_WHITE);
    barHorizontal->fillCircle(5+percentX,5,5,TFT_WHITE);
    if ( width < imageWidth ) { // only if my view is smaller than buffer width
        for(int y=0;y<barHorizontal->height();y++) {
            for(int x=0;x<barHorizontal->width();x++) {
                bool mustDraw = barHorizontal->readPixel(x,y);
                if ( mustDraw ) {
                    uint16_t color = canvas->readPixel(15+x,5+y);
                    uint16_t fcolor = tft->alphaBlend(fadeDownBarsValue,TFT_WHITE,color);
                    canvas->drawPixel(15+x,5+y,fcolor);
                }
            }
        }
    }

    if ( nullptr == barVertical ) { // create bar buffer V
        barVertical=new TFT_eSprite(tft);
        barVertical->setColorDepth(1);
        barVertical->createSprite(10,height-30);
    }
    barVertical->fillSprite(TFT_BLACK);
    barVertical->drawRoundRect(0,0,10,height-30,5,TFT_WHITE);
    barVertical->fillCircle(5,5+percentY,5,TFT_WHITE);
    if ( height < imageHeight ) { // only if my view is smaller than buffer height
        for(int y=0;y<barVertical->height();y++) {
            for(int x=0;x<barVertical->width();x++) {
                bool mustDraw = barVertical->readPixel(x,y);
                if ( mustDraw ) {
                    uint16_t color = canvas->readPixel(5+x,15+y);
                    uint16_t fcolor = tft->alphaBlend(fadeDownBarsValue,TFT_WHITE,color);
                    canvas->drawPixel(5+x,15+y,fcolor);
                }
            }
        }
    }
}

void Buffer::Refresh(bool direct) {
    //lLog("Buffer %p refresh direct: %s \n",this,(direct?"true":"false"));
    if ( false == dirty ) { return; } // I'm clean!!
    Control::Refresh();

    if ( nullptr != bufferPushCallback ) { (bufferPushCallback)(bufferPushCallbackParam); }
    canvas->fillSprite(backgroundColor);
    //centered
    canvas->setPivot(0,0);
    imageCanvas->setPivot(offsetX,offsetY);
    imageCanvas->pushRotated(canvas,0);
    // redraw bars?
    if ( showBars ) { DrawBars(); }
    if ( direct ) { canvas->pushSprite(clipX,clipY); }
    if ( fadeDownBarsValue > 0 ) {
        fadeDownBarsValue-=32;
        dirty=true; // fade down
    }
}
