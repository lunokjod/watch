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

#ifndef __LUNOKIOT__WIDGET__BASE__
#define __LUNOKIOT__WIDGET__BASE__

#include <LilyGoWatch.h>

// https://github.com/Bodmer/TFT_eSPI/blob/master/examples/Sprite/Orrery/Orrery.ino
class Drawable {
    public:
        static const uint16_t MASK_COLOR=0x0100; // RGB565 0x0100 //RGB888 0x002000
        //static const uint32_t MASK_COLOR2=tft->color565(0x01,0x20,0x00);
        virtual void DirectDraw(int16_t x,int16_t y)=0;
        virtual void DrawTo(TFT_eSprite * endCanvas, int16_t x=0, int16_t y=0,uint16_t maskColor=MASK_COLOR)=0;
};

#endif
