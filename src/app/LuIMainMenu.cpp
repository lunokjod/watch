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

#include <LilyGoWatch.h>

#include "../resources.hpp"

#include "Battery.hpp"
#include "Notifications.hpp"
#include "About.hpp"
#include "Steps.hpp"
#include "Brightness.hpp"
#include "Activities.hpp"
#include "Calculator.hpp"
#include "Stopwatch.hpp"
#include "LuISettingsMenu.hpp"
#include "Calendar.hpp"
//#include "LuiExperiment.hpp"
//#include "LuITest.hpp"
//#include "LuIDemoRubiks.hpp"
//#include "ScreenTest.hpp"
#include "KnowLocations.hpp"
#include "LuiGamesMenu.hpp"
//#include "LuaLauncher.hpp"
//#include "FileBrowser.hpp"
#ifdef LUNOKIOT_LILYGO_TWATCH_BUTTON_FAULTY
#include "Lamp.hpp"
#endif
//#include "PartitionBrowser.hpp"
//#include "LunoNoid.hpp"
//#include "Dungeon/Dungeon.hpp"
//#include "LuiChess.hpp"
#include "MostUsedApps.hpp"
using namespace LuI;
const IconMenuEntry LuIMenuItems[] = {
    
    {"Back", img_mainmenu_back_bits, img_mainmenu_back_height, img_mainmenu_back_width, [](IGNORE_PARAM) { LaunchWatchface(); } },

//    {"MostUsed",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new LuIMostUsedApplication()); } },
//    {"Chess",img_mainmenu_chess_bits, img_mainmenu_chess_height, img_mainmenu_chess_width, [](IGNORE_PARAM) { LaunchApplication(new ChessApplication()); } },
//    {"Log",img_mainmenu_cpu_bits, img_mainmenu_cpu_height, img_mainmenu_cpu_width, [](void *unused) { LaunchApplication(new LogViewApplication()); } },

//    {"Partitions",img_mainmenu_partition_bits, img_mainmenu_partition_height, img_mainmenu_partition_width, [](IGNORE_PARAM) { LaunchApplication(new PartitionExplorerApplication()); } },
//    {"FileBrowser",img_mainmenu_folder_bits, img_mainmenu_folder_height, img_mainmenu_folder_width, [](IGNORE_PARAM) { LaunchApplication(new FileExplorerApplication()); } },
//    {"Calendar",img_mainmenu_calendar_bits, img_mainmenu_calendar_height, img_mainmenu_calendar_width, [](IGNORE_PARAM) { LaunchApplication(new CalendarApplication()); } },
    //{"Dungeon",img_mainmenu_dungeon_bits, img_mainmenu_dungeon_height, img_mainmenu_dungeon_width, [](void *unused) { LaunchApplication(new DungeonGameApplication()); } },
//    {"LunoNoid",img_mainmenu_lunonoid_bits, img_mainmenu_lunonoid_height, img_mainmenu_lunonoid_width, [](void *unused) { LaunchApplication(new LunoNoidGameApplication()); } },

//    {"Prov", img_mainmenu_provisioning_bits, img_mainmenu_provisioning_height, img_mainmenu_provisioning_width, [](void *unused) { LaunchApplication(new Provisioning2Application()); } },

//    {"Lua",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [&](IGNORE_PARAM) { LaunchApplication(new LuaLauncher(HelloworldLuaScript)); } },
//    {"Screen",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [&](IGNORE_PARAM) { LaunchApplication(new ScreenTestApplication()); } },
//    {"Rubik's",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new LuIExperimentRubiksApplication()); } },
//    {"LuIDemos",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new LuiExperimentApplication()); } },
    {"Bright",img_mainmenu_bright_bits, img_mainmenu_bright_height, img_mainmenu_bright_width, [](IGNORE_PARAM) { LaunchApplication(new BrightnessApplication()); } },
#ifdef LUNOKIOT_LILYGO_TWATCH_BUTTON_FAULTY
    {"Lamp",img_mainmenu_lamp_bits, img_mainmenu_lamp_height, img_mainmenu_lamp_width, [](IGNORE_PARAM) { LaunchApplication(new LampApplication()); } },
#endif
    {"Notify", img_mainmenu_notifications_bits, img_mainmenu_notifications_height, img_mainmenu_notifications_width, [](IGNORE_PARAM) { LaunchApplication(new NotificacionsApplication()); } },
    {"Steps",img_mainmenu_steps_bits, img_mainmenu_steps_height, img_mainmenu_steps_width, [](IGNORE_PARAM) { LaunchApplication(new StepsApplication()); } },
    {"Battery",img_mainmenu_battery_bits, img_mainmenu_battery_height, img_mainmenu_battery_width,  [](IGNORE_PARAM) { LaunchApplication(new BatteryApplication()); } },
    {"Stopwatch",img_mainmenu_stopwatch_bits, img_mainmenu_stopwatch_height, img_mainmenu_stopwatch_width, [](IGNORE_PARAM) { LaunchApplication(new StopwatchApplication()); } },
    {"Settings",img_mainmenu_options_bits, img_mainmenu_options_height, img_mainmenu_options_width, [](IGNORE_PARAM) { LaunchApplication(new LuISettingsMenuApplication()); } },
    {"Locations",img_mainmenu_zone_bits, img_mainmenu_zone_height, img_mainmenu_zone_width, [](IGNORE_PARAM) { LaunchApplication(new KnowLocationApplication()); } },
    {"Calendar",img_mainmenu_calendar_bits, img_mainmenu_calendar_height, img_mainmenu_calendar_width, [](IGNORE_PARAM) { LaunchApplication(new CalendarApplication()); } },
    {"Calculator",img_mainmenu_calculator_bits, img_mainmenu_calculator_height, img_mainmenu_calculator_width, [](IGNORE_PARAM) { LaunchApplication(new CalculatorApplication()); } },
    {"Games",img_mainmenu_games_bits, img_mainmenu_games_height, img_mainmenu_games_width, [](IGNORE_PARAM) { LaunchApplication(new LuIGamesMenuApplication()); } },
    {"About",img_mainmenu_about_bits, img_mainmenu_about_height, img_mainmenu_about_width, [](IGNORE_PARAM) { LaunchApplication(new AboutApplication()); } },
};
int LuIMenuItemsNumber = sizeof(LuIMenuItems) / sizeof(LuIMenuItems[0])-1;


