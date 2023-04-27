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

#ifndef __LUNOKIOT__WATCHFACE_BASIC__
#define __LUNOKIOT__WATCHFACE_BASIC__

#include "../system/Application.hpp"

class WatchfaceBasic : public LunokIoTApplication {
    private:
        unsigned long nextRefresh=0;
        ActiveRect * topLeftButton = nullptr;    
        ActiveRect * topRightButton = nullptr;
        ActiveRect * bottomRightButton = nullptr;
        ActiveRect * bottomLeftButton = nullptr;
        bool showDot=false;
        unsigned long dotRefresh=0;
        uint8_t dotSize=8;
        CanvasWidget * displayBuffer = nullptr;
        // RED
        const uint32_t LedTimeColor=tft->color24to16(0xdd3030);
        // BLUE const uint32_t LedTimeColor=tft->color24to16(0x005fff);
        int32_t refreshPoint=0;
        int32_t refreshPointDelayMS=0;
    public:
        const char *AppName() override { return "Watchface basic"; };
        WatchfaceBasic();
        ~WatchfaceBasic();
        virtual const bool isWatchface() { return true; }
        virtual const bool mustShowAsTask() { return false; }
        bool Tick();
};

#endif
