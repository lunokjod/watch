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

#ifndef __LUNOKIOT__LOCKPICKING_GAME__
#define __LUNOKIOT__LOCKPICKING_GAME__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/GaugeWidget.hpp"

class LockPickingGameApplication: public TemplateApplication {
        unsigned long nextRedraw=0;
        GaugeWidget * wristGauge = nullptr;
        int16_t lastAngle=0;
        int16_t levelCorrectAngle=0;
        int16_t levelDriftAngle=20;
        int16_t currentLevel=0;
        bool playerWin=false;
        const int16_t aMIN=90;
        const int16_t aMAX=250;
        uint8_t nearby=0;
        unsigned long nextHapticBeat=0;
    public:
        const char *AppName() override { return "LockPickingGame"; };
        LockPickingGameApplication();
        ~LockPickingGameApplication();
        bool Tick();
};

#endif
