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

#ifndef __LUNOKIOT__UI__VALUE_SELECTOR___
#define __LUNOKIOT__UI__VALUE_SELECTOR___

#include <Arduino.h>
#include "lunokiot_config.hpp"

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include "../UI.hpp"

class ValueSelector: public ActiveRect, public CanvasWidget {
    public:
        bool showsign;
        unsigned long nextDragStep=0;
        int16_t x;
        int16_t y;
        uint32_t valStep=1;
        int32_t stepSize=0;
        int32_t valMin;
        int32_t valMax;
        int32_t selectedValue=0;
        uint32_t backgroundColor;
        uint32_t textColor=ThCol(text);
        CanvasWidget * buffer=nullptr;
        virtual ~ValueSelector();
        virtual void InternalRedraw();
        virtual void DirectDraw();
        bool Interact(bool touch, int16_t tx,int16_t ty);
        ValueSelector(int16_t x,int16_t y, int16_t h, int16_t w,int32_t valMin=0,int32_t valMax=100,uint32_t backgroundColor=ThCol(background),bool showsign=false,uint32_t valStep=1);
        void DrawTo(TFT_eSprite * endCanvas);
};
typedef class ValueSelector MujiValue; // thanks man! :-*

#endif
