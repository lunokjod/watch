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
#include "Playground9.hpp"
#include "../UI/UI.hpp"
extern TTGOClass *ttgo; // ttgo library shit ;)
PlaygroundApplication9::~PlaygroundApplication9() {
    
}


PlaygroundApplication9::PlaygroundApplication9() {
    canvas->deleteSprite();
    canvas->setColorDepth(4);
    canvas->createSprite(TFT_WIDTH,TFT_HEIGHT);
    //colorMap[1] = canvas->color565(255,0,0);
    colorMap[0] = canvas->color24to16(0x00022b);
    colorMap[1] = canvas->color24to16(0x020831);
    colorMap[2] = canvas->color24to16(0x060f39);
    colorMap[3] = canvas->color24to16(0x081440);
    colorMap[4] = canvas->color24to16(0x0b1b47);
    colorMap[5] = canvas->color24to16(0x0e204e);
    colorMap[6] = canvas->color24to16(0x102654);
    colorMap[7] = canvas->color24to16(0x142d5d);
    colorMap[8] = canvas->color24to16(0x173264);
    colorMap[9] = canvas->color24to16(0x1f4379);
    colorMap[10] = canvas->color24to16(0x28568e);
    colorMap[11] = canvas->color24to16(0x2b6da4);
    colorMap[12] = canvas->color24to16(0x1b9fc2);
    colorMap[13] = canvas->color24to16(0x0ecadd);
    colorMap[14] = canvas->color24to16(0x0ecbdc);
    colorMap[15] = canvas->color24to16(0x00f7f7);
    canvas->createPalette(colorMap,16);
    canvas->fillSprite(1);
    ttgo->setBrightness(32);

/*    
    uint8_t R = random(0,255);
    uint8_t G = random(0,255);
    uint8_t B = random(0,255);
    canvas->setPaletteColor(1,canvas->color565(R,G,B));
    colorOffset++;
    if ( colorOffset > 15 ) { colorOffset = 0; }
    */
    
}
bool PlaygroundApplication9::Tick() {
    
    uint16_t lastColor = canvas->getPaletteColor(0);
    for(int c=0;c<15;c++) {
        uint16_t nextColor = canvas->getPaletteColor(c+1);
        canvas->setPaletteColor(c,nextColor);
    }
    canvas->setPaletteColor(15,lastColor);
    
    if (millis() > nextRedraw ) {
        // draw code here
        int16_t x = random(0,TFT_WIDTH);
        int16_t y = random(0,TFT_HEIGHT);
        int off=random(0,15);
        int radMax=random(5,50);
        for(int r=radMax;r>1;r--) {
            canvas->fillCircle(x,y,r,colorMap[off]);
            off++;
            if(off>15) { off=0; }
        }
        nextRedraw=millis()+(1000/8); // tune your required refresh
        return true;
    }
    return false;
}