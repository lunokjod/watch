#ifndef __LUNOKIOT__BRIGHTNESS_APP__
#define __LUNOKIOT__BRIGHTNESS_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "Watchface.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GaugeWidget.hpp"

class BrightnessApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
    public:
        ButtonImageXBMWidget * btnBack = nullptr;
        GaugeWidget * brightGauge = nullptr;
        BrightnessApplication();
        ~BrightnessApplication();
        bool Tick();
};

#endif
