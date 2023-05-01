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

#ifndef __LUNOKIOT__CANVAS__WIDGET___HEADER__
#define __LUNOKIOT__CANVAS__WIDGET___HEADER__
//#include <Arduino.h>
//#include <LilyGoWatch.h>
#include <libraries/TFT_eSPI/TFT_eSPI.h>

//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include "../base/Widget.hpp"

class CanvasWidget : public Drawable {
    public:
        int8_t colorDepth=16;
        TFT_eSprite *canvas=nullptr; // must be nullptr on creation
        // BRG
        CanvasWidget(int16_t h, int16_t w, int8_t colorDepth=16);
        bool RebuildCanvas(int16_t h=0, int16_t w=0, int8_t colorDepth=16);
        void DirectDraw(int16_t x,int16_t y) override;
        void DrawTo(TFT_eSprite * endCanvas, int16_t x=0, int16_t y=0,uint16_t maskColor=MASK_COLOR) override;
        void DumpTo(TFT_eSprite * endCanvas, int16_t x=0, int16_t y=0, uint16_t maskColor=MASK_COLOR);
        void DumpAlphaTo(TFT_eSprite * endCanvas, int16_t x=0, int16_t y=0, uint8_t alpha=128, uint16_t maskColor=MASK_COLOR);
        virtual ~CanvasWidget();
};

#endif
