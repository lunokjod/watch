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

#include "../../lunokIoT.hpp"
#include "base/Control.hpp"
#include "base/Container.hpp"

#include "Button.hpp"
#include "../../app/LogView.hpp"
#include "../UI.hpp"
using namespace LuI;

void Button::Refresh(bool swap) {
    Control::Refresh(swap);
    // draw button here!
    const int32_t radius = 8;
    uint16_t finalColor = color;
        
    // bright
    uint16_t brightColor = canvas->alphaBlend(64,finalColor,TFT_WHITE);
    // shadow
    uint16_t shadowColor = canvas->alphaBlend(64,finalColor,TFT_BLACK);

    //if ( false == directDraw ) {
    if (( false == directDraw ) && ( swap )) {
        //finalColor = ColorSwap(finalColor);
        //brightColor = ColorSwap(brightColor);
        //shadowColor = ColorSwap(shadowColor);
        finalColor = ByteSwap(finalColor);
        brightColor = ByteSwap(brightColor);
        shadowColor = ByteSwap(shadowColor);
    }

    // bright
    canvas->drawRoundRect( 0, 0,canvas->width()-1,canvas->height()-1,radius,brightColor);
    // shadow
    canvas->drawRoundRect( 1, 1,canvas->width()-1,canvas->height()-1,radius,shadowColor);
    // face
    uint16_t faceColor = finalColor;
    // bright when pressed
    if ( lastTouched ) { faceColor=canvas->alphaBlend(192,finalColor,TFT_WHITE); }
    canvas->fillRoundRect(1,1,canvas->width()-2,canvas->height()-2,radius,faceColor);

    Container::Refresh(swap);
}

Button::Button(LuI_Layout layout, size_t childs,uint16_t color): Container(layout,childs),color(color) {
    border=5;
    touchEnabled=true;
}
