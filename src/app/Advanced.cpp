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
#include "Advanced.hpp"
#include "LogView.hpp"
#include "../resources.hpp"
#include "../UI/UI.hpp"
#include "Shutdown.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
//#include "../UI/widgets/SwitchWidget.hpp"

AdvancedSettingsApplication::~AdvancedSettingsApplication() {
    if ( nullptr != btnBack ) { delete btnBack; }
    if ( nullptr != btnErase ) { delete btnErase; }
    if ( nullptr != btnLog ) { delete btnLog; }
}

AdvancedSettingsApplication::AdvancedSettingsApplication() {
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](void *bah){
        LaunchWatchface();
    },img_back_32_bits,img_back_32_height,img_back_32_width,ThCol(text),ThCol(button),false);
    btnErase=new ButtonImageXBMWidget(5,5,64,64,[&,this](void *bah){
        NVS.eraseAll(true);
        LaunchApplication(new ShutdownApplication(true,false));
    },img_trash_32_bits,img_trash_32_height,img_trash_32_width,ThCol(text),ThCol(high));

    btnEraseSPIFFS = new ButtonTextWidget(79,5,64,240-5-79,[](void *bah) {
        NVS.setInt("littleFSReady",false);
        LaunchApplication(new ShutdownApplication(true,true));
    },"LittleFS",ThCol(text),ThCol(high));

    btnLog=new ButtonImageXBMWidget(20,80,64,200,[&,this](void *bah){
        LaunchApplication(new LogViewApplication());
    },img_log_32_bits,img_log_32_height,img_log_32_width,ThCol(text),ThCol(background_alt));

    Tick();
}

bool AdvancedSettingsApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY);
    btnErase->Interact(touched,touchX, touchY);
    btnEraseSPIFFS->Interact(touched,touchX, touchY);
    btnLog->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        btnBack->DrawTo(canvas);
        btnErase->DrawTo(canvas);
        btnEraseSPIFFS->DrawTo(canvas);
        btnLog->DrawTo(canvas);
        nextRedraw=millis()+(1000/10);
        return true;
    }
    return false;
}
