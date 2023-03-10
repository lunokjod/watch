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
#include "lunokiot_config.hpp"
#include "SwitchWidget.hpp"

#include "../../app/LogView.hpp"

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include <functional>
SwitchWidget::~SwitchWidget() {
    delete buffer;
}
SwitchWidget::SwitchWidget(int16_t x, int16_t y, UICallback notifyTo, uint32_t switchColor)
                : ActiveRect(x,y,64,64,notifyTo), CanvasWidget(64,64), switchColor(switchColor) {
    lUIDeepLog("%s new %p\n",__PRETTY_FUNCTION__,this);

    buffer = new CanvasWidget(64,64);
    InternalRedraw();
}
void SwitchWidget::InternalRedraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    buffer->canvas->fillSprite(CanvasWidget::MASK_COLOR);
    // clean old redraws
    buffer->canvas->fillCircle(64-20,32,switchHeight,ThCol(background));
    buffer->canvas->fillCircle(20,32,switchHeight,ThCol(background));
    // base
    int16_t border = 2;
    buffer->canvas->fillRoundRect(0,22,64,20,10,ThCol(dark));
    buffer->canvas->fillRoundRect(border,22+border,64-(border*2),20-(border*2),8, switchColor);
    const int32_t dotSize = 8;

    int16_t baseColor = ThCol(darken);
    int16_t dotColor = ThCol(shadow);
    if ( GetEnabled() ) {
        baseColor = ThCol(highlight);
        dotColor = ThCol(light);
    }
    
    if ( switchEnabled ) {
        buffer->canvas->fillCircle(64-20,32,switchHeight,ThCol(shadow));
        uint32_t moreIntenseColor = canvas->alphaBlend(192,baseColor,switchColor);
        buffer->canvas->fillCircle(64-20,32,switchHeight-border,moreIntenseColor);
        buffer->canvas->fillCircle(64-20,32,dotSize,dotColor); // decorative dot remarks true
    } else {
        buffer->canvas->fillCircle(20,32,switchHeight,ThCol(dark));
        uint32_t moreIntenseColor = canvas->alphaBlend(64,baseColor,switchColor);
        buffer->canvas->fillCircle(20,32,switchHeight-border,moreIntenseColor);
        buffer->canvas->fillCircle(20,32,dotSize,dotColor); // decorative dot remarks false
    }
    buffer->canvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
}
void SwitchWidget::DirectDraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    buffer->canvas->pushSprite(x,y,Drawable::MASK_COLOR);
}

void SwitchWidget::DrawTo(TFT_eSprite * endCanvas) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    if ( nullptr != endCanvas ) { CanvasWidget::DrawTo(endCanvas,x,y); }
}
void SwitchWidget::SetEnabled(bool enabled) {
    ActiveRect::SetEnabled(enabled);
    InternalRedraw();
}

bool SwitchWidget::Interact(bool touch, int16_t tx,int16_t ty) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    bool interact = ActiveRect::Interact(touch,tx, ty);
    if (interact != lastInteract ) {
        if ( lastInteract ) { switchEnabled = (!switchEnabled); }
        lastInteract = interact;
        InternalRedraw();
        DirectDraw();
    }
    return interact;
}
