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
#include "../UI/controls/Check.hpp"
#include "../UI/controls/IconMenu.hpp"
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/View3D/View3D.hpp"
#include "../UI/controls/Buffer.hpp"
#include "LogView.hpp"
#include "../UI/UI.hpp"
#include <LilyGoWatch.h>
using namespace LuI;
#include "../resources.hpp"

#include "LuIDemoPlanet.hpp"
#include "LuIGameOfLife.hpp"
#include "LuIDemoMask.hpp"

extern TFT_eSPI * tft;
#include "../../static/img_worldcollide_160.c"


const IconMenuEntry LuIExperimentsMenuItems[] = {
    {"Home",img_home_64_bits, img_home_64_height, img_home_64_width, [](void *obj) {
        LuiExperimentApplication * self = (LuiExperimentApplication *)obj;
        self->bufferInfo->offsetX=0;
        self->bufferInfo->offsetY=0;
        TFT_eSprite * myView = self->bufferInfo->GetBuffer();
        myView->fillSprite(ByteSwap(ThCol(background)));

        myView->setFreeFont(&FreeMonoBold12pt7b);
        myView->setTextColor(ByteSwap(ThCol(text)));
        myView->setTextSize(1);
        myView->setTextDatum(TC_DATUM);
        myView->drawString("Welcome LuI Demos",myView->width()/2,5);
        myView->setFreeFont(&Picopixel);
        myView->setTextSize(2);
        myView->setTextColor(ByteSwap(ThCol(text_alt)));
        myView->drawString("a lightweight UI for esp32",myView->width()/2,26);
        myView->drawXBitmap((myView->width()-lunokiot_logo_32_width)/2,40,lunokiot_logo_32_bits, lunokiot_logo_32_width,lunokiot_logo_32_height, ByteSwap(TFT_LIGHTGREY));

        myView->setTextColor(ByteSwap(ThCol(text)));
        myView->setTextDatum(TL_DATUM);
        myView->setTextSize(1);
        myView->setFreeFont(&FreeMonoBold9pt7b);
        myView->drawString("This",5,90);
        myView->setFreeFont(&FreeMono9pt7b);
        myView->drawString("application is a",60,86);
        myView->drawString("example itself about",5,102);
        myView->setTextColor(ByteSwap(TFT_GREEN));
        myView->drawString("how to use LuI layout",5,118);

        myView->setTextColor(ByteSwap(ThCol(text)));
        myView->drawString("Availiable controls:",5,134);

        myView->setTextDatum(TC_DATUM);
        myView->setFreeFont(&FreeMonoBold9pt7b);
        int column=myView->width()/2;
        int offs=150;
        myView->drawString("Containers",column,offs);
        offs+=16;
        myView->drawString("Text",column,offs);
        offs+=16;
        myView->drawString("Button",column,offs);
        offs+=16;
        myView->drawString("Check",column,offs);
        offs+=16;
        myView->drawString("XBM image",column,offs);
        offs+=16;
        myView->drawString("RGB565 image",column,offs);
        offs+=16;
        myView->drawString("Buffer",column,offs);
        offs+=16;
        myView->drawString("IconMenu",column,offs);
        offs+=16;
        myView->drawString("Paginator",column,offs);
        offs+=16;
        myView->drawString("View3D",column,offs);


        self->bufferInfo->dirty=true;

    } },
    { "Planet",img_planet_64_bits, img_planet_64_height, img_planet_64_width, [](void *obj) {
        LuiExperimentApplication * self = (LuiExperimentApplication *)obj;
        TFT_eSprite * myView = self->bufferInfo->GetBuffer();
        self->bufferInfo->offsetX=0;
        self->bufferInfo->offsetY=0;
        myView->fillSprite(ByteSwap(ThCol(background)));

        myView->setFreeFont(&FreeMonoBold12pt7b);
        myView->setTextColor(ByteSwap(ThCol(text)));
        myView->setTextSize(1);
        myView->setTextDatum(TC_DATUM);
        myView->drawString("Moon collision",myView->width()/2,0);
        myView->drawString("Demo",myView->width()/2,24);

        myView->pushImage((myView->width()-img_worldcollide_160.width)/2,52,img_worldcollide_160.width,img_worldcollide_160.height, (uint16_t *)img_worldcollide_160.pixel_data);

        myView->setFreeFont(&FreeMonoBold9pt7b);
        myView->drawString("This example exposes",5,140);
        myView->drawString("the View3D and",5,156);
        myView->drawString("rendering steps",5,172);
        myView->drawString("callbacks usage",5,188);

        myView->setFreeFont(&FreeMono9pt7b);
        int offs=208;
        myView->drawString("View3D capabilities",5,offs);
        offs+=16;
        myView->drawString("Materials and colors",5,offs);
        offs+=16;
        myView->drawString("Render modes,Bilboards",5,offs);
        offs+=16;
        myView->drawString("Load multiple meshes",5,offs);
        offs+=16;
        myView->drawString("Control camera",5,offs);
        offs+=16;
        myView->drawString("Render step callbacks",5,offs);
        offs+=16;
        myView->drawString("Animation callbacks",5,offs);

        self->bufferInfo->dirty=true;
    } },
    { "Glider",img_glider_64_bits, img_glider_64_height, img_glider_64_width, [](void *obj) {
        LuiExperimentApplication * self = (LuiExperimentApplication *)obj;
        TFT_eSprite * myView = self->bufferInfo->GetBuffer();
        self->bufferInfo->offsetX=0;
        self->bufferInfo->offsetY=0;
        myView->fillSprite(ByteSwap(ThCol(background)));

        myView->setFreeFont(&FreeMonoBold12pt7b);
        myView->setTextColor(ByteSwap(ThCol(text)));
        myView->setTextSize(1);
        myView->setTextDatum(TC_DATUM);
        myView->drawString("Conway",myView->width()/2,0);
        myView->drawString("Game of life",myView->width()/2,24);
        myView->setFreeFont(&FreeMonoBold9pt7b);
        myView->setTextSize(1);
        myView->drawString("Try to draw a glider!",5,140);

        self->bufferInfo->dirty=true;
    } },
    { "3dMask",img_cube_64_bits, img_cube_64_height, img_cube_64_width, [](void *obj) {
        LuiExperimentApplication * self = (LuiExperimentApplication *)obj;
        TFT_eSprite * myView = self->bufferInfo->GetBuffer();
        self->bufferInfo->offsetX=0;
        self->bufferInfo->offsetY=0;
        myView->fillSprite(ByteSwap(ThCol(background)));

        myView->setFreeFont(&FreeMonoBold12pt7b);
        myView->setTextColor(ByteSwap(ThCol(text)));
        myView->setTextSize(1);
        myView->setTextDatum(TC_DATUM);
        myView->drawString("3DMask",myView->width()/2,0);
        myView->drawString("Demo",myView->width()/2,24);
        self->bufferInfo->dirty=true;
    } }
};
int LuIExperimentsMenuItemsNumber = sizeof(LuIExperimentsMenuItems) / sizeof(LuIExperimentsMenuItems[0])-1;

