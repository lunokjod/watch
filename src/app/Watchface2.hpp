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
        CanvasWidget * SphereHands = nullptr;
        CanvasWidget * SphereForeground = nullptr;
        static CanvasWidget * StateDisplay;
        TFT_eSprite * lastCanvas=nullptr;
        //bool taskRunning=false;
        esp_event_handler_instance_t ListenerUI;
        //unsigned long pushLetter=0;
        uint8_t bannerSpace=0;
        static int16_t bannerOffset;
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
        const uint8_t CleanupFPS=3;
        const uint8_t DesiredFPS=16;
        char *textBuffer = nullptr;
        void DrawDot(TFT_eSprite * view, int32_t x, int32_t y, int32_t r, uint16_t color, uint16_t capColor);
    public:
        bool markForDestroy=false;
        void DestroyDoubleBuffer();
        const char *AppName() override { return "Analogic watchface"; };
        Watchface2Application();
        virtual ~Watchface2Application();
        void RedrawBLE();
        void RedrawDisplay();
        void RedrawHands(struct tm *timeinfo);
        void Redraw();
        bool Tick();
        const bool isWatchface() override { return true; }
        const bool mustShowAsTask() override { return false; }
        int lastMin = -1;
        const int DisplayFontWidth=19;
    tunable:
        const int8_t DotSize=3;
        const bool ShowNumbers=false;
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
        const uint16_t SecondsBrightColor=tft->alphaBlend(128,SecondsColor,TFT_WHITE);
        const uint8_t MaxSecondsLen = 98; // define the lenght of hand (from up)

        const int32_t MinutesTickness = 4;
        const uint16_t MinutesColor=tft->color24to16(0x787ca0);
        const uint16_t MinutesBrightColor=tft->alphaBlend(128,MinutesColor,TFT_WHITE);
        const uint8_t MinMinutesLen = 30;
        const uint8_t MaxMinutesLen = 60; // define the lenght of hand

        const int32_t HoursTickness = 5;
        const uint16_t HoursColor=ThCol(highlight);
        const uint16_t HoursBrightColor=tft->alphaBlend(128,HoursColor,TFT_WHITE);
        const uint8_t MinHourLen = 30;
        const uint8_t MaxHourLen = 45; // define the lenght of hand
};

#endif
