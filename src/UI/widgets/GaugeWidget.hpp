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

#ifndef __LUNOKIOT__GAUGE__WIDGET___HEADER__
#define __LUNOKIOT__GAUGE__WIDGET___HEADER__
#include "CanvasWidget.hpp"
#include "../activator/ActiveRect.hpp"

#include "../UI.hpp"

class GaugeWidget : public CanvasWidget, ActiveRect {
    public:
        uint32_t gaugeColor;
        uint32_t needleColor;
        uint32_t markColor;
        uint32_t incrementColor;
        CanvasWidget * buffer;
        bool dirty = true;
        int16_t lastAngle = 0;
        int16_t selectedAngle = 0;
        int32_t markX = -1;
        int32_t markY = -1;
        void ForceDraw();
        GaugeWidget(int16_t x, int16_t y, int16_t d=100, uint32_t wgaugeColor=ThCol(button), uint32_t markColor=ThCol(middle), uint32_t needleColor=ThCol(highlight), uint32_t incrementColor=ThCol(max));
        void DrawTo(TFT_eSprite * endCanvas);
        void DirectDraw();
        ~GaugeWidget();
        bool Interact(bool touch, int16_t tx,int16_t ty, int16_t margin=0); // t for "tap"
};

#endif
