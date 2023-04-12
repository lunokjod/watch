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

#include "ScreenTest.hpp"
#include <LilyGoWatch.h>

#include "LogView.hpp"

#include "../UI/UI.hpp"
#include "../resources.hpp"

using namespace LuI;

extern TFT_eSPI * tft;

ScreenTestApplication::ScreenTestApplication() {
    // fill things
    //canvas->setColorDepth(8);
    canvas->setSwapBytes(true);
    canvas->pushImage(0,0,testImageBackground.width,testImageBackground.height,(const uint16_t *)testImageBackground.pixel_data);
    canvas->setSwapBytes(false);
    //canvas->pushImage(30,30,canvas->width(),canvas->height(),(const uint16_t *)canvas->frameBuffer(0));

    directDraw=true; // allow controls to direct redraw itself instead of push whole view Sprite
}

ScreenTestApplication::~ScreenTestApplication() {
    /*
    if ( nullptr != backgroundFrom ) { 
        if ( backgroundFrom->created() ) { backgroundFrom->deleteSprite(); }
        delete backgroundFrom;
        backgroundFrom=nullptr;
    }*/
}

bool ScreenTestApplication::Tick() {
    //lAppLog("System time: %lu\n",millis()-lastTickMS);
    unsigned long bTick = millis();

    // do a battery tests
    //tft->fillScreen(TFT_BLACK);
    unsigned long bPushFullScreen = millis();
    canvas->pushSprite(0,0); // 33milis
    lAppLog("Push fullscreen time: %lu\n",millis()-bPushFullScreen);

    //lAppLog("Tick time: %lu\n",millis()-bTick);
    lastTickMS=millis();
    return false;
}