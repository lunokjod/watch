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

#ifndef __LUNOKIOT__WATCHFACE2_APP__
#define __LUNOKIOT__WATCHFACE2_APP__

#include <Arduino.h>
#include "../system/Network.hpp"
#include "../system/Application.hpp"
#include "../UI/activator/ActiveRect.hpp"
#include "../UI/widgets/CanvasZWidget.hpp"

class Watchface2Application: public LunokIoTApplication {
    protected:
        unsigned long nextRefresh=0;
        //CanvasZWidget * colorBuffer = nullptr;
        CanvasWidget * SphereBackground = nullptr;
        CanvasWidget * SphereForeground = nullptr;

        ActiveRect * topLeftButton = nullptr;    
        ActiveRect * topRightButton = nullptr;
        ActiveRect * bottomRightButton = nullptr;
        ActiveRect * bottomLeftButton = nullptr;
        int16_t middleX;
        int16_t middleY;
        int16_t radius;
        const int16_t margin = 5;
        const float DEGREE_HRS = (360/24);
        int markAngle=0;
    public:
        const char *AppName() override { return "Analogic watchface"; };
        Watchface2Application();
        virtual ~Watchface2Application();
        bool Tick();
        const bool isWatchface() override { return true; }
        const bool mustShowAsTask() override { return false; }
    tunable:
        // hourt numbers on watchface (12/3/6/9)
        const int16_t NumberMargin = 20; // distance inner border of sphere
        const GFXfont * NumberFreeFont = &FreeMonoBold24pt7b; // font
        const uint8_t NumberSize = 1; // size
        const uint16_t NumberColorBright=tft->color24to16(0x546084); // bright
        const uint16_t NumberColor=tft->color24to16(0x1f2d56); // color
        const uint16_t NumberColorShadow=tft->color24to16(0x071232); // shadow
        const uint8_t OverSphereAlpha=128;
        // hands values
        const int32_t SecondsTickness = 2;
        const uint16_t SecondsColor=ThCol(clock_hands_second);
        const uint8_t MaxSecondsLen = 10; // define the lenght of hand

        const int32_t MinutesTickness = 3;
        const uint16_t MinutesColor=tft->color24to16(0x787ca0);
        const uint8_t MaxMinutesLen = 100; // define the lenght of hand

        const int32_t HoursTickness = 5;
        const uint16_t HoursColor=ThCol(highlight);
        const uint8_t MaxHourLen = 60; // define the lenght of hand
};

#endif
