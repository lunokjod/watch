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

#ifndef __LUNOKIOT__UI__BUTTONIMAGE___
#define __LUNOKIOT__UI__BUTTONIMAGE___

#include "ButtonWidget.hpp"
#include <functional>
#include "../UI.hpp"
class ButtonImageXBMWidget: public ButtonWidget {
    public:
        uint32_t xbmColor;
        const unsigned char * XBMBitmapPtr;
        int16_t xbmH;
        int16_t xbmW;
        void InternalRedraw() override;
        ButtonImageXBMWidget(int16_t x,int16_t y, int16_t h, int16_t w, 
                        UICallback notifyTo, 
                        const unsigned char * XBMBitmapPtr,
                        const int16_t xbmH, const int16_t xbmW, uint16_t xbmColor=ThCol(text),
                        uint32_t btnBackgroundColor=ThCol(button), bool borders=true);
};
#endif
