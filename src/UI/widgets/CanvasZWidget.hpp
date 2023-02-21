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

#ifndef __LUNOKIOT__CANVAS__Z_WIDGET___HEADER__
#define __LUNOKIOT__CANVAS__Z_WIDGET___HEADER__
#include <Arduino.h>
//#include <LilyGoWatch.h>
#include <libraries/TFT_eSPI/TFT_eSPI.h>

#include "lunokiot_config.hpp"
#include "CanvasWidget.hpp"


class CanvasZWidget : public CanvasWidget {
    public:
        TFT_eSprite * GetScaledImage(float scale);
        TFT_eSprite * GetScaledImageClipped(float scale,int16_t maxW, int16_t maxH);
        CanvasZWidget(int16_t h=0, int16_t w=0, float z=1.0, int8_t colorDeepth=16);
        void DrawTo(int16_t x=0, int16_t y=0, float z=1.0, bool centered=false, int32_t maskColor=MASK_COLOR);
        void DrawTo(TFT_eSprite * endCanvas=nullptr, int16_t x=0, int16_t y=0, float z=1.0, bool centered=false, int32_t maskColor=MASK_COLOR);
        virtual ~CanvasZWidget();
        bool SetScale(float z=1.0);
        float GetScale() { return z; } // read only, use draw to set again
    protected:
        float z=1; // don't touch this value direcly, only for read (@TODO getter and private)
    /*
        static const uint32_t MASK_COLOR=0x100; // ttgo->tft->color565(0x01,0x20,0x00)
        TFT_eSprite *canvas = nullptr;
        unsigned long lastRefresh=0;
        // BRG
        void RebuildCanvas(int16_t h=0, int16_t w=0);
        TFT_eSprite *canvas;
        */
};

#endif
