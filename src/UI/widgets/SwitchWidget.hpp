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

#ifndef __LUNOKIOT__UI__SWITCH___
#define __LUNOKIOT__UI__SWITCH___

#include "../activator/ActiveRect.hpp"  // parents
#include "CanvasWidget.hpp"
#include <functional>                   // callbacks/lambda
#include "../UI.hpp"                    // theme

class SwitchWidget: public ActiveRect, public CanvasWidget {
    protected:
        bool lastInteract=false;
    public:
        CanvasWidget * buffer=nullptr;
        void SetEnabled(bool enabled);
        bool Interact(bool  touch, int16_t tx,int16_t ty);
        virtual ~SwitchWidget();

        SwitchWidget(int16_t x, int16_t y, UICallback notifyTo=nullptr, uint32_t switchColor=ThCol(button));

        virtual void InternalRedraw();
        virtual void DrawTo(TFT_eSprite * endCanvas);
        virtual void DirectDraw();

        volatile bool switchEnabled=false;
        uint32_t switchColor;
        const int32_t switchHeight = 20; //@TODO this must define canvas sizes

};
#endif
