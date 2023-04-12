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

#ifndef __LUNOKIOT__CONTROL_BASE_CLASS__
#define __LUNOKIOT__CONTROL_BASE_CLASS__

#include "../../../lunokIoT.hpp"
#include <libraries/TFT_eSPI/TFT_eSPI.h>
//#include <LilyGoWatch.h>
#include "../../UI.hpp"
extern TFT_eSPI * tft;
namespace LuI {

    // defines root of any screen element
    class Control {
        protected:
            TFT_eSprite *canvas=nullptr;
            bool lastTouched=false;
            Control *parent=nullptr;
            //bool lastIn=false; // triggers when finger leaves my area
        public:
            bool dirty=true; // forces redraw on instance

            // events
            UICallback touchCallback=nullptr; // where call meanwhile thumb in
            void * touchCallbackParam=nullptr; // what pass to callback

            UICallback inTouchCallback=nullptr; // where call when thumb in
            void * inTouchCallbackParam=nullptr; // what pass to callback

            UICallback outTouchCallback=nullptr; // where call when thumb out
            void * outTouchCallbackParam=nullptr; // what pass to callback

            UICallback tapCallback=nullptr; // where call when tap
            void * tapCallbackParam=nullptr; // what pass to callback
            
            UICallback dragCallback=nullptr; // where call when drag
            void * dragCallbackParam=nullptr; // what pass to callback
            
            uint32_t height=0;
            uint32_t width=0;
            uint32_t clipX=0;
            uint32_t clipY=0;
            virtual void EventHandler();
            virtual TFT_eSprite * GetCanvas();
            bool Attach(INOUT Control *control);
            virtual ~Control();
            Control(uint32_t w=100, uint32_t h=100);
            void SetSize(uint32_t w, uint32_t h);
            void SetWidth(uint32_t w);
            void SetHeight(uint32_t h);
            void Rebuild();
            virtual void Refresh(bool direct=false);
    };
};


#endif
