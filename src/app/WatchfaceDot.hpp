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

#ifndef __LUNOKIOT__APPLICATION__WATCHFACE_DOT__
#define __LUNOKIOT__APPLICATION__WATCHFACE_DOT__

#include "../system/Application.hpp"
#include "../UI/activator/ActiveRect.hpp"

class WatchfaceDotApplication : public LunokIoTApplication {
    private:
        ActiveRect * bottomRightButton = nullptr;
        unsigned long nextRefresh=0;
        CanvasWidget * displayBuffer = nullptr;
        //uint8_t *dotSize = nullptr;
        bool drawDot=false;
        unsigned long nextBlink=0;
        uint8_t dotSize=2;
        //bool dotSizeUpDown=false;
        int16_t locX=0;
        int16_t locY=0;
        bool locXDir=false;
        bool locYDir=false;
        int16_t circleRadius[20] = { 0 };
        int16_t circleX[20]= { 0 };
        int16_t circleY[20]= { 0 };
    public:
        const char *AppName() override { return "Watchface Dot"; };
        WatchfaceDotApplication();
        ~WatchfaceDotApplication();
        bool Tick();
        const bool isWatchface() override { return true; }
        const bool mustShowAsTask() override { return false; }
};

#endif
