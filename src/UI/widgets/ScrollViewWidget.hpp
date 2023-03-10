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

#ifndef __LUNOKIOT__UI__SCROLLVIEW___
#define __LUNOKIOT__UI__SCROLLVIEW___
//ScrollViewWidget.hpp
#include <Arduino.h>

#include "lunokiot_config.hpp"

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include <functional>

class ScrollViewWidget: public ActiveRect, public CanvasWidget {
    public:
        uint32_t offsetX=0;
        uint32_t offsetY=0;
        uint32_t btnBackgroundColor;
        virtual ~ScrollViewWidget();
        void BuildCanvas(int16_t h, int16_t w);
        //bool CheckBounds();
        CanvasWidget * backroundView;
        CanvasWidget *_img;
        ScrollViewWidget(int16_t x,int16_t y, int16_t h, int16_t w, uint32_t btnBackgroundColor, CanvasWidget *view);
        void DrawTo(TFT_eSprite * endCanvas);
};
#endif
