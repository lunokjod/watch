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

//#include <Arduino.h>
#include "AppTemplate.hpp"
//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include "../lunokiot_config.hpp"

#include "../app/LogView.hpp"   // for lLog functions
#include "../static/img_backscreen_24.xbm" // system back screen image
//#include "../static/img_back_32.xbm"              // old back button
#include "../UI/widgets/ButtonImageXBMWidget.hpp" // back button

TemplateApplication::~TemplateApplication() { delete btnBack; }

TemplateApplication::TemplateApplication() {
    btnBack=new ButtonImageXBMWidget(0,canvas->height()-32,32,32,[&,this](void *unused){
        LaunchWatchface();
    },img_backscreen_24_bits,img_backscreen_24_height,img_backscreen_24_width,ThCol(background_alt),0,false);
}

bool TemplateApplication::Tick() {
    bool backTap = btnBack->Interact(touched,touchX, touchY); 
    if ( backTap ) {
        btnBack->DirectDraw();
    } else {
    //if ( millis() > nextRefresh ) {
        btnBack->DrawTo(canvas);
    }
    return backTap;
}
