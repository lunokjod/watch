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
#include <libraries/TFT_eSPI/TFT_eSPI.h>

//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include <functional>
#include "../base/Widget.hpp"
//#include "lunokiot_config.hpp"
#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include "../../app/LogView.hpp"
#include "../UI.hpp"
#include "ButtonWidget.hpp"

ButtonWidget::~ButtonWidget() {
    delete buffer;
    lUIDeepLog("%s delete %p\n",__PRETTY_FUNCTION__,this);
}
void ButtonWidget::DirectDraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    buffer->canvas->pushSprite(x,y,Drawable::MASK_COLOR);
}

bool ButtonWidget::Interact(bool touch, int16_t tx,int16_t ty) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    if ( 0.0 != touchDragDistance ) { return false; } // only tap
    bool response = ActiveRect::Interact(touch,tx,ty);
    if ( pushed != lastPushed ) {
        InternalRedraw();
        DirectDraw();
        lastPushed=pushed;
    }
    return response;
}

ButtonWidget::ButtonWidget(int16_t x, int16_t y, int16_t h, int16_t w, UICallback notifyTo, uint32_t btnBackgroundColor, bool borders)
                : ActiveRect(x,y,h,w,notifyTo), CanvasWidget(h,w), btnBackgroundColor(btnBackgroundColor), borders(borders) {
    lUIDeepLog("%s new %p\n",__PRETTY_FUNCTION__,this);
    buffer = new CanvasWidget(h,w);
    InternalRedraw();
}

bool ButtonWidget::CheckBounds() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    if ( ( ActiveRect::w != canvas->width() ) || ( ActiveRect::h != canvas->height() ) ) {
        lUILog("ButtonWidget: New size for %p requested: Regenerating canvas...\n", this);
        RebuildCanvas(ActiveRect::h,ActiveRect::w);
        return true;
    }
    return false;
}

void ButtonWidget::InternalRedraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    CheckBounds();
    canvas->fillSprite(Drawable::MASK_COLOR); // prepare the color-swap buffer
    buffer->canvas->fillSprite(Drawable::MASK_COLOR); // prepare the color-swap buffer
    const int32_t radius = 8;
    int32_t border = 3;
    uint32_t buttonColor = btnBackgroundColor;
    if ( borders ) {
        // why 0? remember, every object has their own canvas, their coordinates are relative to their own canvas :)
        buffer->canvas->fillRoundRect( 0, 0,ActiveRect::w,ActiveRect::h,radius,ThCol(dark));
        if ( false == enabled ) { border = 4; }
        if ( false == enabled ) { buttonColor = ThCol(darken); }
        buffer->canvas->fillRoundRect( border, border,(ActiveRect::w-(border*2)),(ActiveRect::h-(border*2)),radius,buttonColor);
    }
    if ( ( pushed )&&( borders )) {
        border = 6;
        uint16_t lightColor = canvas->alphaBlend(128,buttonColor,ThCol(light));
        buffer->canvas->fillRoundRect( border, border,(ActiveRect::w-(border*2)),(ActiveRect::h-(border*2)),radius,lightColor);
    }
    buffer->canvas->setPivot(0,0);
    canvas->setPivot(0,0);
    buffer->canvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
}

void ButtonWidget::DrawTo(TFT_eSprite * endCanvas) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    if ( nullptr != endCanvas ) { CanvasWidget::DrawTo(endCanvas,x,y); }
}
