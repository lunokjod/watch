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

#ifndef __LUNOKIOT__CONTROL_ICONMENU_CLASS__
#define __LUNOKIOT__CONTROL_ICONMENU_CLASS__

#include "../../lunokIoT.hpp"
#include "base/Control.hpp"
#include <LilyGoWatch.h>
#include "base/Control.hpp"
#include "base/Container.hpp"

namespace LuI {

    typedef struct {
        const char *name;
        const unsigned char * imagebits;
        const int16_t height;
        const int16_t width;
        UICallback callback;

    } IconMenuEntry;

    // defines root of any screen element
    class IconMenu: public Control {
        protected:
            const uint32_t imageWidth=0;
            const uint32_t imageHeight=0;
            TFT_eSprite * bufferCanvas=nullptr;
            const IconMenuEntry *entries;
            int numEntries=0;
        public:
            bool showCatalogue=false;
            uint16_t backgroundColor=TFT_BLACK;
            int selectedEntry=0;
            UICallback pageCallback=nullptr; // where call when tap
            void * pageCallbackParam=nullptr; // what pass to callback

            bool alreadyChanged=false;
            int16_t lastRelDrag=0;
            int16_t relDrag=0;
            LuI_Layout layout=LuI_Horizontal_Layout;
            int32_t displacement=0;
            void Launch();
            ~IconMenu();
            IconMenu(LuI_Layout layout=LuI_Horizontal_Layout, int menuEntries=0,const IconMenuEntry *optionList=nullptr);
            void Refresh(bool direct=false) override;
    };
};


#endif
