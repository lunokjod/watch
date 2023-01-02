#ifndef __LUNOKIOT__BRIGHTNESS_APP__
#define __LUNOKIOT__BRIGHTNESS_APP__
//#include <Arduino.h>
#include "../UI/AppTemplate.hpp"

#include "../UI/widgets/GaugeWidget.hpp"

class BrightnessApplication: public TemplateApplication {
    private:
        GaugeWidget * brightGauge = nullptr;
    public:
        const char *AppName() override { return "Screen brightness"; };
        BrightnessApplication();
        ~BrightnessApplication();
        bool Tick();
};

#endif
