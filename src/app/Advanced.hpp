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

#ifndef __LUNOKIOT__ADVANCED_SETTINGS_APP__
#define __LUNOKIOT__ADVANCED_SETTINGS_APP__
#include <Arduino.h>
//#include <LilyGoWatch.h>
#include "../system/Application.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/ButtonTextWidget.hpp"

class AdvancedSettingsApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
    public:
        const char *AppName() override { return "Advanced settings"; };
        ButtonImageXBMWidget * btnBack = nullptr;
        ButtonImageXBMWidget * btnErase = nullptr;
        ButtonTextWidget * btnEraseSPIFFS = nullptr;
        ButtonImageXBMWidget * btnLog = nullptr;
        AdvancedSettingsApplication();
        ~AdvancedSettingsApplication();
        bool Tick();
};

#endif
