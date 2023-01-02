#ifndef __LUNOKIOT__SETTINGSMENU_APP__
#define __LUNOKIOT__SETTINGSMENU_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"

class SettingsMenuApplication: public LunokIoTApplication {
    private:
        int16_t lastTouchX=0;
        bool lastTouch = false;
        bool displaced = false;
        unsigned long nextRedraw = 0;
        int32_t displacement = 0;
        int16_t newDisplacement=0;
        int32_t currentAppOffset=0;
        const static int MenuItemSize = 130;
        int32_t appNamePosX = (TFT_WIDTH/2);
        int32_t appNamePosY = TFT_HEIGHT-20;
    public:
        const char *AppName() override { return "Settings menu"; };
        SettingsMenuApplication();
        ~SettingsMenuApplication();
        bool Tick();
};

#endif
