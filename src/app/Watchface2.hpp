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
#include "../system/Datasources/kvo.hpp"
#include "../system/Network.hpp"
#include "../system/Application.hpp"
//#include "../UI/activator/ActiveRect.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
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
        // event monitoring for UI
        EventUIKVO * UIRefreshKVO=nullptr;
        EventUIKVO * UIContinueKVO=nullptr;
        // system event monitoring for Status
        EventKVO * BatteryFullKVO=nullptr;
        EventKVO * BatteryChargingKVO=nullptr;
        EventKVO * USBInKVO=nullptr;
        EventKVO * USBOutKVO=nullptr;
        EventKVO * ScreenDirectionKVO=nullptr;
        bool ShowNumbersMustRefresh=true;
        //unsigned long pushLetter=0;
        uint8_t bannerSpace=0;
        static int16_t bannerOffset;
        /*
        ActiveRect * topLeftButton = nullptr;
        ActiveRect * topRightButton = nullptr;
        ActiveRect * bottomRightButton = nullptr;
        ActiveRect * bottomLeftButton = nullptr;
        */
        ButtonImageXBMWidget * topLeftButton = nullptr;
        ButtonImageXBMWidget * topRightButton = nullptr;
        ButtonImageXBMWidget * bottomRightButton = nullptr;
        ButtonImageXBMWidget * bottomLeftButton = nullptr;
        int16_t middleX;
        int16_t middleY;
        int16_t radius;
        const int16_t margin = 5;
        const float DEGREE_HRS = (360/24);
        int markAngle=0;
        const uint8_t CleanupFPS=3;
        const uint8_t DesiredFPS=12;
        char *textBuffer = nullptr;
        void DrawDot(TFT_eSprite * view, int32_t x, int32_t y, int32_t r, uint16_t color, uint16_t capColor);
        char * whatToSay =nullptr;
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
        void NumberLayerReraw();
        bool Tick();
        const bool isWatchface() override { return true; }
        const bool mustShowAsTask() override { return false; }
        int lastMin = -1;
        const int DisplayFontWidth=19;
        void SetStatus(char*what);
        void SetStatus(const char*what);
        SemaphoreHandle_t StatusSemaphore = xSemaphoreCreateMutex();
        bool wifiStatus=false;
        bool weatherStatus=false;
    tunable:
        const uint16_t DisplayColor=tft->color24to16(0x437aff);
        const int16_t DisplaySpeed=12; // speed of text display
        const int8_t DotSize=3;
        // hour numbers on watchface (12/3/6/9)
        const bool ShowNumbers=true;
        const int16_t NumberMargin = 16; // distance inner border of sphere
        const GFXfont * NumberFreeFont = &FreeMonoBold18pt7b; // font
        const uint8_t NumberSize = 1; // size
        const uint16_t NumberColorBright=tft->color24to16(0x546084); // bright
        const uint16_t NumberColor=tft->color24to16(0x1f2d56); // color
        const uint16_t NumberColorShadow=tft->color24to16(0x071232); // shadow
        const uint8_t OverSphereAlpha=92;
        // hands values
        const int32_t SecondsTickness = 2;
        const uint16_t SecondsColor=ThCol(clock_hands_second);
        const uint16_t SecondsBrightColor=tft->alphaBlend(128,SecondsColor,TFT_WHITE);
        const uint8_t MaxSecondsLen = 98; // define the lenght of hand (from up)

        const int32_t MinutesTickness = 6;
        const uint16_t MinutesColor=tft->color24to16(0x787ca0);
        const uint16_t MinutesBrightColor=tft->alphaBlend(128,MinutesColor,TFT_WHITE);
        const uint8_t MinMinutesLen = 30;
        const uint8_t MaxMinutesLen = 80; // define the lenght of hand

        const int32_t HoursTickness = 8;
        const uint16_t HoursColor=tft->color24to16(0xfcb61d);
        const uint16_t HoursBrightColor=tft->alphaBlend(128,HoursColor,TFT_BLACK);
        const uint8_t MinHourLen = 30;
        const uint8_t MaxHourLen = 45; // define the lenght of hand
};

#endif
