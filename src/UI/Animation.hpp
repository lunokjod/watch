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

#ifndef __LUNOKIOT__ANIMATION__
#define __LUNOKIOT__ANIMATION__

/*
 * https://www.geeksforgeeks.org/2d-transformation-rotation-objects/ READ THIS!!!
 * https://www.geeksforgeeks.org/2d-transformation-in-computer-graphics-set-1-scaling-of-objects/?ref=lbp
 * https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/?ref=lbp
 * https://www.geeksforgeeks.org/mid-point-circle-drawing-algorithm/?ref=lbp
 */ 
#include <Arduino.h>

#ifdef LILYGO_DEV
#include <LilyGoWatch.h>
#endif
//M5_DEV

#include "../lunokiot_config.hpp"

class AnimationDescriptor {
    public:
        bool done = false;
        int32_t fromX = 0;
        int32_t fromY = 0;
        int32_t toX = 0;
        int32_t toY = 0;
        int32_t currentX = 0;
        int32_t currentY = 0;
        int32_t stepSize = 5;

        AnimationDescriptor(int32_t fromX,int32_t fromY,int32_t toX, int32_t toY, int32_t stepSize);
        void Step();
        void Draw();
};

#endif
