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

#ifndef __LUNOKIOT__STEPSMILESTONESETUP_APP__
#define __LUNOKIOT__STEPSMILESTONESETUP_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"
#include "../UI/widgets/ValueSelector.hpp"
#include "../UI/widgets/SwitchWidget.hpp"

class StepsMilestoneSetupApplication: public TemplateApplication {
    private:
        unsigned long nextRedraw=0;
        unsigned long nextSpinStep=0;
    public:
        const char *AppName() override { return "Step Milestone settings"; };
        ValueSelector * desiredSteps = nullptr;
        StepsMilestoneSetupApplication();
        ~StepsMilestoneSetupApplication();
        bool Tick();
};

#endif
