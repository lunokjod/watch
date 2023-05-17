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

#ifndef __LUNOKIOT__GRAPH__WIDGET___HEADER__
#define __LUNOKIOT__GRAPH__WIDGET___HEADER__
#include <Arduino.h>
#include "lunokiot_config.hpp"
#include "CanvasWidget.hpp"

class GraphWidget : public CanvasWidget{
    private:
    public:
        int64_t minValue;
        int64_t maxValue;
        uint32_t markColor;
        uint32_t backgroundColor;
        uint32_t outColor;
        bool inverted=true;
        int64_t lastValue;
        CanvasWidget *graph;
        GraphWidget(int16_t h=100, int16_t w=220, int64_t minValue=0, int64_t maxValue=100, uint32_t markColor=TFT_YELLOW, uint32_t backgroundColor=CanvasWidget::MASK_COLOR, uint32_t outColor=TFT_RED);
        bool PushValue(int64_t value);
        void DirectDraw(int16_t x,int16_t y) override;
        void DrawTo(TFT_eSprite * endCanvas, int16_t x=0, int16_t y=0);
        ~GraphWidget();
};

#endif
