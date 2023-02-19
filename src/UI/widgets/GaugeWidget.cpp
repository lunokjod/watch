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

#include "GaugeWidget.hpp"
#include "CanvasWidget.hpp"
#include "../UI.hpp"

bool StaticGaugeWidgetCallback(int x,int y, int cx, int cy, int angle, int step, void* payload) {
    // recover the "this" context
    GaugeWidget * obj = ( GaugeWidget* )payload;
    //@TODO move this code to internal Guge method
    int32_t circleRadius = (obj->canvas->height()+obj->canvas->width())/4;
    int32_t holeRadius = circleRadius/2;
    int32_t markheight = holeRadius/2;
    // draw the mark
    if ( angle < obj->selectedAngle ) {
        obj->buffer->canvas->fillCircle(x,y,markheight*0.5,obj->incrementColor);
    }
    // set the values for needle
    if ( angle == obj->selectedAngle ) {
        obj->markX=x;
        obj->markY=y;
    }
    return true;
}


GaugeWidget::GaugeWidget(int16_t x, int16_t y, int16_t d, uint32_t wgaugeColor, uint32_t markColor, uint32_t needleColor, uint32_t incrementColor) 
        : CanvasWidget(d,d),ActiveRect(x,y,d,d, nullptr),gaugeColor(wgaugeColor),markColor(markColor),needleColor(needleColor), incrementColor(incrementColor) {
    buffer = new CanvasWidget(d,d);
    ForceDraw();
}

void GaugeWidget::ForceDraw() {
    // draw!
    canvas->fillSprite(CanvasWidget::MASK_COLOR); // clean final image
    buffer->canvas->fillSprite(CanvasWidget::MASK_COLOR);

    int32_t cx = h/2;
    int32_t cy = w/2;
    int32_t circleRadius = (h+w)/4;
    int32_t holeRadius = circleRadius/2;
    int32_t markheight = holeRadius/2;
    // solve glitch on first redraw
    if ( -1 == markX ) { markX = cx; }
    if ( -1 == markY ) { markY = markheight; }
    
    buffer->canvas->fillCircle(cx,cy,circleRadius,gaugeColor);
    buffer->canvas->fillCircle(cx,markheight,markheight,markColor);
    DescribeCircle(cx,cy,circleRadius-markheight, StaticGaugeWidgetCallback,(void *)this);
    // tap
    buffer->canvas->fillCircle(markX,markY,markheight-1,needleColor);
    buffer->canvas->fillCircle(markX,markY,markheight*0.6,canvas->alphaBlend(64,needleColor,TFT_WHITE));
    buffer->canvas->drawCircle(markX,markY,markheight,TFT_BLACK);

    buffer->canvas->fillCircle(cx,cy,holeRadius,CanvasWidget::MASK_COLOR); // center circle
    buffer->DrawTo(canvas);
}
bool GaugeWidget::Interact(bool touch, int16_t tx,int16_t ty, int16_t margin) {
    if ( !touch ) { return false; }
    if ( !InRadius(tx,ty,x,y,((h+w)/2)+margin) ) { return false; }

    // obtain vector from center
    int16_t tdvX = tx-(x+((h+w)/4)); // this sprite is rectangular
    int16_t tdvY = ty-(y+((h+w)/4)); // don't care H or W
 
    // Obtain the angle from vector in degrees
    double radAngle = atan2(tdvY, tdvX);
    int16_t degAngle = radAngle * (180.0/3.14);
    int16_t nuSelectedAngle = (180+degAngle)-90;
    if ( nuSelectedAngle < 0 ) { nuSelectedAngle = 360-abs(nuSelectedAngle); } // ugly :(
    //Serial.printf("ANGLE: DEG: %d\n", nuSelectedAngle);
    if ( nuSelectedAngle != selectedAngle ) {
        selectedAngle = nuSelectedAngle;
        dirty = true;
    }
    return true;
}
GaugeWidget::~GaugeWidget() {
    delete buffer;
}

void GaugeWidget::DirectDraw() {
    if ( dirty ) {
        ForceDraw();
        dirty = false;
        if ( lastAngle == selectedAngle ) { return; }
        buffer->canvas->pushSprite(x,y,Drawable::MASK_COLOR);
        lastAngle = selectedAngle;
    }
}

void GaugeWidget::DrawTo(TFT_eSprite * endCanvas) {
    if ( dirty ) { 
        ForceDraw();
        dirty = false;
    }
    CanvasWidget::DrawTo(endCanvas, x, y);
}
