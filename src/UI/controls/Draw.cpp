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

#include "Draw.hpp"
#include "../../app/LogView.hpp"
#include <Ticker.h>

using namespace LuI;

Draw::~Draw() {
    lLog("Draw %p destroyed!\n",this);
    if ( nullptr != imageCanvas ) {
        imageCanvas->deleteSprite();
        delete imageCanvas;
        imageCanvas=nullptr;
    }
}
Draw::Draw(IN uint32_t width, IN uint32_t height)//,LuI_Layout layout, size_t childs)
                                : imageWidth(width),imageHeight(height) {//,Container(layout,childs) {
    lLog("Draw %p created!\n",this);
    imageCanvas=new TFT_eSprite(tft);
    imageCanvas->setColorDepth(16);
    imageCanvas->createSprite(width,height);
}

void Draw::Refresh(bool direct) {
    //lLog("Buffer %p refresh direct: %s swap: %s\n",this,(direct?"true":"false"),(swap?"true":"false"));
    if ( false == dirty ) { return; } // I'm clean!!
    Control::Refresh(direct);

    if ( nullptr != drawPushCallback ) { (drawPushCallback)(drawPushCallbackParam); }

    //centered
    //canvas->setPivot(0,0);
    //imageCanvas->setPivot(0,offsetY);
    imageCanvas->pushRotated(canvas,0);

    //dirty=true;
}