LuiExperimentApplication::LuiExperimentApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    // fill view with background color
    canvas->fillSprite(ThCol(background_alt)); // use theme colors

    /// create root container with two slots horizontal
    Container * screen = new Container(LuI_Horizontal_Layout,2);
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,2);

    // main view space
    Container * viewContainer = new Container(LuI_Horizontal_Layout,1);
    //viewContainer->border=10;
    // if use quota, all the slot quotas must sum equal the total number of elements
    // example: default quota in 2 controls result 50/50% of view space with 1.0 of quota everyone (2.0 in total)

    screen->AddChild(viewContainer,1.68);
    // add bottom button bar shirnked
    screen->AddChild(bottomButtonContainer,0.32);

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

    Button *launchButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
    launchButton->border=10;
    //launchButton->AddChild(nullptr);
    launchButton->AddChild(new Text("Launch",TFT_YELLOW));
    launchButton->tapCallback=[&](void * obj){
        IconMenu * menu = (IconMenu *)obj;
        // force the opened app to back to me on exit
        UICallback pleaseBackToMe = [](void * obj){
            LaunchApplication(new LuiExperimentApplication());
        };
        
        if ( 1 == menu->selectedEntry ) {
            LuIExperimentPlanetApplication * app = new LuIExperimentPlanetApplication();
            app->backButton->tapCallback=pleaseBackToMe;
            LaunchApplication(app);
        }
        else if ( 2 == menu->selectedEntry ) {
            LuIDemoGameOfLifeApplication * app = new LuIDemoGameOfLifeApplication();
            app->backButton->tapCallback=pleaseBackToMe;
            LaunchApplication(app);
        }
        else if ( 3 == menu->selectedEntry ) {
            LuIExperimentMaskApplication * app = new LuIExperimentMaskApplication();
            app->backButton->tapCallback=pleaseBackToMe;
            LaunchApplication(app);
        }

        //lLog("LAUNCH!!!\n");
        //launchButton->dirty=true;
    }; // callback when tap
    bottomButtonContainer->AddChild(launchButton,1.65);

    // here the main view
    Container * mainDiv = new Container(LuI_Horizontal_Layout,2);

    //Container * menuDiv = new Container(LuI_Vertical_Layout,1);
    demoMenu = new IconMenu(LuI_Horizontal_Layout,LuIExperimentsMenuItemsNumber,LuIExperimentsMenuItems);
    demoMenu->tapCallback=nullptr; // dont launch on tap
    demoMenu->showCatalogue=true; // show other icons
    demoMenu->backgroundColor=ThCol(background_alt);
    demoMenu->pageCallbackParam=this;
    demoMenu->tapCallbackParam=this;
    demoMenu->pageCallback = [](void * obj){
        LuiExperimentApplication * self = (LuiExperimentApplication *)obj;
        //lLog("AAUUUU: %d\n",self->demoMenu->selectedEntry);
        (LuIExperimentsMenuItems[self->demoMenu->selectedEntry].callback)(self->demoMenu->tapCallbackParam);
    };
    launchButton->tapCallbackParam=demoMenu; // for launchButton


    //menuDiv->AddChild(new Text((char*)"<",ThCol(text),false,1,&FreeMonoBold24pt7b),0.2);
    //menuDiv->AddChild(demoMenu,2.6);
    //menuDiv->AddChild(new Text((char*)">",ThCol(text),false,1,&FreeMonoBold24pt7b),0.2);

    mainDiv->AddChild(demoMenu,0.8);
    bufferInfo = new Buffer(240,320);
    mainDiv->AddChild(bufferInfo,1.2);
    viewContainer->AddChild(mainDiv);

    AddChild(screen);
    
    // fill default first page
    (LuIExperimentsMenuItems[0].callback)(demoMenu->tapCallbackParam);

    directDraw=true; // allow controls to direct redraw itself instead of push whole view Sprite
    // Thats all! the app is running
}
