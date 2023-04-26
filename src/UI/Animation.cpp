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

#include "Animation.hpp"
#include <LilyGoWatch.h>
extern TTGOClass *ttgo;

AnimationDescriptor::AnimationDescriptor(int32_t fromX,int32_t fromY,int32_t toX, int32_t toY, int32_t stepSize)
                            : fromX(fromX), fromY(fromY),toX(toX),toY(toY),stepSize(stepSize)  {
                                currentX = fromX;
                                currentY = fromY;
}

void AnimationDescriptor::Step() {
    Serial.printf("DEBUG Step: ");
    int32_t incrementX = 0;
    int32_t incrementY = 0;

    if ( currentX < toX ) {
        incrementX=stepSize;
    } else if ( currentX > toX ) {
        incrementX=(stepSize*-1);
    }

    if ( currentY < toY ) {
        incrementY=stepSize;
    } else if ( currentY > toY ) {
        incrementY=(stepSize*-1);
    }
    currentX+=incrementX;
    currentY+=incrementY;
    Serial.printf("incrementX: %d", incrementX);
    Serial.printf(" incrementY: %d\n", incrementY);

    Serial.printf("  CurrentX: %d", currentX);
    Serial.printf(" CurrentY: %d\n", currentY);
}

void AnimationDescriptor::Draw() {
    tft->fillRoundRect(currentX,currentY,35,35,5,0x5DFC);
}
