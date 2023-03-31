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

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "MainMenu.hpp"
#include "../lunokiot_config.hpp"
#include "SettingsMenu.hpp"

#include "../resources.hpp"

#include "Battery.hpp"


#include "Notifications.hpp"

//#include "Rotation.hpp"
//#include "../static/img_rotate_120.xbm"

//#include "Provisioning2.hpp"
//#include "../static/img_mainmenu_provisioning.xbm"

//#include "SetTime.hpp"
//#include "../static/img_mainmenu_settime.xbm"

//#include "SetTimeZone.hpp"
//#include "../static/img_timezone_120.xbm"

#include "About.hpp"

#include "Steps.hpp"

#include "Brightness.hpp"

//#include "Advanced.hpp"
//#include "../static/img_mainmenu_cpu.xbm"

#include "Activities.hpp"

//#include "Bluetooth.hpp"
//#include "../static/img_mainmenu_bluetooth.xbm"


//#include "../static/img_mainmenu_chat.xbm"
//#include "PeerApplication.hpp"

#include "Calculator.hpp"

//#include "../static/img_mainmenu_dungeon.xbm"
//#include "Dungeon/Dungeon.hpp"
//#include "LockPicking.hpp"

#include "Stopwatch.hpp"

#include "Settings.hpp"

#ifdef LILYGO_WATCH_2020_V3
//#include "../static/img_mainmenu_mic.xbm"
//#include "Mic.hpp"
#endif

#ifdef LUNOKIOT_DEBUG_UI_DEMO
#include "Playground.hpp"
#include "Playground0.hpp"
#include "Playground1.hpp"
#include "Playground2.hpp"
#include "Playground3.hpp"
#include "Playground4.hpp"
#include "Playground5.hpp"
#include "Playground6.hpp"
#include "Playground7.hpp"
#include "Playground8.hpp"
#include "Playground9.hpp"
#include "Playground10.hpp"
#include "Playground11.hpp"
#include "Playground12.hpp"
#include "Playground13.hpp"
#endif

//#include "../static/img_mainmenu_update.xbm"
//#include "OTAUpdate.hpp"

//#include "../static/img_mainmenu_lamp.xbm"
//#include "Lamp.hpp"

#include "LogView.hpp"
//#include "../static/img_mainmenu_lastapps.xbm"
//#include "TaskSwitcher.hpp"

//#include "../static/img_mainmenu_keyboard.xbm"
//#include "FreehandKeyboardSetup.hpp"
//#include "BLEMonitor.hpp"

#include "Calendar.hpp"

//#include "../static/img_mainmenu_player.xbm"
//#include "BLEPlayer.hpp"

//#include "../static/img_mainmenu_watchface.xbm"
//#include "WatchfaceSelector.hpp"

//#include "BatteryLog.hpp"
//#include "WatchfaceAlwaysOn.hpp"
//#include "Engine3D.hpp"
//#include "ThumbTest.hpp"
//#include "GestureSettings.hpp"
//#include "Assistant.hpp"
#include "LuIDebug.hpp"
//#include "LuiBufferTest.hpp"
#include "LuIButtonsTest.hpp"
#include "LuiExperiment.hpp"

typedef struct {
    const char *name;
    const unsigned char * imagebits;
    int16_t height;
    int16_t width;
    UICallback callback;

} MainMenuApplicationEntry;

int32_t currentAppOffset = 0;
const MainMenuApplicationEntry AllApps[] = {
    {"Back", img_mainmenu_back_bits, img_mainmenu_back_height, img_mainmenu_back_width, [](IGNORE_PARAM) { LaunchWatchface(); } },
//    {"Rotate",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new LuIRotateApplication()); } },

    {"LuI",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new LuiExperimentApplication()); } },
