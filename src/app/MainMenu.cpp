#include <Arduino.h>
#include <LilyGoWatch.h>
#include "MainMenu.hpp"
#include "../lunokiot_config.hpp"
#include "../static/img_mainmenu_back.xbm"
#include "../static/img_mainmenu_options.xbm"
#include "SettingsMenu.hpp"

#include "../static/img_mainmenu_battery.xbm"
#include "Battery.hpp"
#include "../static/img_mainmenu_debug.xbm"

#include "Notifications.hpp"
#include "../static/img_mainmenu_notifications.xbm"

#include "Rotation.hpp"
#include "../static/img_rotate_120.xbm"

#include "Provisioning2.hpp"
#include "../static/img_mainmenu_provisioning.xbm"

#include "SetTime.hpp"
#include "../static/img_mainmenu_settime.xbm"

#include "SetTimeZone.hpp"
#include "../static/img_timezone_120.xbm"

#include "About.hpp"
#include "../static/img_mainmenu_about.xbm"

#include "Steps.hpp"
#include "../static/img_mainmenu_steps.xbm"

#include "Brightness.hpp"
#include "../static/img_mainmenu_bright.xbm"

#include "Advanced.hpp"
#include "../static/img_mainmenu_cpu.xbm"

#include "Activities.hpp"
#include "../static/img_mainmenu_activity.xbm"

#include "Bluetooth.hpp"
#include "../static/img_mainmenu_bluetooth.xbm"


#include "../static/img_mainmenu_chat.xbm"
#include "PeerApplication.hpp"

#include "../static/img_mainmenu_calculator.xbm"
#include "Calculator.hpp"

#include "../static/img_mainmenu_dungeon.xbm"
//#include "Dungeon/Dungeon.hpp"
#include "LockPicking.hpp"

#include "../static/img_mainmenu_stopwatch.xbm"
#include "Stopwatch.hpp"

#include "Settings.hpp"

#ifdef LILYGO_WATCH_2020_V3
#include "../static/img_mainmenu_mic.xbm"
#include "Mic.hpp"
#endif

#ifdef LUNOKIOT_DEBUG_UI
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

#include "../static/img_mainmenu_update.xbm"
#include "OTAUpdate.hpp"

#include "../static/img_mainmenu_lamp.xbm"
#include "Lamp.hpp"

#include "LogView.hpp"
#include "../static/img_mainmenu_lastapps.xbm"
#include "TaskSwitcher.hpp"

#include "../static/img_mainmenu_keyboard.xbm"
#include "FreehandKeyboardSetup.hpp"
#include "BLEMonitor.hpp"

#include "../static/img_mainmenu_calendar.xbm"
#include "Calendar.hpp"

#include "ThumbTest.hpp"

typedef struct {
    const char *name;
    unsigned char * imagebits;
    int16_t height;
    int16_t width;
    UICallback callback;

} MainMenuApplicationEntry;

