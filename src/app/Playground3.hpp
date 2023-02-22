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

#ifndef __LUNOKIOT__PLAYGROUND3_APP__
#define __LUNOKIOT__PLAYGROUND3_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../UI/widgets/GaugeWidget.hpp"

class PlaygroundApplication3: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        unsigned long timeTick=0;
        unsigned long fixTimeTimeout=-1;
        const unsigned long SetTimeTimeout=5000; 
        bool showTime = true;
        bool timeIsSet = false;
        int hour=0;
        int minute=0;
    public:
        const char *AppName() override { return "Playground3 test"; };

        GaugeWidget * hourGauge = nullptr;
        GaugeWidget * minuteGauge = nullptr;

        PlaygroundApplication3();
        ~PlaygroundApplication3();
        bool Tick();
};

#endif
