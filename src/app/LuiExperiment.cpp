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

#include "../UI/AppLuITemplate.hpp"
#include "LuiExperiment.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/View3D/View3D.hpp"
#include "../UI/controls/Buffer.hpp"
#include "LogView.hpp"

#include "../../static/img_backscreen_24.xbm" // system back screen image
//#include "../../static/img_smartwatch_160.xbm"
#include "../system/Datasources/kvo.hpp"
#include "../system/SystemEvents.hpp"
#include <ArduinoNvs.h>

#include <LilyGoWatch.h>
using namespace LuI;

extern uint8_t bmaRotation;
extern TTGOClass *ttgo;
extern SemaphoreHandle_t UISemaphore;

LuiExperimentApplication::~LuiExperimentApplication() {
    if ( nullptr != ScreenRotateEvent ) { delete ScreenRotateEvent; }
    NVS.setInt("ScreenRot",tft->getRotation(),false);
}

uint8_t LuiExperimentApplication::BMAtoTFTOrientation(uint8_t bma) {
    if(0 == bmaRotation) { return 3; } // ttgo->tft->setRotation(3); }
    else if(1 == bmaRotation) { return 1; } // ttgo->tft->setRotation(1); }
    else if(2 == bmaRotation) { return 0; } //ttgo->tft->setRotation(0); }
    else if(3 == bmaRotation) { return 2; } //ttgo->tft->setRotation(2); }
    return -1;
}

LuiExperimentApplication::LuiExperimentApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    // fill view with background color
    canvas->fillSprite(ThCol(background)); // use theme colors

    /// create root container with two slots horizontal
    Container * screen = new Container(LuI_Horizonal_Layout,2);
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,2);

    // main view space
    Container * viewContainer = new Container(LuI_Horizonal_Layout,1);

    // if use quota, all the slot quotas must sum equal the total number of elements
    // example: default quota in 2 controls result 50/50% of view space with 1.0 of quota everyone (2.0 in total)

    screen->AddChild(viewContainer,1.65);
    // add bottom button bar shirnked
    screen->AddChild(bottomButtonContainer,0.35);

    // add back button to dismiss
    Button *backButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
    backButton->border=10;
    backButton->tapCallback=[](void * obj){ LaunchWatchface(); }; // callback when tap
    // load icon in XBM format
    XBM * backButtonIcon = new XBM(img_backscreen_24_width,img_backscreen_24_height,img_backscreen_24_bits,true);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.35);
    bottomButtonContainer->AddChild(nullptr,1.65);


    //viewContainer->AddChild(new XBM(img_smartwatch_160_width,img_smartwatch_160_height,img_smartwatch_160_bits,true));

    ScreenRotateEvent = new EventKVO([&, this](){
        lUILog("User screen rotation: %u\n", bmaRotation);
        if( xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_TIME_TICKS) == pdTRUE )  {
            ttgo->tft->setRotation(BMAtoTFTOrientation(bmaRotation));
            this->canvas->pushSprite(0,0);
            xSemaphoreGive( UISemaphore );
        }
        //this->child->dirty=true;
    },BMA_EVENT_DIRECTION);
/*
bmaRotation <-> tftRotation
0 = led right = 3
1 = led left  = 1
2 = led up    = 0
3 = led down  = 2

*/
    AddChild(screen);
    directDraw=true; // allow controls to direct redraw itself instead of push whole view Sprite
    // Thats all! the app is running
}

/*
bool LuiExperimentApplication::Tick() {
    TemplateLuIApplication::EventHandler();
    bool mustPush0 = app0->Tick();
    if ( mustPush0 ) {
        TFT_eSprite * buffImg0 = testBuffer0->GetBuffer();
        app0->canvas->setPivot(0,0);
        buffImg0->setPivot(0,0);
        app0->canvas->pushRotated(buffImg0,0);
        testBuffer0->dirty=true;
    }
    bool mustPush1 = app1->Tick();
    if ( mustPush1 ) {
        TFT_eSprite * buffImg1 = testBuffer1->GetBuffer();
        app1->canvas->setPivot(0,0);
        buffImg1->setPivot(0,0);
        app1->canvas->pushRotated(buffImg1,0);
        testBuffer1->dirty=true;
    }

    return false; // is directDraw application
}
*/