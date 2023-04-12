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

#ifndef __LUNOKIOT__CONTROL_TEXT_CLASS__
#define __LUNOKIOT__CONTROL_TEXT_CLASS__

#include "../../lunokIoT.hpp"
#include "base/Control.hpp"
#include <LilyGoWatch.h>

namespace LuI {

    // defines root of any screen element
    class Text: public Control {
        protected:
            const uint32_t imageTextWidth=0;
            const uint32_t imageTextHeight=0;
            TFT_eSprite * imageTextCanvas=nullptr;
            const GFXfont *font=nullptr;
            char * text=nullptr;
            uint16_t color=TFT_WHITE;
            uint16_t backgroundColor=Drawable::MASK_COLOR;
            bool swapColor=false;
            uint8_t textSize=1;
        public:
            void SetBackgroundColor(uint16_t color) { backgroundColor=color; }
            void SetText(const char * what);
            ~Text();
            Text(const char * what, IN uint16_t color=TFT_WHITE,IN bool swap=false, IN uint8_t tsize=1, IN GFXfont *font=&FreeMonoBold9pt7b);
            void Refresh(bool direct=false) override;
    };
};


#endif
