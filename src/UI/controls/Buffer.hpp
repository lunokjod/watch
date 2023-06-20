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

#ifndef __LUNOKIOT__CONTROL_BUFFER_CLASS__
#define __LUNOKIOT__CONTROL_BUFFER_CLASS__

#include "../../lunokIoT.hpp"
#include "base/Control.hpp"
#include <LilyGoWatch.h>
#include "base/Control.hpp"
#include "base/Container.hpp"

namespace LuI {

    // defines root of any screen element
    class Buffer: public Control {
        protected:
            const uint32_t imageWidth=0;
            const uint32_t imageHeight=0;
            TFT_eSprite * imageCanvas=nullptr;
            TFT_eSprite * barHorizontal=nullptr;
            TFT_eSprite * barVertical=nullptr;
            int16_t fadeDownBarsValue=255; // used to fadedown bars effect
        public:
            int16_t backgroundColor=ThCol(background);
            UICallback bufferPushCallback=nullptr; // where call when tap
            void * bufferPushCallbackParam=nullptr; // what pass to callback
            //bool hideBars=true;
            bool showBars=true;
            int32_t offsetX=0;
            int32_t offsetY=0;
            ~Buffer();
            TFT_eSprite * GetBuffer() { return imageCanvas; }
            void SetBuffer(TFT_eSprite *newBuffer) {
                TFT_eSprite * temp = imageCanvas;
                imageCanvas=newBuffer;
                temp->deleteSprite();
                delete temp;
            }
            void DrawBars();
            Buffer(IN uint32_t width, IN uint32_t height);
            void Refresh(bool direct=false) override;
    };
};


#endif