//    {"LuIButtons",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new DebugLuIButtonsApplication()); } },
//    {"LuI3D",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new DebugLuIApplication()); } },
//    {"3DTEST",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new Engine3DApplication()); } },
//    {"LuIBuffer",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new LuiBufferTestApplication()); } },
//    {"Hello",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new AssistantApplication()); } },
//    {"BattLog",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new BatteryLogApplication()); } },
//    {"Select",img_mainmenu_watchface_bits, img_mainmenu_watchface_height, img_mainmenu_watchface_width, [](IGNORE_PARAM) { LaunchApplication(new WatchfaceSelectorApplication()); } },
//    {"Gestures",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new GestureSettings()); } },
//    {"Dungeon",img_mainmenu_dungeon_bits, img_mainmenu_dungeon_height, img_mainmenu_dungeon_width, [](IGNORE_PARAM) { LaunchApplication(new DungeonGameApplication()); } },
//    {"LockPick",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new LockPickingGameApplication()); } },
//    {"BLEMonitor",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new BLEMonitorApplication()); } },
//    {"Advanced",img_mainmenu_cpu_bits, img_mainmenu_cpu_height, img_mainmenu_cpu_width, [](IGNORE_PARAM) { LaunchApplication(new AdvancedSettingsApplication()); } },
//    {"Pair",img_mainmenu_bluetooth_bits, img_mainmenu_bluetooth_height, img_mainmenu_bluetooth_width, [](IGNORE_PARAM) { LaunchApplication(new BluetoothApplication()); } },
//    {"Keyboard",img_mainmenu_keyboard_bits, img_mainmenu_keyboard_height, img_mainmenu_keyboard_width, [](IGNORE_PARAM) { LaunchApplication(new FreeHandKeyboardSetupApplication()); } },
//    {"ThumbTest",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new ThumbTest()); } },
//    {"Tasks", img_mainmenu_lastapps_bits, img_mainmenu_lastapps_height, img_mainmenu_lastapps_width, [](IGNORE_PARAM) { LaunchApplication(new TaskSwitcher()); } },
//    {"Log", img_mainmenu_provisioning_bits, img_mainmenu_provisioning_height, img_mainmenu_provisioning_width, [](IGNORE_PARAM) { LaunchApplication(new LogViewApplication()); } },
//    {"Prov", img_mainmenu_provisioning_bits, img_mainmenu_provisioning_height, img_mainmenu_provisioning_width, [](IGNORE_PARAM) { LaunchApplication(new Provisioning2Application()); } },
//    {"Update",img_mainmenu_update_bits, img_mainmenu_update_height, img_mainmenu_update_width, [](IGNORE_PARAM) { LaunchApplication(new OTAUpdateApplication()); } },
#ifdef LILYGO_WATCH_2020_V3
//    {"Mic test",img_mainmenu_mic_bits, img_mainmenu_mic_height, img_mainmenu_mic_width, [](IGNORE_PARAM) { LaunchApplication(new MicApplication()); } },
#endif
    {"Bright",img_mainmenu_bright_bits, img_mainmenu_bright_height, img_mainmenu_bright_width, [](IGNORE_PARAM) { LaunchApplication(new BrightnessApplication()); } },
    {"Notify", img_mainmenu_notifications_bits, img_mainmenu_notifications_height, img_mainmenu_notifications_width, [](IGNORE_PARAM) { LaunchApplication(new NotificacionsApplication()); } },
    {"Steps",img_mainmenu_steps_bits, img_mainmenu_steps_height, img_mainmenu_steps_width, [](IGNORE_PARAM) { LaunchApplication(new StepsApplication()); } },
    {"Battery",img_mainmenu_battery_bits, img_mainmenu_battery_height, img_mainmenu_battery_width,  [](IGNORE_PARAM) { LaunchApplication(new BatteryApplication()); } },
    {"Stopwatch",img_mainmenu_stopwatch_bits, img_mainmenu_stopwatch_height, img_mainmenu_stopwatch_width, [](IGNORE_PARAM) { LaunchApplication(new StopwatchApplication()); } },
    {"Settings",img_mainmenu_options_bits, img_mainmenu_options_height, img_mainmenu_options_width, [](IGNORE_PARAM) { LaunchApplication(new SettingsMenuApplication()); } },
    {"Calendar",img_mainmenu_calendar_bits, img_mainmenu_calendar_height, img_mainmenu_calendar_width, [](IGNORE_PARAM) { LaunchApplication(new CalendarApplication()); } },
    {"Calculator",img_mainmenu_calculator_bits, img_mainmenu_calculator_height, img_mainmenu_calculator_width, [](IGNORE_PARAM) { LaunchApplication(new CalculatorApplication()); } },
//    {"Lamp",img_mainmenu_lamp_bits, img_mainmenu_lamp_height, img_mainmenu_lamp_width, [](IGNORE_PARAM) { LaunchApplication(new LampApplication()); } },
//    {"Player",img_mainmenu_player_bits, img_mainmenu_player_height, img_mainmenu_player_width, [](IGNORE_PARAM) { LaunchApplication(new BLEPlayerApplication()); } },
//    {"Peers",img_mainmenu_chat_bits, img_mainmenu_chat_height, img_mainmenu_chat_width, [](IGNORE_PARAM) { LaunchApplication(new PeerApplication()); } },
    {"About",img_mainmenu_about_bits, img_mainmenu_about_height, img_mainmenu_about_width, [](IGNORE_PARAM) { LaunchApplication(new AboutApplication()); } },
