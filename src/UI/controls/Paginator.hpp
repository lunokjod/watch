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

#ifndef __LUNOKIOT__CONTROL_PAGINATOR_CLASS__
#define __LUNOKIOT__CONTROL_PAGINATOR_CLASS__

#include "../../lunokIoT.hpp"
#include "base/Control.hpp"
#include <LilyGoWatch.h>
#include "../../app/LogView.hpp"
namespace LuI {

    // defines root of any screen element
    class Paginator: public Control {
        protected:
            uint8_t pages;
            bool useBackground=false;
            uint16_t backgroundColor; 
        public:
            int16_t border=5;
            uint8_t current;
            void SetBackgroundColor(uint16_t color) { backgroundColor=color; useBackground=true; }
            Paginator(IN uint8_t pages);
            void SetCurrent(IN uint8_t page) { current=page; dirty=true; }
            void Refresh(bool direct=false) override;
    };
};


#endif
