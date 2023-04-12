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

void Button::Refresh(bool direct) {
    lLog("Button %p refresh canvas dirty: %s direct: %s\n",this,(dirty?"true":"false"),(direct?"true":"false"));
    // I'm not dirty, but called refresh, send the refresh to children trying to resolve
    if ( false == dirty ) {
        Container::Refresh(direct); // notify to childs
        return;

    }
    // redraw me, I'm dirty
    Control::Refresh();

    if ( decorations ) {
        // draw button here!
        const int32_t radius = 8;
        uint16_t finalColor = color;
            
        // bright
        uint16_t brightColor = tft->alphaBlend(64,finalColor,TFT_WHITE);
        // shadow
        uint16_t shadowColor = tft->alphaBlend(64,finalColor,TFT_BLACK);

        // swap colors if needed
        /*
        if ( false == direct ) {
            finalColor = ByteSwap(finalColor);
            brightColor = ByteSwap(brightColor);
            shadowColor = ByteSwap(shadowColor);
        }*/

        // bright
        canvas->drawRoundRect( 0, 0,canvas->width()-1,canvas->height()-1,radius,brightColor);
        // shadow
        canvas->drawRoundRect( 1, 1,canvas->width()-1,canvas->height()-1,radius,shadowColor);
        // face
        uint16_t faceColor = finalColor;
        // bright when pressed
        if ( lastTouched ) { faceColor=tft->alphaBlend(192,finalColor,TFT_WHITE); }
        canvas->fillRoundRect(1,1,canvas->width()-2,canvas->height()-2,radius,faceColor);
    } else { // no decoration, fill with color?
        //canvas->fillSprite(color);
    }
    // Get children appearance, don't allow to push direct
    if ( direct ) {
        Container::Refresh(false);
        canvas->pushSprite(clipX,clipY, Drawable::MASK_COLOR);
        return;
    }
    Container::Refresh(direct);
}

Button::Button(LuI_Layout layout, size_t childs,bool decorations,uint16_t color): Container(layout,childs),decorations(decorations),color(color) {
    // a little stylished borders if decorations are enabled
    if ( decorations ) { border=5; }
}

void Button::EventHandler() {
    // I'm a button, react about events (tapCallback mostly)
    Control::EventHandler();
    // Butt... I'm also a container, 'share the word' x'D with children :)
    Container::EventHandler();
}