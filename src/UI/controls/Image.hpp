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

#ifndef __LUNOKIOT__CONTROL_IMAGE_CLASS__
#define __LUNOKIOT__CONTROL_IMAGE_CLASS__

#include "../../lunokIoT.hpp"
#include "base/Control.hpp"
#include <LilyGoWatch.h>
#include "base/Container.hpp"

namespace LuI {

    // defines root of any screen element
    class Image: public Container {
        protected:
            const uint32_t imageWidth=0;
            const uint32_t imageHeight=0;
            const unsigned char *imageData=nullptr;
            TFT_eSprite * imageCanvas=nullptr;
            uint16_t maskColor = TFT_PINK;
        public:
            uint32_t offsetX=0;
            uint32_t offsetY=0;
            ~Image();
            Image(IN uint32_t width, IN uint32_t height, IN unsigned char *data=nullptr,uint16_t maskColor=TFT_PINK,LuI_Layout layout=LuI_Vertical_Layout, size_t childs=1);
            void Refresh(bool direct=false) override;
            TFT_eSprite * GetBuffer() { return imageCanvas; }
    };
};


#endif
