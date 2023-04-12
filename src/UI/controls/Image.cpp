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

#include "Image.hpp"
#include "../../app/LogView.hpp"

using namespace LuI;

Image::~Image() {
    lLog("Image %p destroyed!\n",this);
    if ( nullptr != imageCanvas ) {
        imageCanvas->deleteSprite();
        delete imageCanvas;
        imageCanvas=nullptr;
    }
}

Image::Image(IN uint32_t width,IN uint32_t height, IN unsigned char *data,
                                uint16_t maskColor,LuI_Layout layout, size_t childs)
                    : imageWidth(width),imageHeight(height),imageData(data),Container(layout,childs) {
    lLog("Image %p created!\n",this);
    uint16_t fcolor = maskColor;
    /*
    if ( false == swap ) {
        fcolor = ByteSwap(maskColor);
    }*/
    //lLog("---> ORIGINAL MASK COLOR: %x SWAPPED: %x\n",maskColor,fcolor);
    this->maskColor = fcolor;
    imageCanvas=new TFT_eSprite(tft);
    imageCanvas->setColorDepth(16);
    imageCanvas->createSprite(width,height);
    imageCanvas->setSwapBytes(true);
    if ( nullptr != data ) { imageCanvas->pushImage(0,0,width,height, (uint16_t *)data); }
    imageCanvas->setSwapBytes(false);
}

void Image::Refresh(bool direct) {
    lLog("Image %p refresh\n",this);
    Control::Refresh(direct);
    uint16_t fMaskcolor = maskColor;
    // center image
    int cXOff=(int(width)-imageCanvas->width())/2;
    int cYOff=(int(height)-imageCanvas->height())/2;
    for(int y=0;y<imageCanvas->height();y++) {
        int32_t dY = cYOff+y;
        for(int x=0;x<imageCanvas->width();x++) {
            uint16_t originalColor = imageCanvas->readPixel(x,y);
            if ( originalColor == fMaskcolor) { continue; }
            if ( direct ) { originalColor = ByteSwap(originalColor); }
            int32_t dX = cXOff+x;
            canvas->drawPixel(dX,dY,originalColor);
        }
    }

    Container::Refresh(direct);
}
