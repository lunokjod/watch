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

#ifndef __LUNOKIOT__CONTAINER_BASE_CLASS__
#define __LUNOKIOT__CONTAINER_BASE_CLASS__
#include "../../../lunokIoT.hpp"
#include "Control.hpp"
#define LuI_Layout bool
#define LuI_Horizontal_Layout true
#define LuI_Vertical_Layout false
namespace LuI {
//    uint16_t  dmaBuffer2[16*16]; // Toggle buffer for 16*16 MCU block, 512bytes
//    uint16_t* dmaBufferPtr = dmaBuffer1;
//    bool dmaBufferSel = 0;

    // defines a container
    class Container : public Control {
        protected:
            bool layout=LuI_Vertical_Layout;
            size_t currentChildSlot=0;
            size_t childNumber=1;
            Control ** children;
            float *quotaValues=nullptr;
            //bool parentRefresh=true;
        public:
            uint8_t border=0;
            bool AddChild(INOUT Control *control,IN float quota=1.0);
            Container(LuI_Layout layout=LuI_Vertical_Layout, size_t childs=1);
            ~Container();
            void EventHandler() override;
            void Refresh(bool direct=false) override;
    };
};

#endif
