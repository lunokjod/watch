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

#include <Arduino.h>
#include <LilyGoWatch.h>

#include "ValueSelector.hpp"
#include "../../app/LogView.hpp"

ValueSelector::~ValueSelector() {
    delete buffer;
}

ValueSelector::ValueSelector(int16_t x,int16_t y, int16_t h, int16_t w,int32_t valMin,int32_t valMax,uint32_t backgroundColor, bool showsign,uint32_t valStep):
            CanvasWidget(h,w),x(x),y(y),
            ActiveRect(x,y,h,w),
            valMin(valMin),valMax(valMax),backgroundColor(backgroundColor),showsign(showsign),valStep(valStep) {
    buffer = new CanvasWidget(h,w);
    InternalRedraw();
}

bool ValueSelector::Interact(bool touch, int16_t tx,int16_t ty) {
    if ( millis() > nextDragStep ) {
        if ( stepSize != 0 ) {
            selectedValue+=stepSize; // up or down depend of value sign
            if ( selectedValue > valMax ) { selectedValue = valMax; }
            else if ( selectedValue < valMin ) { selectedValue = valMin; }
            //lUILog("Step size: %d sel: %d\n",stepSize,selectedValue);
            InternalRedraw();
            DirectDraw();
            stepSize/=2;
        }
        nextDragStep=millis()+5;
    }
    if ( touch ) {
        if ( ActiveRect::InRect(tx,ty,x,y,h,w)) {
            // basic up/down button functionality
            if ( ActiveRect::InRect(tx,ty,x,y,h/2,w)) {
                stepSize=-valStep;
            } else {
                stepSize=valStep;
            }
            // if user drags, use distance as stepSize
            if ( touchDragDistance > 0.0 ) {
                stepSize = touchDragDistance/10;
                if ( stepSize < 1 ) { stepSize=1; }
                else if ( stepSize > 20 ) { stepSize=20; }

                if ( touchDragAngle < 0 ) { stepSize*=-1; } // convert to negative (up direction)
            }
            //lUILog("TOUCH INSIDE!! DIST: %f step: %d\n", touchDragDistance,stepSize);
            return true;
        }
    } else {
        // check the value limits anyway
        if ( selectedValue > valMax ) { selectedValue = valMax; }
        else if ( selectedValue < valMin ) { selectedValue = valMin; }
    }
    return false;
}


void ValueSelector::DirectDraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    //buffer->canvas->fillSprite(random(0,0xffff));
    buffer->canvas->pushSprite(x,y,Drawable::MASK_COLOR);
}


void ValueSelector::InternalRedraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);



    buffer->canvas->fillSprite(backgroundColor);

    buffer->canvas->setTextFont(0);
    buffer->canvas->setTextWrap(false,false);
    char textBuffer[14] = { 0 }; // more than 2^32

    buffer->canvas->setTextSize(3);
    buffer->canvas->setTextColor(canvas->alphaBlend(128,textColor,ThCol(shadow)));

    // descendants
    buffer->canvas->setTextDatum(BC_DATUM);
    int posY = (buffer->canvas->height()/2)-20;
    int posX = buffer->canvas->width()/2;
    int32_t currVal = selectedValue-1;
    while ( posY > 25 ) {
        if ( currVal < valMin ) { break; }
        //char * showsignString = (char *)"";
        //if ( showsign ) { if ( currVal > 0 ) { showsignString=(char *)"+"; } }        
        if ( showsign ) {
            sprintf(textBuffer, "%+d", currVal);
        } else {
            sprintf(textBuffer,"%d", currVal);
        }
        buffer->canvas->drawString(textBuffer, posX, posY);
        currVal--;
        posY-=25;
    }

    // ascendants
    buffer->canvas->setTextDatum(TC_DATUM);
    posY = (buffer->canvas->height()/2)+20;
    currVal = selectedValue+1;
    while ( posY < canvas->height()-25 ) {
        if ( currVal > valMax ) { break; }
        char * showsignString = (char *)"";
        if ( showsign ) { if ( currVal > 0 ) { showsignString=(char *)"+"; } }
        sprintf(textBuffer,"%s%d", showsignString, currVal);
        buffer->canvas->drawString(textBuffer, posX, posY);
        currVal++;
        posY+=25;
    }

    // dissolve effect upper
    int16_t alpha = 255;
    for(int y=(h/2)-22;y>0;y--) {
        for(int x=0;x<canvas->width();x++) {
            uint16_t color = buffer->canvas->readPixel(x,y);
            uint16_t mixColor = canvas->alphaBlend(alpha,color,backgroundColor);
            buffer->canvas->drawPixel(x,y,mixColor);
        }
        alpha-=5;
        if ( alpha < 0 ) { alpha=0; }
    }

    // dissolve effect lower
    alpha = 255;
    for(int y=(h/2)+20;y<h;y++) {
        for(int x=0;x<canvas->width();x++) {
            uint16_t color = buffer->canvas->readPixel(x,y);
            uint16_t mixColor = canvas->alphaBlend(alpha,color,backgroundColor);
            buffer->canvas->drawPixel(x,y,mixColor);
        }
        alpha-=5;
        if ( alpha < 0 ) { alpha=0; }
    }

    // current value
    posX = buffer->canvas->width()/2;
    posY = buffer->canvas->height()/2;
    buffer->canvas->setTextSize(4);
    buffer->canvas->setTextDatum(CC_DATUM);
    char * showsignString = (char *)"";
    if ( showsign ) { if ( selectedValue > 0 ) { showsignString=(char *)"+"; } }
    sprintf(textBuffer,"%s%d", showsignString, selectedValue);
    //sprintf(textBuffer,"%d", selectedValue);
    buffer->canvas->setTextColor(ThCol(shadow));
    buffer->canvas->drawString(textBuffer, posX+2, posY+2);

    buffer->canvas->setTextColor(textColor);
    buffer->canvas->drawString(textBuffer, posX, posY);
    buffer->DrawTo(canvas);
}

void ValueSelector::DrawTo(TFT_eSprite * endCanvas) {
    if ( nullptr != endCanvas ) {
        CanvasWidget::DrawTo(endCanvas,x,y);
    }
}