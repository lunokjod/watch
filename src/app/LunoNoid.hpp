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

#ifndef __LUNOKIOT__LUNOID_GAME_APP__
#define __LUNOKIOT__LUNOID_GAME_APP__

#include "../system/Application.hpp"


class LunoNoidGameApplication: public LunokIoTApplication {
    private:
        uint8_t level=0;
        int16_t credits=3;      // default lifes
        bool stickyBall = true; // begin with chance of set
        int32_t playerPosition=tft->width()/2; // center on the screen
        unsigned long nextTimeout=0;
        const uint8_t BlockWidth=40;
        const uint8_t BlockHeight=20;
        const uint8_t BarLenght=70;
        TFT_eSprite * enemyMap=nullptr;
        int32_t ballX = tft->width()/2;
        int32_t ballY = tft->height()/2;
        int8_t BallSpeedX=4;
        int8_t BallSpeedY=5;
        unsigned long nextRefresh=0;
        //TFT_eSprite * doubleBuffer=nullptr;
        void GatherButtons();
        bool groundSet=false;
        float refRollval=0;
        float refPitchval=0;
        float rollval=0;
        float pitchval=0;
    public:
        const char *AppName() override { return "LunoNoid game"; };
        LunoNoidGameApplication();
        virtual ~LunoNoidGameApplication();
        bool Tick();
};

#endif
