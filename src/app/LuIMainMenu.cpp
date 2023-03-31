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
#include "LuIMainMenu.hpp"
#include "../UI/controls/IconMenu.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Paginator.hpp"
#include "../UI/controls/Text.hpp"
#include "LogView.hpp"

#include "../system/Datasources/kvo.hpp"
#include "../system/SystemEvents.hpp"

#include <LilyGoWatch.h>

#include "../resources.hpp"
#include "SettingsMenu.hpp"
#include "Battery.hpp"
#include "Notifications.hpp"
#include "About.hpp"
#include "Steps.hpp"
#include "Brightness.hpp"
#include "Activities.hpp"
#include "Calculator.hpp"
#include "Stopwatch.hpp"
#include "Settings.hpp"
#include "Calendar.hpp"
#include "LuiExperiment.hpp"

#include "LogView.hpp"

using namespace LuI;

const IconMenuEntry LuIMainMenuItems[] = {
    {"Back", img_mainmenu_back_bits, img_mainmenu_back_height, img_mainmenu_back_width, [](IGNORE_PARAM) { LaunchWatchface(); } },
//    {"LuI",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new LuiExperimentApplication()); } },
    {"Bright",img_mainmenu_bright_bits, img_mainmenu_bright_height, img_mainmenu_bright_width, [](IGNORE_PARAM) { LaunchApplication(new BrightnessApplication()); } },
    {"Notify", img_mainmenu_notifications_bits, img_mainmenu_notifications_height, img_mainmenu_notifications_width, [](IGNORE_PARAM) { LaunchApplication(new NotificacionsApplication()); } },
    {"Steps",img_mainmenu_steps_bits, img_mainmenu_steps_height, img_mainmenu_steps_width, [](IGNORE_PARAM) { LaunchApplication(new StepsApplication()); } },
    {"Battery",img_mainmenu_battery_bits, img_mainmenu_battery_height, img_mainmenu_battery_width,  [](IGNORE_PARAM) { LaunchApplication(new BatteryApplication()); } },
    {"Stopwatch",img_mainmenu_stopwatch_bits, img_mainmenu_stopwatch_height, img_mainmenu_stopwatch_width, [](IGNORE_PARAM) { LaunchApplication(new StopwatchApplication()); } },
    {"Settings",img_mainmenu_options_bits, img_mainmenu_options_height, img_mainmenu_options_width, [](IGNORE_PARAM) { LaunchApplication(new SettingsMenuApplication()); } },
    {"Calendar",img_mainmenu_calendar_bits, img_mainmenu_calendar_height, img_mainmenu_calendar_width, [](IGNORE_PARAM) { LaunchApplication(new CalendarApplication()); } },
    {"Calculator",img_mainmenu_calculator_bits, img_mainmenu_calculator_height, img_mainmenu_calculator_width, [](IGNORE_PARAM) { LaunchApplication(new CalculatorApplication()); } },
    {"About",img_mainmenu_about_bits, img_mainmenu_about_height, img_mainmenu_about_width, [](IGNORE_PARAM) { LaunchApplication(new AboutApplication()); } },
};
int LuIMainMenuItemsNumber = sizeof(LuIMainMenuItems) / sizeof(LuIMainMenuItems[0])-1;


LuIMainMenuApplication::LuIMainMenuApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    canvas->fillSprite(TFT_BLACK);
    Container * screen = new Container(LuI_Horizonal_Layout,3); // up pager, center icon, bottom text
    paginator=new Paginator(LuIMainMenuItemsNumber);
    paginator->border=40;
    paginator->SetBackgroundColor(TFT_BLACK);
    screen->AddChild(paginator,0.5);
    mainMenu = new IconMenu(LuI_Horizonal_Layout,LuIMainMenuItemsNumber,LuIMainMenuItems);
    screen->AddChild(mainMenu,2.0);
    entryText=new Text("",TFT_WHITE,false,1,&FreeMonoBold18pt7b);
    entryText->SetBackgroundColor(TFT_BLACK);
    screen->AddChild(entryText,0.5);
    mainMenu->pageCallbackParam=this;
    mainMenu->pageCallback = [](void * obj){
        LuIMainMenuApplication * self = (LuIMainMenuApplication *)obj;
        self->paginator->SetCurrent(self->mainMenu->selectedEntry); // update paginator
        self->entryText->SetText((char*)(LuIMainMenuItems[self->mainMenu->selectedEntry].name)); // update text
    };
    // update position later to clarify code
    const int firstOffset=1;
    mainMenu->selectedEntry=firstOffset; // don't show "back" as first option
    paginator->current=firstOffset;
    entryText->SetText((char*)LuIMainMenuItems[firstOffset].name);
    AddChild(screen);

    directDraw=true; // allow controls to sync itself with the screen
}