int32_t currentAppOffset = 0;
MainMenuApplicationEntry AllApps[] = {
    {"Back", img_mainmenu_back_bits, img_mainmenu_back_height, img_mainmenu_back_width, [](void *unused) { LaunchWatchface(); } },
//    {"DEBUG",img_mainmenu_calendar_bits, img_mainmenu_calendar_height, img_mainmenu_calendar_width, [](void *unused) { LaunchApplication(new CalendarApplication()); } },
//    {"LockPick",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new LockPickingGameApplication()); } },
//    {"Playground12",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication12()); } },
//    {"Playground13",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication13()); } },
//      {"BLEMonitor",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new BLEMonitorApplication()); } },
//      {"Advanced",img_mainmenu_cpu_bits, img_mainmenu_cpu_height, img_mainmenu_cpu_width, [](void *unused) { LaunchApplication(new AdvancedSettingsApplication()); } },
//    {"Pair",img_mainmenu_bluetooth_bits, img_mainmenu_bluetooth_height, img_mainmenu_bluetooth_width, [](void *unused) { LaunchApplication(new BluetoothApplication()); } },
//    {"Playground11",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication11()); } },
//    {"Keyboard",img_mainmenu_keyboard_bits, img_mainmenu_keyboard_height, img_mainmenu_keyboard_width, [](void *unused) { LaunchApplication(new FreeHandKeyboardSetupApplication()); } },
//    {"ThumbTest",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new ThumbTest()); } },
//    {"Tasks", img_mainmenu_lastapps_bits, img_mainmenu_lastapps_height, img_mainmenu_lastapps_width, [](void *unused) { LaunchApplication(new TaskSwitcher()); } },
//    {"Log", img_mainmenu_provisioning_bits, img_mainmenu_provisioning_height, img_mainmenu_provisioning_width, [](void *unused) { LaunchApplication(new LogViewApplication()); } },
//    {"Prov", img_mainmenu_provisioning_bits, img_mainmenu_provisioning_height, img_mainmenu_provisioning_width, [](void *unused) { LaunchApplication(new Provisioning2Application()); } },
//    {"Update",img_mainmenu_update_bits, img_mainmenu_update_height, img_mainmenu_update_width, [](void *unused) { LaunchApplication(new OTAUpdateApplication()); } },
//    {"Dungeon",img_mainmenu_dungeon_bits, img_mainmenu_dungeon_height, img_mainmenu_dungeon_width, [](void *unused) { LaunchApplication(new DungeonGameApplication()); } },
#ifdef LILYGO_WATCH_2020_V3
//    {"Mic test",img_mainmenu_mic_bits, img_mainmenu_mic_height, img_mainmenu_mic_width, [](void *unused) { LaunchApplication(new MicApplication()); } },
#endif
    {"Bright",img_mainmenu_bright_bits, img_mainmenu_bright_height, img_mainmenu_bright_width, [](void *unused) { LaunchApplication(new BrightnessApplication()); } },
    {"Steps",img_mainmenu_steps_bits, img_mainmenu_steps_height, img_mainmenu_steps_width, [](void *unused) { LaunchApplication(new StepsApplication()); } },
    {"Battery",img_mainmenu_battery_bits, img_mainmenu_battery_height, img_mainmenu_battery_width,  [](void *unused) { LaunchApplication(new BatteryApplication()); } },
    {"Stopwatch",img_mainmenu_stopwatch_bits, img_mainmenu_stopwatch_height, img_mainmenu_stopwatch_width, [](void *unused) { LaunchApplication(new StopwatchApplication()); } },
    {"Settings",img_mainmenu_options_bits, img_mainmenu_options_height, img_mainmenu_options_width, [](void *unused) { LaunchApplication(new SettingsMenuApplication()); } },
    {"Calendar",img_mainmenu_calendar_bits, img_mainmenu_calendar_height, img_mainmenu_calendar_width, [](void *unused) { LaunchApplication(new CalendarApplication()); } },
    {"Calculator",img_mainmenu_calculator_bits, img_mainmenu_calculator_height, img_mainmenu_calculator_width, [](void *unused) { LaunchApplication(new CalculatorApplication()); } },
    {"Lamp",img_mainmenu_lamp_bits, img_mainmenu_lamp_height, img_mainmenu_lamp_width, [](void *unused) { LaunchApplication(new LampApplication()); } },
    {"Peers",img_mainmenu_chat_bits, img_mainmenu_chat_height, img_mainmenu_chat_width, [](void *unused) { LaunchApplication(new PeerApplication()); } },
//    {"Notify", img_mainmenu_notifications_bits, img_mainmenu_notifications_height, img_mainmenu_notifications_width, [](void *unused) { LaunchApplication(new NotificacionsApplication()); } },
    {"About",img_mainmenu_about_bits, img_mainmenu_about_height, img_mainmenu_about_width, [](void *unused) { LaunchApplication(new AboutApplication()); } },
#ifdef LUNOKIOT_DEBUG_UI_DEMO
    {"GyroMonitor",img_mainmenu_activity_bits, img_mainmenu_activity_height, img_mainmenu_activity_width, [](void *unused) { LaunchApplication(new ActivitiesApplication()); } },
    {"Playground12",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication12()); } },
    {"Playground11",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication11()); } },
    {"Playground10",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication10()); } },
    {"Playground9",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication9()); } },
    {"Playground8",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication8()); } },
    {"Playground7",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication7()); } },
    {"Playground6",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication6()); } },
    {"Playground5",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication5()); } },
    {"Playground4",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication4()); } },
    {"Playground3",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication3()); } },
    {"Playground2",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication2()); } },
    {"Playground1",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication1()); } },
    {"Playground0",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication0()); } },
    {"Playground",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, [](void *unused) { LaunchApplication(new PlaygroundApplication()); } },
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
