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

using namespace LuI;

Buffer::~Buffer() {
    lLog("Buffer %p destroyed!\n",this);
    if ( nullptr != imageCanvas ) {
        imageCanvas->deleteSprite();
        delete imageCanvas;
        imageCanvas=nullptr;
    }
}
Buffer::Buffer(IN uint32_t width, IN uint32_t height,bool swap)//,LuI_Layout layout, size_t childs)
                                : imageWidth(width),imageHeight(height) {//,Container(layout,childs) {
    lLog("Buffer %p created!\n",this);
    imageCanvas=new TFT_eSprite(tft);
    imageCanvas->setColorDepth(16);
    imageCanvas->createSprite(width,height);
    //dragEnable=true;
    dragCallbackParam=this;
    dragCallback = [](void * obj){
        Buffer * self = (Buffer *)obj;
        TFT_eSprite * buff = self->GetBuffer();
        int32_t originalOffX = self->offsetX;
        int32_t originalOffY = self->offsetY;
        int16_t relDragX = touchDragVectorX; //self->lastDragX-touchDragVectorX;
        int16_t relDragY = touchDragVectorY; // self->lastDragY-touchDragVectorY;

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

        // redraw only if offset changes
        if ( ( originalOffX != self->offsetX ) || ( originalOffY != self->offsetY) ) {
            self->dirty=true;
        }
    };

}

void Buffer::Refresh(bool direct,bool swap) {
    lLog("Buffer %p refresh\n",this);
    Control::Refresh(direct,swap);
    //centered
    canvas->setPivot(0,0); //(canvas->width()/2),(canvas->height()/2));
    //canvas->fillSprite(ThCol(background));

    /*
    //FAKE all to get the contents on the sprite instead the control
    TFT_eSprite * realCanvas = canvas;
    uint32_t realWidth = width;
    uint32_t realHeight = height;
    width=imageCanvas->width();
    height=imageCanvas->width();
    canvas=imageCanvas;
    Container::Refresh(swap);
    canvas=realCanvas;
    width=realWidth;
    height=realHeight;
    */
    imageCanvas->setPivot(offsetX,offsetY);
    imageCanvas->pushRotated(canvas,0);
}

void Buffer::EventHandler() {

    if ( dirty ) { Refresh(); }
/*
    ttgo->tft->drawRect(clipX+3,clipY+3,width-6,height-6,TFT_BLUE);
    
    // @TODO GET THE WINDOW!!!
    for(size_t current=0;current<currentChildSlot;current++) {
        if ( nullptr == children[current] ) { continue; }
        children[current]->EventHandler();
        if ( children[current]->dirty ) {
            children[current]->Refresh(true);
*/
            //TFT_eSprite * what = children[current]->GetCanvas();
            /*
            if ( directDraw ) {
                ttgo->tft->setSwapBytes(true);
                what->pushSprite(children[current]->clipX,children[current]->clipY,Drawable::MASK_COLOR);
            }*/
        //}
    //}
    /*
    uint32_t oTouchX = touchX;
    uint32_t oTouchY = touchY;
    touchX-=offsetX;
    touchY-=offsetY;
    */
    Control::EventHandler();
    //touchX=oTouchX;
    //touchY=oTouchY;
}