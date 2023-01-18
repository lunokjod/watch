#ifndef __LUNOKIOT__ADVANCED_SETTINGS_APP__
#define __LUNOKIOT__ADVANCED_SETTINGS_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class AdvancedSettingsApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
    public:
        const char *AppName() override { return "Advanced settings"; };
        ButtonImageXBMWidget * btnBack = nullptr;
        ButtonImageXBMWidget * btnErase = nullptr;
        ButtonImageXBMWidget * btnLog = nullptr;
        AdvancedSettingsApplication();
        ~AdvancedSettingsApplication();
        bool Tick();
};

#endif
