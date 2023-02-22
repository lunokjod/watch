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

#ifndef __LUNOKIOT__STEPWATCH_APP__
#define __LUNOKIOT__STEPWATCH_APP__
/*
 * This app explains the KVO funcionality (callback when bus event received)
 */

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../system/Application.hpp"

class StopwatchApplication: public TemplateApplication {
    private:
        unsigned long nextRedraw=0;
        ButtonImageXBMWidget * resetBtn = nullptr;
        ButtonImageXBMWidget * pauseBtn = nullptr;
        ButtonImageXBMWidget * startBtn = nullptr;
        unsigned long nextBlink=0;
        bool blink=false;
    public:
        const char *AppName() override { return "Stopwatch"; };
        static unsigned long pauseTime;
        static unsigned long starTime;
        StopwatchApplication();
        ~StopwatchApplication();
        bool Tick();
};

#endif
