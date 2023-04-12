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
#include "LuiTwoApps.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/View3D/View3D.hpp"
#include "../UI/controls/Buffer.hpp"
#include "LogView.hpp"
#include "../resources.hpp"

#include "Steps.hpp"
#include "WatchfaceDot.hpp"

using namespace LuI;

LuiTwoAppsApplication::~LuiTwoAppsApplication() {
    if ( nullptr != app0 ) { delete app0; }
    if ( nullptr != app1 ) { delete app1; }
}

LuiTwoAppsApplication::LuiTwoAppsApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    // fill view with background color
    canvas->fillSprite(ThCol(background)); // use theme colors

    /// create root container with two slots horizontal
    Container * screen = new Container(LuI_Horizontal_Layout,2);
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,2);

    // main view space
    Container * viewContainer = new Container(LuI_Vertical_Layout,2);

    // if use quota, all the slot quotas must sum equal the total number of elements
    // example: default quota in 2 controls result 50/50% of view space with 1.0 of quota everyone (2.0 in total)

    // add main view with quota of 1.7
    screen->AddChild(viewContainer,1.65);
    // add bottom button bar shirnked
    screen->AddChild(bottomButtonContainer,0.35);
    // 1.7 + 0.3 = 2.0 of quota (fine for 2 slots)

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


    // app 0 on buffer0
    app0 = new StepsApplication();
    testBuffer0=new Buffer(TFT_WIDTH,TFT_HEIGHT);
    viewContainer->AddChild(testBuffer0);
    // app 1 on buffer1
    app1 = new WatchfaceDotApplication();
    testBuffer1=new Buffer(TFT_WIDTH,TFT_HEIGHT);
    viewContainer->AddChild(testBuffer1);


    AddChild(screen);
    directDraw=true; // allow controls to direct redraw itself instead of push whole view Sprite
    // Thats all! the app is running
}


bool LuiTwoAppsApplication::Tick() {
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
