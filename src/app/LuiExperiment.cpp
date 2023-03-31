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


#include "../system/Datasources/kvo.hpp"
#include "../system/SystemEvents.hpp"
#include <ArduinoNvs.h>

#include <LilyGoWatch.h>
using namespace LuI;
#include "../resources.hpp"


const IconMenuEntry TestIcons[] = {
//    {"Back", img_mainmenu_back_bits, img_mainmenu_back_height, img_mainmenu_back_width, [](IGNORE_PARAM) { LaunchWatchface(); } },
    {"Bright",img_mainmenu_bright_bits, img_mainmenu_bright_height, img_mainmenu_bright_width, [](IGNORE_PARAM) {  } },
    {"Notify", img_mainmenu_notifications_bits, img_mainmenu_notifications_height, img_mainmenu_notifications_width, [](IGNORE_PARAM) {  } },
    {"Steps",img_mainmenu_steps_bits, img_mainmenu_steps_height, img_mainmenu_steps_width, [](IGNORE_PARAM) {  } },
    {"Battery",img_mainmenu_battery_bits, img_mainmenu_battery_height, img_mainmenu_battery_width,  [](IGNORE_PARAM) {  } },
    {"Stopwatch",img_mainmenu_stopwatch_bits, img_mainmenu_stopwatch_height, img_mainmenu_stopwatch_width, [](IGNORE_PARAM) {  } },
    {"Settings",img_mainmenu_options_bits, img_mainmenu_options_height, img_mainmenu_options_width, [](IGNORE_PARAM) {  } },
    {"Calendar",img_mainmenu_calendar_bits, img_mainmenu_calendar_height, img_mainmenu_calendar_width, [](IGNORE_PARAM) {  } },
    {"Calculator",img_mainmenu_calculator_bits, img_mainmenu_calculator_height, img_mainmenu_calculator_width, [](IGNORE_PARAM) {  } },
    {"Lamp",img_mainmenu_lamp_bits, img_mainmenu_lamp_height, img_mainmenu_lamp_width, [](IGNORE_PARAM) {  } },
    {"About",img_mainmenu_about_bits, img_mainmenu_about_height, img_mainmenu_about_width, [](IGNORE_PARAM) {  } },
};
int TestIconsNumber = sizeof(TestIcons) / sizeof(TestIcons[0])-1;


LuiExperimentApplication::LuiExperimentApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    // fill view with background color
    canvas->fillSprite(TFT_BLACK); // use theme colors

    /// create root container with two slots horizontal
    Container * screen = new Container(LuI_Horizonal_Layout,2);
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,2);

    // main view space
    Container * viewContainer = new Container(LuI_Horizonal_Layout,1);
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
    XBM * backButtonIcon = new XBM(img_backscreen_24_width,img_backscreen_24_height,img_backscreen_24_bits,true);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.35);
    bottomButtonContainer->AddChild(nullptr,1.65);

    IconMenu * testMenu0 = new IconMenu(LuI_Horizonal_Layout,TestIconsNumber,TestIcons);
    //IconMenu * testMenu1 = new IconMenu(LuI_Vertical_Layout,TestIconsNumber,TestIcons);
    viewContainer->AddChild(testMenu0);
    //viewContainer->AddChild(testMenu1);
    AddChild(screen);
    directDraw=true; // allow controls to direct redraw itself instead of push whole view Sprite
    // Thats all! the app is running
}
