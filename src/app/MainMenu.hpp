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

#ifndef __LUNOKIOT__MAINMENU_APP__
#define __LUNOKIOT__MAINMENU_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"

class MainMenuApplication: public LunokIoTApplication {
    private:
        int16_t lastTouchX=0;
        bool lastTouch = false;
        bool displaced = false;
        unsigned long nextRedraw = 0;
        int32_t displacement = 0;
        int16_t newDisplacement=0;
        int32_t currentAppOffset=0;
        const static int MenuItemSize = 130;
        int32_t appNamePosX = (TFT_WIDTH/2);
        int32_t appNamePosY = TFT_HEIGHT-20;
    public:
        const char *AppName() override { return "Main menu"; };
        const bool mustShowAsTask() override { return false; }
        MainMenuApplication();
        bool Tick();
};

#endif
