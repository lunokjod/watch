#include <Arduino.h>
#include <LilyGoWatch.h>
#include "MainMenu.hpp"
#include "../lunokiot_config.hpp"
#include "../static/img_mainmenu_back.xbm"
#include "Watchface.hpp"
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
#endif


typedef struct {
    const char *name;
    unsigned char * imagebits;
    int16_t height;
    int16_t width;
    std::function<void ()> callback;

} MainMenuApplicationEntry;

int32_t currentAppOffset = 0;
MainMenuApplicationEntry AllApps[] = {
    {"Back", img_mainmenu_back_bits, img_mainmenu_back_height, img_mainmenu_back_width, []() { LaunchApplication(new WatchfaceApplication()); } },
    {"Activity",img_mainmenu_activity_bits, img_mainmenu_activity_height, img_mainmenu_activity_width, []() { LaunchApplication(new ActivitiesApplication()); } },
    {"Bright",img_mainmenu_bright_bits, img_mainmenu_bright_height, img_mainmenu_bright_width, []() { LaunchApplication(new BrightnessApplication()); } },
    {"Steps",img_mainmenu_steps_bits, img_mainmenu_steps_height, img_mainmenu_steps_width, []() { LaunchApplication(new StepsApplication()); } },
    {"Battery",img_mainmenu_battery_bits, img_mainmenu_battery_height, img_mainmenu_battery_width,  []() { LaunchApplication(new BatteryApplication()); } },
    {"Options",img_mainmenu_options_bits, img_mainmenu_options_height, img_mainmenu_options_width, []() { LaunchApplication(new SettingsApplication()); } },
    {"Advanced",img_mainmenu_cpu_bits, img_mainmenu_cpu_height, img_mainmenu_cpu_width, []() { LaunchApplication(new AdvancedSettingsApplication()); } },
    {"Set time", img_mainmenu_settime_bits, img_mainmenu_settime_height, img_mainmenu_settime_width, []() { LaunchApplication(new SetTimeApplication()); } },
    {"Timezone",img_timezone_120_bits, img_timezone_120_height, img_timezone_120_width, []() { LaunchApplication(new SetTimeZoneApplication()); } },
    {"Prov", img_mainmenu_provisioning_bits, img_mainmenu_provisioning_height, img_mainmenu_provisioning_width, []() { LaunchApplication(new Provisioning2Application()); } },
    {"Rotation", img_rotate_120_bits, img_rotate_120_height, img_rotate_120_width, []() { LaunchApplication(new RotationApplication()); } },
    {"Notify", img_mainmenu_notifications_bits, img_mainmenu_notifications_height, img_mainmenu_notifications_width, []() { LaunchApplication(new NotificacionsApplication()); } },
    {"About",img_mainmenu_about_bits, img_mainmenu_about_height, img_mainmenu_about_width, []() { LaunchApplication(new AboutApplication()); } },
/*
#ifdef LUNOKIOT_DEBUG_UI
    {"Playground9",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new PlaygroundApplication9()); } },
    {"Playground8",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new PlaygroundApplication8()); } },
    {"Playground7",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new PlaygroundApplication7()); } },
    {"Playground6",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new PlaygroundApplication6()); } },
    {"Playground5",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new PlaygroundApplication5()); } },
    {"Playground4",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new PlaygroundApplication4()); } },
    {"Playground3",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new PlaygroundApplication3()); } },
    {"Playground2",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new PlaygroundApplication2()); } },
    {"Playground1",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new PlaygroundApplication1()); } },
    {"Playground0",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new PlaygroundApplication0()); } },
    {"Playground",img_mainmenu_debug_bits, img_mainmenu_debug_height, img_mainmenu_debug_width, []() { LaunchApplication(new PlaygroundApplication()); } },
#endif
*/
};
int MaxAppOffset = sizeof(AllApps) / sizeof(AllApps[0])-1;

MainMenuApplication::~MainMenuApplication() {

}
MainMenuApplication::MainMenuApplication() {
    canvas->setTextSize(1);
    canvas->setTextDatum(BC_DATUM);
    canvas->setFreeFont(&FreeMonoBold18pt7b);
    canvas->setTextColor(TFT_WHITE);
}
bool MainMenuApplication::Tick() {
    if (( touched ) && (false == lastTouch)) {
        displaced = false;
        lastTouchX = touchX;
        //Serial.println("THUMB IN");
    } else if (( false == touched ) && (lastTouch)) {
        //Serial.println("THUMB OUT");
        displacement+=newDisplacement;
        newDisplacement = 0;
        if ( false == displaced ) { AllApps[currentAppOffset].callback(); }
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
        canvas->fillSprite(TFT_BLACK);
        int32_t lastCurrentAppOffset = currentAppOffset-1;
        if ( lastCurrentAppOffset > -1 ) {
            canvas->drawXBitmap((TFT_WIDTH/2)-(AllApps[lastCurrentAppOffset].width/2)-newDisplacement-(MenuItemSize+(MenuItemSize/2)),
                                (TFT_HEIGHT/2)-(AllApps[lastCurrentAppOffset].height/2),
                                AllApps[lastCurrentAppOffset].imagebits,
                                AllApps[lastCurrentAppOffset].width,
                                AllApps[lastCurrentAppOffset].height, TFT_WHITE);            
        }
        canvas->drawXBitmap((TFT_WIDTH/2)-(AllApps[currentAppOffset].width/2)-newDisplacement,
                            (TFT_HEIGHT/2)-(AllApps[currentAppOffset].height/2),
                            AllApps[currentAppOffset].imagebits,
                            AllApps[currentAppOffset].width,
                            AllApps[currentAppOffset].height, TFT_WHITE);
        int32_t nextCurrentAppOffset = currentAppOffset+1;
        if ( nextCurrentAppOffset < MaxAppOffset+1 ) { 
            canvas->drawXBitmap(((TFT_WIDTH/2)-(AllApps[nextCurrentAppOffset].width/2))-newDisplacement+(MenuItemSize+(MenuItemSize/2)),
                                (TFT_HEIGHT/2)-(AllApps[nextCurrentAppOffset].height/2),
                                AllApps[nextCurrentAppOffset].imagebits,
                                AllApps[nextCurrentAppOffset].width,
                                AllApps[nextCurrentAppOffset].height, TFT_WHITE);            
        }
        if ( 0 == newDisplacement ) {
            //Show element count
            //currentAppOffset > MaxAppOffset
            const uint8_t dotRad = 5;
            const uint8_t dotSpacing = 12;
            int32_t x=(TFT_WIDTH-(MaxAppOffset*dotSpacing))/2;
            int32_t y=15;
            for(int32_t c=0;c<=MaxAppOffset;c++) {
                if ( c == currentAppOffset ) {
                    canvas->fillCircle(x+(dotSpacing*c),y,dotRad,TFT_WHITE);
                } else if ( c < currentAppOffset ) {
                    canvas->drawCircle(x+(dotSpacing*c),y,dotRad-1,canvas->color24to16(0x555f68));
                } else {
                    canvas->fillCircle(x+(dotSpacing*c),y,dotRad-1,canvas->color24to16(0x555f68));
                }
            }
            //Show the app name
            canvas->drawString(AllApps[currentAppOffset].name, appNamePosX, appNamePosY);
        }

        nextRedraw = millis()+(1000/12);
        return true;
    }
    return false;
}
