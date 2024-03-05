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

#ifdef LILYGO_DEV
#include <LilyGoWatch.h>
#elif defined(M5_DEV)
#include <M5Core2.h>
#endif

//#include <libraries/TFT_eSPI/TFT_eSPI.h>
//extern TFT_eSPI *tft;
#include "lunokiot_config.hpp"
#include "ButtonImageXBMWidget.hpp"
#include "../../app/LogView.hpp"
#include "../../UI/UI.hpp"
#include "CanvasWidget.hpp"
#include <functional>

ButtonImageXBMWidget::ButtonImageXBMWidget(int16_t x, int16_t y, int16_t h, int16_t w, 
                                    UICallback notifyTo, 
                                    const uint8_t * XBMBitmapPtr,
                                    const int16_t xbmH, const int16_t xbmW, uint16_t xbmColor,
                                    uint32_t btnBackgroundColor,
                                    bool borders)
                        : XBMBitmapPtr(XBMBitmapPtr), xbmH(xbmH), xbmW(xbmW), xbmColor(xbmColor),
                                ButtonWidget(x,y,h,w, notifyTo, btnBackgroundColor, borders) {
    lUIDeepLog("%s new %p\n",__PRETTY_FUNCTION__,this);
    InternalRedraw();

}

void ButtonImageXBMWidget::InternalRedraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    ButtonWidget::InternalRedraw(); // call button normal draw

    // add the icon
    uint32_t imgColor = xbmColor;
    if ( enabled ) {
        buffer->canvas->drawXBitmap(((w/2)-(xbmW/2))+1,((h/2)-(xbmH/2))+1,XBMBitmapPtr, xbmW, xbmH, ThCol(shadow));
    } else {
        imgColor = TFT_DARKGREY;
    }
    buffer->canvas->drawXBitmap((w/2)-(xbmW/2),(h/2)-(xbmH/2),XBMBitmapPtr, xbmW, xbmH, imgColor);
    buffer->canvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
}
