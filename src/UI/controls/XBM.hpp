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

#ifndef __LUNOKIOT__CONTROL_XBMIMAGE_CLASS__
#define __LUNOKIOT__CONTROL_XBMIMAGE_CLASS__

#include "../../lunokIoT.hpp"
#include "base/Control.hpp"
#include <LilyGoWatch.h>

namespace LuI {

    // defines root of any screen element
    class XBM: public Control {
        protected:
            const uint32_t imageWidth=0;
            const uint32_t imageHeight=0;
            const unsigned char *imageData=nullptr;
            TFT_eSprite * imageCanvas=nullptr;
        public:
            ~XBM();
            XBM(IN uint32_t width, IN uint32_t height, IN unsigned char *data,bool swap=false);
            void Refresh(bool direct=false,bool swap=false) override;
    };
};


#endif
