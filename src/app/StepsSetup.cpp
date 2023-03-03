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
#include <LilyGoWatch.h>
#include "Settings.hpp"
#include "../static/img_back_32.xbm"
#include <ArduinoNvs.h>
#include "Steps.hpp"
#include "StepsSetup.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../lunokiot_config.hpp"
#include "../lunokIoT.hpp"

extern bool UILongTapOverride;

StepsSetupApplication::~StepsSetupApplication() {
    userTall = tallValue->selectedValue;
    NVS.setInt("UserTall",userTall, false);
    userMaleFemale = genderSelect->switchEnabled;
    NVS.setInt("UserSex",userMaleFemale, false);
    if ( true == userMaleFemale ) { // duplicated code on main
        // FEMALE PROPS
        stepDistanceCm = userTall * WOMAN_STEP_PROPORTION;
    } else {
        stepDistanceCm = userTall * MAN_STEP_PROPORTION;
    }
    if ( nullptr != btnBack ) { delete btnBack; }
    if ( nullptr != genderSelect ) { delete genderSelect; }
    if ( nullptr != tallValue ) { delete tallValue; }
    UILongTapOverride=false;
}

StepsSetupApplication::StepsSetupApplication() {
    btnBack=new ButtonImageXBMWidget(TFT_WIDTH-69,TFT_HEIGHT-69,64,64,[&,this](IGNORE_PARAM){
        LaunchApplication(new StepsApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,ThCol(text),ThCol(background),false);
    genderSelect = new SwitchWidget(142,20); // ,[&,this](void *unused){ });
    tallValue = new ValueSelector(10,10,220,110,120,220,ThCol(background));
    tallValue->selectedValue = userTall;
    tallValue->InternalRedraw();
    genderSelect->switchEnabled = userMaleFemale;
    Tick();
    UILongTapOverride=true;
}

bool StepsSetupApplication::Tick() {

    btnBack->Interact(touched,touchX, touchY);
    genderSelect->Interact(touched,touchX, touchY);
    tallValue->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        btnBack->DrawTo(canvas);
        genderSelect->DrawTo(canvas);
        tallValue->DrawTo(canvas);

        canvas->setTextFont(0);
        canvas->setTextSize(3);
        canvas->setTextDatum(TL_DATUM);
        canvas->setTextWrap(false,false);
        canvas->setTextColor(ThCol(text));
        canvas->drawString("M", 120,40);
        canvas->drawString("F", 210,40);

        canvas->setTextSize(4);
        canvas->setTextDatum(BL_DATUM);
        canvas->drawString("CM", 120,120);
        canvas->setTextSize(3);
        canvas->setTextDatum(TL_DATUM);
        canvas->drawString("Tall", 120,120);

        nextRedraw=millis()+(1000/4);
        return true;
    }
    return false;
}
