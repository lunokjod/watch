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

#include "Paginator.hpp"
#include "../../app/LogView.hpp"
#include "../UI.hpp"

using namespace LuI;

Paginator::Paginator(IN uint8_t pages): pages(pages) {
}

void Paginator::Refresh(bool direct) {
    if ( false == dirty ) { return; }
    lLog("Paginator %p refresh dirty: %s direct: %s\n",this,(dirty?"true":"false"),(direct?"true":"false"));
    Control::Refresh();
    uint16_t backColor = Drawable::MASK_COLOR;
    if ( useBackground ) { backColor=backgroundColor; }
    canvas->fillSprite(backColor);
    const uint8_t dotRad = 5;
    const int16_t separator=(width-(border*2))/pages;
    for(int pn=0;pn<=pages;pn++) {
        if ( pn < current ) { canvas->drawCircle(border+(pn*separator),height/2,dotRad,TFT_DARKGREY); }
        else if ( pn >= current ) { canvas->fillCircle(border+(pn*separator),height/2,dotRad,TFT_DARKGREY); }
    }
    canvas->fillCircle(border+(current*separator),height/2,dotRad,TFT_WHITE);
    if ( direct ) {
        canvas->pushSprite(clipX,clipY,Drawable::MASK_COLOR);
    }
}
