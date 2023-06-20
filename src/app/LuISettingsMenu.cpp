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
#include "LuISettingsMenu.hpp"
#include "../UI/controls/IconMenu.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Paginator.hpp"
#include "../UI/controls/Text.hpp"
#include "LogView.hpp"

#include <LilyGoWatch.h>

#include "../resources.hpp"
#include "LuIMainMenu.hpp"

#include "Settings.hpp"
#include "Battery.hpp"
#include "Notifications.hpp"
#include "LuIRotation.hpp"
#include "Provisioning2.hpp"
#include "SetTime.hpp"
#include "SetTimeZone.hpp"
#include "Brightness.hpp"
#include "Advanced.hpp"
#include "Bluetooth.hpp"
#include "BLEMonitor.hpp"
#include "Theme.hpp"
#include "FreehandKeyboardSetup.hpp"
#include "GestureSettings.hpp"
#include "WatchfaceSelector.hpp"
#include "FileBrowser.hpp"
#include "PartitionBrowser.hpp"

using namespace LuI;
const IconMenuEntry LuISettingsMenuItems[] = {
    {"Back", img_mainmenu_watchface_bits, img_mainmenu_watchface_height, img_mainmenu_watchface_width, [](IGNORE_PARAM) { LaunchWatchface(); } },
    {"Back", img_mainmenu_back_bits, img_mainmenu_back_height, img_mainmenu_back_width, [](IGNORE_PARAM) { LaunchApplication(new LuIMainMenuApplication()); } },
    {"Radios",img_mainmenu_wifi_bits, img_mainmenu_wifi_height, img_mainmenu_wifi_width, [](void *unused) { LaunchApplication(new SettingsApplication()); } },
    {"Gestures",img_mainmenu_gesture_bits, img_mainmenu_gesture_height, img_mainmenu_gesture_width, [](void *unused) { LaunchApplication(new GestureSettings()); } },
    {"Watchface",img_mainmenu_watchface_bits, img_mainmenu_watchface_height, img_mainmenu_watchface_width, [](void *unused) { LaunchApplication(new WatchfaceSelectorApplication()); } },
    {"Themes",img_mainmenu_themes_bits, img_mainmenu_themes_height, img_mainmenu_themes_width, [](void *unused) { LaunchApplication(new ThemeApplication()); } },
    {"Pair",img_mainmenu_bluetooth_bits, img_mainmenu_bluetooth_height, img_mainmenu_bluetooth_width, [](void *unused) { LaunchApplication(new BluetoothApplication()); } },
    {"Advanced",img_mainmenu_cpu_bits, img_mainmenu_cpu_height, img_mainmenu_cpu_width, [](void *unused) { LaunchApplication(new AdvancedSettingsApplication()); } },
    //{"LittleFS",img_mainmenu_folder_bits, img_mainmenu_folder_height, img_mainmenu_folder_width, [](IGNORE_PARAM) { LaunchApplication(new FileExplorerApplication()); } },
    {"Keyboard",img_mainmenu_keyboard_bits, img_mainmenu_keyboard_height, img_mainmenu_keyboard_width, [](void *unused) { LaunchApplication(new FreeHandKeyboardSetupApplication()); } },
    {"BLEMonitor",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new BLEMonitorApplication()); } },
    {"Set time", img_mainmenu_settime_bits, img_mainmenu_settime_height, img_mainmenu_settime_width, [](void *unused) { LaunchApplication(new SetTimeApplication()); } },
    {"Timezone",img_timezone_120_bits, img_timezone_120_height, img_timezone_120_width, [](void *unused) { LaunchApplication(new SetTimeZoneApplication()); } },
    {"Prov", img_mainmenu_provisioning_bits, img_mainmenu_provisioning_height, img_mainmenu_provisioning_width, [](void *unused) { LaunchApplication(new Provisioning2Application()); } },
    {"Rotation", img_rotate_120_bits, img_rotate_120_height, img_rotate_120_width, [](void *unused) { LaunchApplication(new LuIRotateApplication()); } },
    {"Partitions",img_mainmenu_partition_bits, img_mainmenu_partition_height, img_mainmenu_partition_width, [](IGNORE_PARAM) { LaunchApplication(new PartitionExplorerApplication()); } },
    {"Bright",img_mainmenu_bright_bits, img_mainmenu_bright_height, img_mainmenu_bright_width, [](void *unused) { LaunchApplication(new BrightnessApplication()); } },
};
int LuISettingsMenuItemsNumber = sizeof(LuISettingsMenuItems) / sizeof(LuISettingsMenuItems[0])-1;


LuISettingsMenuApplication::LuISettingsMenuApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    canvas->fillSprite(TFT_BLACK);
    Container * screen = new Container(LuI_Horizontal_Layout,3); // up pager, center icon, bottom text
    paginator=new Paginator(LuISettingsMenuItemsNumber);
    paginator->border=40;
    paginator->SetBackgroundColor(TFT_BLACK);
    screen->AddChild(paginator,0.5);
    mainMenu = new IconMenu(LuI_Horizontal_Layout,LuISettingsMenuItemsNumber,LuISettingsMenuItems);
    screen->AddChild(mainMenu,2.0);
    entryText=new Text((char*)"",TFT_WHITE,false,1,&FreeMonoBold18pt7b);
    entryText->SetBackgroundColor(TFT_BLACK);
    screen->AddChild(entryText,0.5);
    mainMenu->pageCallbackParam=this;
    mainMenu->pageCallback = [](void * obj){
        LuISettingsMenuApplication * self = (LuISettingsMenuApplication *)obj;
        self->paginator->SetCurrent(self->mainMenu->selectedEntry); // update paginator
        self->entryText->SetText((char*)(LuISettingsMenuItems[self->mainMenu->selectedEntry].name)); // update text
    };
    // update position later to clarify code
    const int firstOffset=2;
    mainMenu->selectedEntry=firstOffset; // don't show "back" as first option
    paginator->current=firstOffset;
    entryText->SetText((char*)LuISettingsMenuItems[firstOffset].name);
    AddChild(screen);

    directDraw=true; // allow controls to sync itself with the screen
}
