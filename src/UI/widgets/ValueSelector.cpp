#include <Arduino.h>
#include <LilyGoWatch.h>

#include "ValueSelector.hpp"

ValueSelector::~ValueSelector() {
    if ( nullptr != buffer ) { delete buffer; }
}

ValueSelector::ValueSelector(int16_t x,int16_t y, int16_t h, int16_t w,int32_t valMin,int32_t valMax,uint32_t backgroundColor, bool showsign):
            CanvasWidget(h,w),x(x),y(y),
            ActiveRect(x,y,h,w),
            valMin(valMin),valMax(valMax),backgroundColor(backgroundColor),showsign(showsign) {
    buffer = new CanvasWidget(h,w);
}

bool ValueSelector::Interact(bool touch, int16_t tx,int16_t ty) {
    if ( touch ) {
        if ( ActiveRect::InRect(tx,ty,x,y,h,w)) {
            //Serial.printf("TOUCH INSIDE!! DIST: %f ANGLE: %f\n", touchDragDistance, touchDragAngle);
            if ( ActiveRect::InRect(tx,ty,x,y,h/2,w)) {
                selectedValue--;
            } else {
                selectedValue++;
            }
            if ( selectedValue > valMax ) { selectedValue = valMax; }
            else if ( selectedValue < valMin ) { selectedValue = valMin; }
            return true;
        }
    } else {
        // check the value limits anyway
        if ( selectedValue > valMax ) { selectedValue = valMax; }
        else if ( selectedValue < valMin ) { selectedValue = valMin; }
    }
    return false;
}

void ValueSelector::DrawTo(TFT_eSprite * endCanvas) {
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
        char * showsignString = (char *)"";
        if ( showsign ) { if ( currVal > 0 ) { showsignString=(char *)"+"; } }        
        sprintf(textBuffer,"%s%d", showsignString, currVal);
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
    if ( nullptr != endCanvas ) {
        CanvasWidget::DrawTo(endCanvas,x,y);
    }
}