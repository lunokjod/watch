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

#ifndef __LUNOKIOT__UI__BUTTON___
#define __LUNOKIOT__UI__BUTTON___

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include <functional>
#include "../UI.hpp"

class ButtonWidget: public ActiveRect, public CanvasWidget {
    protected:
        bool lastPushed=false;
    public:
        CanvasWidget * buffer; // this fucking buffer is only for bitswap color

        bool Interact(bool touch, int16_t tx,int16_t ty);
        virtual ~ButtonWidget();

        void BuildCanvas(int16_t h, int16_t w);
        bool CheckBounds(); // rebuild the canvas if is changed (future animations/transitions) don't need to call directly
        ButtonWidget(int16_t x,int16_t y, int16_t h, int16_t w, UICallback notifyTo=nullptr, uint32_t btnBackgroundColor=ThCol(button), bool borders=true);

        virtual void InternalRedraw();
        virtual void DrawTo(TFT_eSprite * endCanvas);
        virtual void DirectDraw();

        bool borders=true; // want borders or only ActiveRect?
        uint32_t btnBackgroundColor; // optional color

};
#endif
