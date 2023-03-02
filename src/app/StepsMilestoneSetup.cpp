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

#include <Arduino.h>
#include <ArduinoNvs.h>
#include <LilyGoWatch.h>
#include "Settings.hpp"
#include "../static/img_back_32.xbm"
#include <ArduinoNvs.h>
#include "Steps.hpp"
#include "StepsMilestoneSetup.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../lunokiot_config.hpp"
#include "../lunokIoT.hpp"
extern bool UILongTapOverride;

StepsMilestoneSetupApplication::~StepsMilestoneSetupApplication() {
    if ( nullptr != desiredSteps ) {
        NVS.setInt("smilestone",desiredSteps->selectedValue,false);
        delete desiredSteps;
    }
    UILongTapOverride=false;
}

StepsMilestoneSetupApplication::StepsMilestoneSetupApplication() {
    // rewrite back button callback
    btnBack->tapActivityCallback = [&, this](IGNORE_PARAM) { LaunchApplication(new StepsApplication()); };
    desiredSteps = new ValueSelector(10,20,180,220,100,20000,ThCol(background),false,100);
    desiredSteps->selectedValue = NVS.getInt("smilestone");
    desiredSteps->InternalRedraw();
    Tick();
    UILongTapOverride=true;
}

bool StepsMilestoneSetupApplication::Tick() {
    TemplateApplication::btnBack->Interact(touched,touchX, touchY);
    desiredSteps->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextDatum(TC_DATUM);
        canvas->setTextColor(ThCol(text));
        canvas->drawString("Milestone Steps", canvas->width()/2,5);
        char buffer[30];
        float distance = (desiredSteps->selectedValue*stepDistanceCm)/100;
        if ( distance > 1000 ) {
            sprintf(buffer,"about %.2f Km",distance/1000);
        } else {
            sprintf(buffer,"about %.2f mts",distance);
        }
        canvas->drawString(buffer, canvas->width()/2,200);

        TemplateApplication::btnBack->DrawTo(canvas);
        desiredSteps->DrawTo(canvas);

        nextRedraw=millis()+(1000/4);
        return true;
    }
    return false;
}
