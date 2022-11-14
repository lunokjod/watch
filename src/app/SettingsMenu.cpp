#include <Arduino.h>
#include <LilyGoWatch.h>
#include "SettingsMenu.hpp"
#include "../lunokiot_config.hpp"

#include "../static/img_mainmenu_back.xbm"
#include "Watchface.hpp"
#include "MainMenu.hpp"
#include "../static/img_mainmenu_options.xbm"
#include "Settings.hpp"
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
#include "Brightness.hpp"
#include "../static/img_mainmenu_bright.xbm"
#include "Advanced.hpp"
#include "../static/img_mainmenu_cpu.xbm"
#include "Bluetooth.hpp"
#include "../static/img_mainmenu_bluetooth.xbm"
#include "BLEMonitor.hpp"
#include "Theme.hpp"
#include "../static/img_mainmenu_themes.xbm"

typedef struct {
    const char *name;
    unsigned char * imagebits;
    int16_t height;
    int16_t width;
    std::function<void ()> callback;

} MainMenuApplicationEntry;

MainMenuApplicationEntry SettingsApps[] = {
    {"Back", img_mainmenu_back_bits, img_mainmenu_back_height, img_mainmenu_back_width, []() { LaunchApplication(new WatchfaceApplication()); } },
    {"Options",img_mainmenu_options_bits, img_mainmenu_options_height, img_mainmenu_options_width, []() { LaunchApplication(new SettingsApplication()); } },
    {"Themes",img_mainmenu_themes_bits, img_mainmenu_themes_height, img_mainmenu_themes_width, []() { LaunchApplication(new ThemeApplication()); } },
    {"Pair",img_mainmenu_bluetooth_bits, img_mainmenu_bluetooth_height, img_mainmenu_bluetooth_width, []() { LaunchApplication(new BluetoothApplication()); } },
    {"Battery",img_mainmenu_battery_bits, img_mainmenu_battery_height, img_mainmenu_battery_width,  []() { LaunchApplication(new BatteryApplication()); } },
    {"BLEMonitor",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new BLEMonitorApplication()); } },
    {"Set time", img_mainmenu_settime_bits, img_mainmenu_settime_height, img_mainmenu_settime_width, []() { LaunchApplication(new SetTimeApplication()); } },
    {"Timezone",img_timezone_120_bits, img_timezone_120_height, img_timezone_120_width, []() { LaunchApplication(new SetTimeZoneApplication()); } },
    {"Prov", img_mainmenu_provisioning_bits, img_mainmenu_provisioning_height, img_mainmenu_provisioning_width, []() { LaunchApplication(new Provisioning2Application()); } },
    {"Rotation", img_rotate_120_bits, img_rotate_120_height, img_rotate_120_width, []() { LaunchApplication(new RotationApplication()); } },
    {"Bright",img_mainmenu_bright_bits, img_mainmenu_bright_height, img_mainmenu_bright_width, []() { LaunchApplication(new BrightnessApplication()); } },
    {"Advanced",img_mainmenu_cpu_bits, img_mainmenu_cpu_height, img_mainmenu_cpu_width, []() { LaunchApplication(new AdvancedSettingsApplication()); } },
};
int MaxSettingsAppOffset = sizeof(SettingsApps) / sizeof(SettingsApps[0])-1;

SettingsMenuApplication::~SettingsMenuApplication() {

}
SettingsMenuApplication::SettingsMenuApplication() {
    Tick(); // force first redraw as splash :)
}
bool SettingsMenuApplication::Tick() {
    if (( touched ) && (false == lastTouch)) { // thumb detected
        displaced = false;
        lastTouchX = touchX;
    } else if (( false == touched ) && (lastTouch)) { // thumb out
        displacement+=newDisplacement;
        newDisplacement = 0;
        if ( false == displaced ) { SettingsApps[currentAppOffset].callback(); }
    } else if ( touched ) {
        newDisplacement = lastTouchX-touchX;
        if ( 0 != newDisplacement) { displaced=true; }
    }

    currentAppOffset = displacement / MenuItemSize;
    if ( currentAppOffset < 0 ) {
        currentAppOffset = 0;
        displacement = 0;
    } else if ( currentAppOffset > MaxSettingsAppOffset ) {
        currentAppOffset = MaxSettingsAppOffset;
        displacement = MaxSettingsAppOffset*MenuItemSize;
    }
    int32_t currentDisplacement = displacement+newDisplacement;
    lastTouch = touched;
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        canvas->setTextSize(2);
        canvas->setTextDatum(TC_DATUM);
        canvas->setFreeFont(0);
        canvas->setTextColor(ThCol(light));
        canvas->drawString("Settings", 120, 32);
        int32_t lastCurrentAppOffset = currentAppOffset-1;
        if ( lastCurrentAppOffset > -1 ) {
            canvas->drawXBitmap((TFT_WIDTH/2)-(SettingsApps[lastCurrentAppOffset].width/2)-newDisplacement-(MenuItemSize+(MenuItemSize/2)),
                                (TFT_HEIGHT/2)-(SettingsApps[lastCurrentAppOffset].height/2),
                                SettingsApps[lastCurrentAppOffset].imagebits,
                                SettingsApps[lastCurrentAppOffset].width,
                                SettingsApps[lastCurrentAppOffset].height, ThCol(light));            
        }
        canvas->drawXBitmap((TFT_WIDTH/2)-(SettingsApps[currentAppOffset].width/2)-newDisplacement,
                            (TFT_HEIGHT/2)-(SettingsApps[currentAppOffset].height/2),
                            SettingsApps[currentAppOffset].imagebits,
                            SettingsApps[currentAppOffset].width,
                            SettingsApps[currentAppOffset].height, ThCol(light));
        int32_t nextCurrentAppOffset = currentAppOffset+1;
        if ( nextCurrentAppOffset < MaxSettingsAppOffset+1 ) { 
            canvas->drawXBitmap(((TFT_WIDTH/2)-(SettingsApps[nextCurrentAppOffset].width/2))-newDisplacement+(MenuItemSize+(MenuItemSize/2)),
                                (TFT_HEIGHT/2)-(SettingsApps[nextCurrentAppOffset].height/2),
                                SettingsApps[nextCurrentAppOffset].imagebits,
                                SettingsApps[nextCurrentAppOffset].width,
                                SettingsApps[nextCurrentAppOffset].height, ThCol(light));            
        }

        if ( 0 == newDisplacement ) { // only when the fingers are quiet
            //Show element count
            const uint8_t dotRad = 5;
            const uint8_t dotSpacing = 12;
            int32_t x=(TFT_WIDTH-(MaxSettingsAppOffset*dotSpacing))/2;
            int32_t y=15;
            for(int32_t c=0;c<=MaxSettingsAppOffset;c++) {
                if ( c == currentAppOffset ) {
                    canvas->fillCircle(x+(dotSpacing*c),y,dotRad,ThCol(light));
                } else if ( c < currentAppOffset ) {
                    canvas->drawCircle(x+(dotSpacing*c),y,dotRad-1,ThCol(middle));
                } else {
                    canvas->fillCircle(x+(dotSpacing*c),y,dotRad-1,ThCol(middle));
                }
            }
            //Show the app name
            canvas->setTextSize(1);
            canvas->setTextDatum(BC_DATUM);
            canvas->setFreeFont(&FreeMonoBold18pt7b);
            canvas->setTextColor(ThCol(light));
            canvas->drawString(SettingsApps[currentAppOffset].name, appNamePosX, appNamePosY);
        }

        nextRedraw = millis()+(1000/16);
        return true;
    }
    return false;
}