LuIMainMenuApplication::LuIMainMenuApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    canvas->fillSprite(TFT_BLACK);
    Container * screen = new Container(LuI_Horizontal_Layout,3); // up pager, center icon, bottom text
    paginator=new Paginator(LuIMenuItemsNumber);
    paginator->border=40;
    paginator->SetBackgroundColor(TFT_BLACK);
    screen->AddChild(paginator,0.5);
    mainMenu = new IconMenu(LuI_Horizontal_Layout,LuIMenuItemsNumber,LuIMenuItems);
    screen->AddChild(mainMenu,2.0);
    entryText=new Text((char*)"",TFT_WHITE,false,1,&FreeMonoBold18pt7b);
    entryText->SetBackgroundColor(TFT_BLACK);
    screen->AddChild(entryText,0.5);
    mainMenu->pageCallback = [&,this](IGNORE_PARAM){
        paginator->SetCurrent(mainMenu->selectedEntry); // update paginator
        entryText->SetText((char*)(LuIMenuItems[mainMenu->selectedEntry].name)); // update text
    };
    // update position later to clarify code
    const int firstOffset=1;
    mainMenu->selectedEntry=firstOffset; // don't show "back" as first option
    paginator->SetCurrent(firstOffset);
    entryText->SetText((char*)LuIMenuItems[firstOffset].name);
    AddChild(screen);

    directDraw=true; // allow controls to sync itself with the screen
}
