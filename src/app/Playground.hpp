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

#ifndef __LUNOKIOT__PLAYGROUND_APP__
#define __LUNOKIOT__PLAYGROUND_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/CanvasZWidget.hpp"

class PlaygroundApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
    public:
        const char *AppName() override { return "Playground test"; };
        //TFT_eSPI *tft = ttgo->tft;
        CanvasWidget *buffer=nullptr;
        bool alphaDirection = true;
        int16_t currentAlpha = 0;
        CanvasWidget *imageTest=nullptr;
        CanvasWidget *imageTest2=nullptr;
        CanvasWidget *imageTest3=nullptr;
        PlaygroundApplication();
        ~PlaygroundApplication();
        bool Tick();
};

#endif
