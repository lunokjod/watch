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
#include "ScrollViewWidget.hpp"

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include <functional>

ScrollViewWidget::~ScrollViewWidget() {
}
ScrollViewWidget::ScrollViewWidget(int16_t x, int16_t y, int16_t h, int16_t w, uint32_t btnBackgroundColor, CanvasWidget *view)
                : ActiveRect(x,y,h,w,[&,this](void *bah){
                    offsetX+=touchDragVectorX;
                    offsetY+=touchDragVectorY;
//        Serial.printf("X: %d Y: %d Dist: %f VecX: %d VecY: %d\n",touchX, touchY,touchDragDistance,touchDragVectorX,touchDragVectorY);
                }), CanvasWidget(h,w), btnBackgroundColor(btnBackgroundColor),backroundView(view) {
    _img = new CanvasWidget(h,w);

}
/*
bool ScrollViewWidget::CheckBounds() {
    if ( nullptr == canvas) { return false; }

    if ( ( ActiveRect::w != canvas->width() ) || ( ActiveRect::h != canvas->height() ) ) {
        Serial.printf("ButtonWidget: New size for %p requested: Regenerating canvas...\n", this);
        RebuildCanvas(ActiveRect::h,ActiveRect::w);
        return true;
    }
    return false;
}
*/
void ScrollViewWidget::DrawTo(TFT_eSprite * endCanvas) {
    if ( nullptr == canvas) { return; }
    uint32_t buttonColor = btnBackgroundColor;
    const uint8_t radius = 30;
    const uint8_t border = 3;
    _img->canvas->fillSprite(btnBackgroundColor);
    _img->canvas->fillRoundRect( 0, 0,ActiveRect::w,ActiveRect::h,radius,CanvasWidget::MASK_COLOR);

    //uint16_t lightColor = canvas->alphaBlend(128,btnBackgroundColor,TFT_WHITE);
    //canvas->fillSprite(lightColor);
    canvas->fillSprite(canvas->color24to16(0x555f68));
    backroundView->DrawTo(canvas,offsetX,offsetY);
    /*
    if ( borders ) {
        // why 0? remember, every object has their own canvas, their coordinates are relative to their own canvas :)
        _img->canvas->fillRoundRect( 0, 0,ActiveRect::w,ActiveRect::h,radius,TFT_BLACK);
        _img->canvas->fillRoundRect( border, border,(ActiveRect::w-(border*2)),(ActiveRect::h-(border*2)),radius,buttonColor);
    } else {
        _img->canvas->fillRoundRect( 0, 0,ActiveRect::w,ActiveRect::h,radius,CanvasWidget::MASK_COLOR);
    }
    */
    _img->canvas->pushRotated(canvas,0,CanvasWidget::MASK_COLOR);

    // normal draw
    if ( nullptr != endCanvas ) {
        CanvasWidget::DrawTo(endCanvas,x,y);
    }
}