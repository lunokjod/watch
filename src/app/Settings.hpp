#ifndef __LUNOKIOT__SETTINGS_APP__
#define __LUNOKIOT__SETTINGS_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "Watchface.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"

class SettingsApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
    public:
        ButtonImageXBMWidget * btnBack = nullptr;
        SwitchWidget * ntpCheck = nullptr;
        SwitchWidget * openweatherCheck = nullptr;
        SwitchWidget * wifiCheck = nullptr;
        SwitchWidget * bleCheck = nullptr;
        SettingsApplication();
        ~SettingsApplication();
        bool Tick();
};

#endif
