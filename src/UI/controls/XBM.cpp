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

#include "XBM.hpp"
#include "../../app/LogView.hpp"

using namespace LuI;

XBM::~XBM() {
    lLog("XBM %p destroyed!\n",this);
    if ( nullptr != imageCanvas ) {
        imageCanvas->deleteSprite();
        delete imageCanvas;
        imageCanvas=nullptr;
    }
}

XBM::XBM(IN uint32_t width,IN uint32_t height, IN unsigned char *data) :
                            imageWidth(width),imageHeight(height),imageData(data) {
    lLog("XBM %p created!\n",this);
    imageCanvas=new TFT_eSprite(tft);
    imageCanvas->setColorDepth(1);
    imageCanvas->createSprite(width,height);
    imageCanvas->fillSprite(TFT_BLACK);
    imageCanvas->drawXBitmap(0,0,data,width,height,TFT_WHITE);

    //imageCanvas->pushImage(0,0,width,height, (uint16_t *)data);
}

void XBM::Refresh(bool direct) {
    lLog("XBM %p refresh direct: %s\n",this,(direct?"true":"false"));
    if (( dirty )||(nullptr==canvas)) { Control::Refresh(direct); }
    //centered
    //canvas->setPivot(canvas->width()/2,canvas->height()/2);
    //imageCanvas->setPivot(imageCanvas->width()/2,imageCanvas->height()/2);
    //imageCanvas->pushRotated(canvas,0,TFT_BLACK);
    uint16_t fcolor = color;
    if ( false == direct ) {
        fcolor = ByteSwap(color);
    }
    // center image
    int cXOff=(int(width)-imageCanvas->width())/2;
    int cYOff=(int(height)-imageCanvas->height())/2;
    //lLog("cXOff: %d cYOff: %d\n",cXOff,cYOff);
    for(int y=0;y<imageCanvas->height();y++) {
        int32_t dY = cYOff+y;
        for(int x=0;x<imageCanvas->width();x++) {
            bool mustDraw = imageCanvas->readPixel(x,y);
            if ( mustDraw) {
                int32_t dX = cXOff+x;
                canvas->drawPixel(dX,dY,fcolor);
            }
        }
    }

}
