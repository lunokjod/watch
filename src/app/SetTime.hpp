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

#ifndef __LUNOKIOT__SET_TIME_APP__
#define __LUNOKIOT__SET_TIME_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/ValueSelector.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class SetTimeApplication: public LunokIoTApplication {
    private:
        bool blinkTime = true;
        unsigned long nextBlinkTime=0;
        unsigned long nextRedraw=0;
        //unsigned long nextSelectorTick=0;
        ValueSelector * hour = nullptr;
        ValueSelector * minute = nullptr;
        ButtonImageXBMWidget * showDateButton = nullptr;
        ButtonImageXBMWidget * setTimeButton = nullptr;
        ButtonImageXBMWidget * backButton = nullptr;
    public:
        const char *AppName() override { return "Set time"; };
        SetTimeApplication();
        ~SetTimeApplication();
        bool Tick();
};

#endif
