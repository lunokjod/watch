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

#ifndef __LUNOKIOT__ABOUT_APP__
#define __LUNOKIOT__ABOUT_APP__

#include <Arduino.h>
//#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/CanvasWidget.hpp"

class AboutApplication: public TemplateApplication {
    private:
        //unsigned long drawOneSecond=0;
        unsigned long nextRedraw=0;
        unsigned long nextStep=0;
        //unsigned long nextScrollRedraw=0;
        uint8_t counterLines=0;
        //const float MAXTIMEZOOM=1.0;
        //const float MINTIMEZOOM=0.5;
        const float TEXTZOOM=0.4;
        bool upOrDown=true;
    public:
        const char *AppName() override { return "About"; };
        TFT_eSprite * textBuffer = nullptr;
        CanvasWidget * colorBuffer = nullptr;
        CanvasWidget * perspectiveTextBuffer = nullptr;
        AboutApplication();
        ~AboutApplication();
        bool Tick();
};

#endif
