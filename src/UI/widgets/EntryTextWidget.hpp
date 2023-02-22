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

#ifndef __LUNOKIOT__UI__ENTRYTEXT___
#define __LUNOKIOT__UI__ENTRYTEXT___

#include "ButtonTextWidget.hpp"
#include <functional>
#include "../UI.hpp"
#include "../../app/LogView.hpp"

class EntryTextWidget: public ButtonTextWidget {
    public:
        char *ptrToText=nullptr;
        //TFT_eSprite * lastCanvas=nullptr;
        EntryTextWidget(int16_t x,int16_t y, int16_t h, int16_t w, const char *text=nullptr,lUIKeyboardType keybType=KEYBOARD_ALPHANUMERIC_FREEHAND);
        lUIKeyboardType keyboarTypeToShow=KEYBOARD_ALPHANUMERIC_FREEHAND;
        void InternalRedraw() override;
        void DrawTo(TFT_eSprite * endCanvas) override;
        void ShowKeyboard();
        void HideKeyboard(char *newValue=nullptr);
        ~EntryTextWidget() {
            HideKeyboard();
            if ( nullptr != ptrToText ) { free(ptrToText); }
        }
};

#endif
