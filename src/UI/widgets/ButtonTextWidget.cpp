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
#include "ButtonTextWidget.hpp"
#include "../../app/LogView.hpp"
#include "../UI.hpp"
#include "CanvasWidget.hpp"
#include <functional>

ButtonTextWidget::ButtonTextWidget(int16_t x,int16_t y, int16_t h, int16_t w, 
            UICallback notifyTo, const char *text,
            uint32_t btnTextColor, uint32_t btnBackgroundColor,
            bool borders) 
            : label(text), btnTextColor(btnTextColor), ButtonWidget(x,y,h,w, notifyTo, btnBackgroundColor, borders) {
    lUIDeepLog("%s new %p\n",__PRETTY_FUNCTION__,this);
    InternalRedraw();
}

void ButtonTextWidget::InternalRedraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    ButtonWidget::InternalRedraw(); // call button normal draw
    // add the text
    if ( nullptr != label ) {
        buffer->canvas->setTextFont(0);
        buffer->canvas->setTextSize(3);
        buffer->canvas->setTextColor(btnTextColor);
        buffer->canvas->setTextDatum(CC_DATUM);
        buffer->canvas->drawString(label,(w/2),(h/2));
    }
    buffer->canvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
}
