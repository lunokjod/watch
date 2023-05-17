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
#include "GraphWidget.hpp"
#include "../../app/LogView.hpp"

GraphWidget::GraphWidget(int16_t h, int16_t w, int64_t minValue, int64_t maxValue, uint32_t markColor, uint32_t backgroundColor, uint32_t outColor)
                : CanvasWidget(h,w), minValue(minValue),maxValue(maxValue),
                markColor(markColor),backgroundColor(backgroundColor), outColor(outColor) {
    graph = new CanvasWidget(h,w);
    graph->canvas->fillSprite(backgroundColor);
}

bool GraphWidget::PushValue(int64_t value) {
    if ( inverted ) {
        graph->canvas->scroll(1,0);
        if ( value > maxValue ) {
            //Serial.printf("UI: GraphWidget: %p Pushed value '%lld' is out of bounds > %lld\n", this, value, maxValue);
            graph->canvas->drawFastVLine(0,0,canvas->height(),outColor);
            return false;
        } else if ( value < minValue ) {
            //Serial.printf("UI: GraphWidget: %p Pushed value '%lld' is out of bounds < %lld\n", this, value, minValue);
            graph->canvas->drawFastVLine(0,0,canvas->height(),outColor);
            return false;
        }
        float range=maxValue-minValue;
        float correctedValue=value-minValue;
        int32_t pcValue = 0;
        if ( 0 == correctedValue ) { pcValue = 0; } // don't perform divide by zero x'D
        else if ( 0 == range ) { pcValue = 0; }     // don't perform divide by zero x'D
        else { pcValue = (correctedValue/range)*canvas->height(); }
        //lUILog("@DEBUG pixels: %d, val: %lld, corrected: %.2f, min: %lld, max: %lld, range: %.2f, canvas: %d\n",pcValue,value,correctedValue,minValue,maxValue,range,canvas->height());
        // bottom to top
        graph->canvas->drawFastVLine(0, 0, canvas->height(), backgroundColor);
        graph->canvas->drawFastVLine(0,canvas->height()-pcValue,pcValue,markColor);
    } else {
        graph->canvas->scroll(-1,0);
        if ( value > maxValue ) {
            //Serial.printf("UI: GraphWidget: %p Pushed value '%lld' is out of bounds > %lld\n", this, value, maxValue);
            graph->canvas->drawFastVLine(canvas->width()-1,0,canvas->height(),outColor);
            return false;
        } else if ( value < minValue ) {
            //Serial.printf("UI: GraphWidget: %p Pushed value '%lld' is out of bounds < %lld\n", this, value, minValue);
            graph->canvas->drawFastVLine(canvas->width()-1,0,canvas->height(),outColor);
            return false;
        }
        float range=maxValue-minValue;
        float correctedValue=value-minValue;
        int32_t pcValue = 0;
        if ( 0 == correctedValue ) { pcValue = 0; } // don't perform divide by zero x'D
        else if ( 0 == range ) { pcValue = 0; }     // don't perform divide by zero x'D
        else { pcValue = (correctedValue/range)*canvas->height(); }
        //lUILog("@DEBUG pixels: %d, val: %lld, corrected: %.2f, min: %lld, max: %lld, range: %.2f, canvas: %d\n",pcValue,value,correctedValue,minValue,maxValue,range,canvas->height());
        // bottom to top
        graph->canvas->drawFastVLine(canvas->width()-1, 0, canvas->height(), backgroundColor);
        graph->canvas->drawFastVLine(canvas->width()-1,canvas->height()-pcValue,pcValue,markColor);
    }


    lastValue = value;
    return true;
}


GraphWidget::~GraphWidget() {
    graph->canvas->deleteSprite();
    delete graph;
}


void GraphWidget::DirectDraw(int16_t x,int16_t y) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    graph->DirectDraw(x,y);
    //graph->ca->pushSprite(x,y,Drawable::MASK_COLOR);
}
void GraphWidget::DrawTo(TFT_eSprite * endCanvas, int16_t x, int16_t y) {
    canvas->fillSprite(backgroundColor);
    graph->DrawTo(canvas);
    CanvasWidget::DrawTo(endCanvas,x,y);
}
