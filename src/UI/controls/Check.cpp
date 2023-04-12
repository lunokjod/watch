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

#include "Check.hpp"
#include "../../app/LogView.hpp"
#include "../UI.hpp"

using namespace LuI;

Check::~Check() {
    lLog("Check %p destroyed!\n",this);
}

Check::Check() {
    lLog("Created Check on %p\n",this);
    // enable tap callback
    tapCallbackParam=this; // what pass to callback
    tapCallback = [](void * obj){ // where call when tap
        Check * self=(Check*)obj;
        self->Switch();
    };
}

void Check::SetCheck(bool check) {
    if ( checked != check ) {
        checked=check;
        dirty=true;
    }
}

void Check::Refresh(bool direct) {
    //lLog("Check %p Refresh \n",this);
    if ( false == dirty ) { return; }
    Control::Refresh();

    //centered
    int32_t centerX=canvas->width()/2;
    int32_t centerY=canvas->height()/2;
    canvas->setPivot(centerX,centerY);
    //const int32_t radius = min(centerX,centerY)/2;
    const int32_t radius = 16;
    
    uint16_t baseColor = ThCol(button);
    uint16_t brightColor = tft->alphaBlend(128,TFT_WHITE,baseColor);
    uint16_t brightLightColor = tft->alphaBlend(128,TFT_WHITE,baseColor);
    uint16_t darkColor = tft->alphaBlend(64,TFT_BLACK,baseColor);
    uint16_t backColor = tft->alphaBlend(64,TFT_BLACK,baseColor);
    uint16_t shadowColor = ThCol(shadow);
    uint16_t highlightColor = ThCol(highlight);
    uint16_t highlightBrightColor = tft->alphaBlend(128,TFT_WHITE,highlightColor);
    uint16_t highlightShadowColor = tft->alphaBlend(128,TFT_BLACK,highlightColor);
    uint16_t markColor = ThCol(mark);
    /*
    if (( false == directDraw ) && ( true == swap )) {
        baseColor = ByteSwap(baseColor);
        brightColor = ByteSwap(brightColor);
        brightLightColor = ByteSwap(brightLightColor);
        darkColor = ByteSwap(darkColor);
        backColor = ByteSwap(backColor);
        shadowColor = ByteSwap(shadowColor);
        highlightColor = ByteSwap(highlightColor);
        highlightBrightColor = ByteSwap(highlightBrightColor);
        highlightShadowColor = ByteSwap(highlightShadowColor);
        
        markColor = ByteSwap(markColor);
    }*/


    // tint over the background
    for(int i=(centerX-radius);i<(centerX+radius);i++) {
        canvas->fillCircle(i,centerY,radius+1,TFT_BLACK);
    }
    for(int i=centerX-radius;i<centerX+radius;i++) {
        canvas->fillCircle(i,centerY,radius,backColor);
    }
    // draw "mecanism"
    if ( false == checked ) { shadowColor=brightColor; }
    canvas->fillRoundRect(centerX-radius,centerY-(radius/2),radius*2,radius,radius/2,shadowColor);
    canvas->drawRoundRect(centerX-radius,centerY-(radius/2),radius*2,radius,radius/2,TFT_BLACK);
    // draw new check
    if ( checked ) {
        canvas->drawCircle((centerX-radius)-1,centerY-1,radius-1,brightLightColor);
        canvas->drawCircle((centerX-radius)+1,centerY+1,radius-1,darkColor);
        canvas->fillCircle(centerX-radius,centerY,radius-1,brightColor);

        canvas->drawCircle((centerX-radius)-1,centerY-1,radius/2,highlightShadowColor);
        canvas->drawCircle((centerX-radius)+1,centerY+1,radius/2,highlightBrightColor);
        canvas->fillCircle(centerX-radius,centerY,radius/2,highlightColor);
        //canvas->drawCircle(centerX-radius,centerY,(radius/2)+1,TFT_BLACK);

    } else {
        canvas->drawCircle(centerX+radius,centerY,radius,TFT_BLACK);
        canvas->drawCircle(centerX+radius,centerY,radius,TFT_BLACK);
        canvas->fillCircle(centerX+radius,centerY,radius,darkColor);
        canvas->fillCircle(centerX+radius,centerY,radius/2,markColor);
    }
    /*
    // base
    int16_t border = 2;
    canvas->fillRoundRect(0,22,64,20,10,ThCol(dark));
    canvas->fillRoundRect(border,22+border,64-(border*2),20-(border*2),8, ThCol(button));
    const int32_t dotSize = 8;

    int16_t baseColor = ThCol(darken);
    int16_t dotColor = ThCol(shadow);
    if ( touchEnabled ) {
        baseColor = ThCol(highlight);
        dotColor = ThCol(light);
    }
    
    if ( checked ) {
        canvas->fillCircle(64-20,32,width,ThCol(shadow));
        uint32_t moreIntenseColor = canvas->alphaBlend(192,baseColor,ThCol(button));
        canvas->fillCircle(64-20,32,height-border,moreIntenseColor);
        canvas->fillCircle(64-20,32,dotSize,dotColor); // decorative dot remarks true
    } else {
        canvas->fillCircle(20,32,height,ThCol(dark));
        uint32_t moreIntenseColor = canvas->alphaBlend(64,baseColor,ThCol(button));
        canvas->fillCircle(20,32,height-border,moreIntenseColor);
        canvas->fillCircle(20,32,dotSize,dotColor); // decorative dot remarks false
    }
    */
   if ( direct ) { canvas->pushSprite(clipX,clipY,Drawable::MASK_COLOR); }
}
/*
// must be called to call my ::Refresh()
void Check::EventHandler() {
    Control::EventHandler();
}
*/