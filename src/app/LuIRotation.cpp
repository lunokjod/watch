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
#include "LuIRotation.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/UI.hpp"
#include "../resources.hpp"
#include "../system/Datasources/kvo.hpp"
#include "../system/SystemEvents.hpp"
#include <ArduinoNvs.h>
#include <LilyGoWatch.h>


using namespace LuI;

extern uint8_t bmaRotation;
extern SemaphoreHandle_t UISemaphore;
extern TFT_eSPI * tft;

LuIRotateApplication::~LuIRotateApplication() {
    if ( nullptr != ScreenRotateEvent ) { delete ScreenRotateEvent; }
    NVS.setInt("ScreenRot",tft->getRotation(),false);
}

uint8_t LuIRotateApplication::BMAtoTFTOrientation(uint8_t bma) {
    /*
    bmaRotation <-> tftRotation
    0 = led right = 3
    1 = led left  = 1
    2 = led up    = 0
    3 = led down  = 2
    */
    if(0 == bma) { return 3; }
    else if(1 == bma) { return 1; }
    else if(2 == bma) { return 0; }
    else if(3 == bma) { return 2; }
    // 4 and 5 are up and down (ignored here)
    //lLog("UNKNOWN BMA!!! %u\n", bma);
    return -1;
}
void LuIRotateApplication::CleanPush(TFT_eSprite * frame, float angle) {
    const uint32_t frameDelay=20;
    //tft->fillScreen(TFT_BLACK);
    int16_t minX,minY,maxX,maxY;
    frame->getRotatedBounds(angle,&minX,&minY,&maxX,&maxY);
    // up
    tft->fillRect(0,0,canvas->width(),minY+1,TFT_BLACK);
    // left
    tft->fillRect(0,minY,minX,maxY,TFT_BLACK);
    // down
    tft->fillRect(0,maxY,canvas->width(),canvas->height()-maxY,TFT_BLACK);
    // right
    tft->fillRect(maxX,minY,canvas->width()-maxX,maxY,TFT_BLACK);
    frame->pushRotated(angle);

    delay(frameDelay);
}
void LuIRotateApplication::DoScreenRotation(uint8_t from, uint8_t to) {
    //lUILog("ANIMATE FROM rotation: %u next: %u\n", from, to);
    tft->setPivot(TFT_WIDTH/2,TFT_HEIGHT/2);
    canvas->setPivot(canvas->width()/2,canvas->height()/2);
    TFT_eSprite * snapshoot = ScaleSprite(canvas,0.8);
    CleanPush(snapshoot,0);
    snapshoot->deleteSprite();
    delete snapshoot;
    snapshoot = ScaleSprite(canvas,0.6);
    CleanPush(snapshoot,0);
    int16_t rotation=0;
    if ( 0 == from ) {
        if ( 1 == to ) { rotation=45;
        } else if ( 3 == to ) { rotation=-45; }
    } else if ( 3 == from ) {
        if ( 0 == to ) { rotation=45;
        } else if ( 2 == to ) { rotation=-45; }
    } else if ( 1 == from ) {
        if ( 2 == to ) { rotation=45;
        } else if ( 0 == to ) { rotation=-45; }
    } else if ( 2 == from ) {
        if ( 3 == to ) { rotation=45;
        } else if ( 1 == to ) { rotation=-45; }
    }
    CleanPush(snapshoot,rotation/2);
    CleanPush(snapshoot,rotation);
    CleanPush(snapshoot,rotation+(rotation/2));
    // 0 to 3/1
    // 3 to 0/2
    // 1 to 2/0
    // 2 to 3/1
    /*
    bmaRotation <-> tftRotation
    0 = led right = 3
    1 = led left  = 1
    2 = led up    = 0
    3 = led down  = 2
    */
    snapshoot->deleteSprite();
    delete(snapshoot);
}

LuIRotateApplication::LuIRotateApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    // fill view with background color
    canvas->fillSprite(ThCol(background)); // use theme colors

    /// create root container with two slots horizontal
    Container * screen = new Container(LuI_Horizontal_Layout,2);
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,2);

    // main view space
    Container * viewContainer = new Container(LuI_Horizontal_Layout,1);

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
    XBM * backButtonIcon = new XBM(img_backscreen_24_width,img_backscreen_24_height,img_backscreen_24_bits);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.35);
    bottomButtonContainer->AddChild(nullptr,1.65);

    // add smartwatch image to ilustrate the orientation
    Container * innerdiv = new Container(LuI_Horizontal_Layout,2);
    innerdiv->AddChild(new Text("This side up",TFT_WHITE,false,1,&FreeMonoBold12pt7b),0.6);
    innerdiv->AddChild(new Image(img_landscape_200.width,img_landscape_200.height,img_landscape_200.pixel_data,true),1.4);
    viewContainer->AddChild(innerdiv);

    ScreenRotateEvent = new EventKVO([&, this](){
        uint8_t lastRotation = tft->getRotation();
        uint8_t nextRotation = BMAtoTFTOrientation(bmaRotation);
        if ( 255 == nextRotation ) { return; }
        if( xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_TIME_TICKS) == pdTRUE )  {
            lUILog("User screen rotation: %u next: %u\n", lastRotation, nextRotation);
            DoScreenRotation(lastRotation,nextRotation);
            tft->setRotation(nextRotation);
            canvas->pushSprite(0,0); // oriented frame (tft need redraw to apply changes)
            xSemaphoreGive( UISemaphore );
        }
    },BMA_EVENT_DIRECTION);
    AddChild(screen);
    directDraw=true; // allow controls to direct redraw itself instead of push whole view Sprite
    // Thats all! the app is running
}