#ifdef LUNOKIOT_DEBUG_UI_DEMO
    {"GyroMonitor",img_mainmenu_activity_bits, img_mainmenu_activity_height, img_mainmenu_activity_width, [](IGNORE_PARAM) { LaunchApplication(new ActivitiesApplication()); } },
    {"Playground12",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication12()); } },
    {"Playground11",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication11()); } },
    {"Playground10",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication10()); } },
    {"Playground9",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication9()); } },
    {"Playground8",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication8()); } },
    {"Playground7",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication7()); } },
    {"Playground6",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication6()); } },
    {"Playground5",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication5()); } },
    {"Playground4",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication4()); } },
    {"Playground3",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication3()); } },
    {"Playground2",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication2()); } },
    {"Playground1",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication1()); } },
    {"Playground0",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication0()); } },
    {"Playground",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](IGNORE_PARAM) { LaunchApplication(new PlaygroundApplication()); } },
#endif
};
int MaxAppOffset = sizeof(AllApps) / sizeof(AllApps[0])-1;

MainMenuApplication::MainMenuApplication() {
    canvas->setTextSize(1);
    canvas->setTextDatum(BC_DATUM);
    canvas->setFreeFont(&FreeMonoBold18pt7b);
    canvas->setTextColor(ThCol(text));
    Tick(); // force first redraw as splash :)
}
bool MainMenuApplication::Tick() {
    if (( touched ) && (false == lastTouch)) { // thumb detected
        displaced = false;
        lastTouchX = touchX;
    } else if (( false == touched ) && (lastTouch)) { // thumb out
        displacement+=newDisplacement;
        newDisplacement = 0;
        if ( false == displaced ) { AllApps[currentAppOffset].callback(this); }
    } else if ( touched ) {
        newDisplacement = lastTouchX-touchX;
        if ( 0 != newDisplacement) { displaced=true; }
    }

    currentAppOffset = displacement / MenuItemSize;
    if ( currentAppOffset < 0 ) {
        currentAppOffset = 0;
        displacement = 0;
    } else if ( currentAppOffset > MaxAppOffset ) {
        currentAppOffset = MaxAppOffset;
        displacement = MaxAppOffset*MenuItemSize;
    }
    int32_t currentDisplacement = displacement+newDisplacement;
    lastTouch = touched;
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(dark));

        int32_t lastCurrentAppOffset = currentAppOffset-1;
        if ( lastCurrentAppOffset > -1 ) {
            canvas->drawXBitmap((TFT_WIDTH/2)-(AllApps[lastCurrentAppOffset].width/2)-newDisplacement-(MenuItemSize+(MenuItemSize/2)),
                                (TFT_HEIGHT/2)-(AllApps[lastCurrentAppOffset].height/2),
                                AllApps[lastCurrentAppOffset].imagebits,
                                AllApps[lastCurrentAppOffset].width,
                                AllApps[lastCurrentAppOffset].height, ThCol(light));            
        }
        canvas->drawXBitmap((TFT_WIDTH/2)-(AllApps[currentAppOffset].width/2)-newDisplacement,
                            (TFT_HEIGHT/2)-(AllApps[currentAppOffset].height/2),
                            AllApps[currentAppOffset].imagebits,
                            AllApps[currentAppOffset].width,
                            AllApps[currentAppOffset].height, ThCol(light));
        int32_t nextCurrentAppOffset = currentAppOffset+1;
        if ( nextCurrentAppOffset < MaxAppOffset+1 ) { 
            canvas->drawXBitmap(((TFT_WIDTH/2)-(AllApps[nextCurrentAppOffset].width/2))-newDisplacement+(MenuItemSize+(MenuItemSize/2)),
                                (TFT_HEIGHT/2)-(AllApps[nextCurrentAppOffset].height/2),
                                AllApps[nextCurrentAppOffset].imagebits,
                                AllApps[nextCurrentAppOffset].width,
                                AllApps[nextCurrentAppOffset].height, ThCol(light));
        }
        if ( 0 == newDisplacement ) { // only when the fingers are quiet
            //Show element count
            //currentAppOffset > MaxAppOffset
            const uint8_t dotRad = 5;
            const uint8_t dotSpacing = 12;
            int32_t x=(TFT_WIDTH-(MaxAppOffset*dotSpacing))/2;
            int32_t y=15;
            for(int32_t c=0;c<=MaxAppOffset;c++) {
                if ( c == currentAppOffset ) {
                    canvas->fillCircle(x+(dotSpacing*c),y,dotRad,ThCol(light));
                } else if ( c < currentAppOffset ) {
                    canvas->drawCircle(x+(dotSpacing*c),y,dotRad-1,ThCol(darken));
                } else {
                    canvas->fillCircle(x+(dotSpacing*c),y,dotRad-1,ThCol(middle));
                }
            }
            //Show the app name
            canvas->drawString(AllApps[currentAppOffset].name, appNamePosX, appNamePosY);
        }
        nextRedraw = millis()+(1000/16);
        return true;
    }
    return false;
}
