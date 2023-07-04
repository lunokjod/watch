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
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "GestureSettings.hpp"
#include "../lunokIoT.hpp"
#include <freertos/task.h>

#include <LilyGoWatch.h>
extern TTGOClass *ttgo;
extern SemaphoreHandle_t I2cMutex;

GestureSettings::GestureSettings() {
    UseDoubleTapSwitch = new SwitchWidget(10,10);    
    bool doubleTapSetting=LoT().GetSettings()->GetInt(SystemSettings::SettingKey::DoubleTap);
    UseDoubleTapSwitch->switchEnabled=doubleTapSetting;
    UseDoubleTapSwitch->InternalRedraw();
    UseLampSwitch = new SwitchWidget(10,72);    
    bool lampGestureSetting= LoT().GetSettings()->GetInt(SystemSettings::SettingKey::LampGesture);
    UseLampSwitch->switchEnabled=lampGestureSetting;
    UseLampSwitch->InternalRedraw();
    Tick(); 
}

GestureSettings::~GestureSettings() {
    if ( nullptr != UseDoubleTapSwitch ) {
        bool doubleTapSetting = UseDoubleTapSwitch->switchEnabled;
        //NVS.setInt("doubleTap",doubleTapSetting,false);
        LoT().GetSettings()->SetInt(SystemSettings::SettingKey::DoubleTap,doubleTapSetting);
        delete UseDoubleTapSwitch;
        BaseType_t done = xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        if ( doubleTapSetting ) {
            bool featureOK = ttgo->bma->enableWakeupInterrupt();
            if ( false == featureOK ) { lEvLog("BMA: Unable to Enable interrupt 'double tap'\n"); }
        } else {
            bool featureOK = ttgo->bma->enableWakeupInterrupt(false);
            if ( false == featureOK ) { lEvLog("BMA: Unable to Disable interrupt 'double tap'\n"); }
        }
        xSemaphoreGive(I2cMutex);
    }
    if ( nullptr != UseLampSwitch ) {
        bool lampGestureSetting = UseLampSwitch->switchEnabled;
        //NVS.setInt("lampGesture",lampGestureSetting,false);
        LoT().GetSettings()->SetInt(SystemSettings::SettingKey::LampGesture,lampGestureSetting);

        delete UseLampSwitch;
    }
}

bool GestureSettings::Tick() {
    TemplateApplication::Tick();
    UseDoubleTapSwitch->Interact(touched,touchX,touchY);
    UseLampSwitch->Interact(touched,touchX,touchY);

    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(ThCol(background)); // use theme colors

        // double tap code
        UseDoubleTapSwitch->DrawTo(canvas);
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextDatum(BL_DATUM);
        uint16_t textColor = ThCol(text);
        if ( false == UseDoubleTapSwitch->switchEnabled ) { textColor = ThCol(text_alt); }
        canvas->setTextColor(textColor);
        canvas->drawString("Double tap",UseDoubleTapSwitch->GetX()+72,UseDoubleTapSwitch->GetY()+32);
        canvas->setTextDatum(TL_DATUM);
        canvas->setTextSize(1);
        canvas->drawString("launches watchface",UseDoubleTapSwitch->GetX()+72,UseDoubleTapSwitch->GetY()+32);

        // enable lamp code
        UseLampSwitch->DrawTo(canvas);
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextDatum(BL_DATUM);
        textColor = ThCol(text);
        if ( false == UseLampSwitch->switchEnabled ) { textColor = ThCol(text_alt); }
        canvas->setTextColor(textColor);
        canvas->drawString("Lamp button",UseLampSwitch->GetX()+72,UseLampSwitch->GetY()+32);
        canvas->setTextDatum(TL_DATUM);
        canvas->setTextSize(1);
        canvas->drawString("launches Lamp",UseLampSwitch->GetX()+72,UseLampSwitch->GetY()+32);



        TemplateApplication::btnBack->DrawTo(canvas);
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
