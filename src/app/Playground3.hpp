#ifndef __LUNOKIOT__PLAYGROUND3_APP__
#define __LUNOKIOT__PLAYGROUND3_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../UI/widgets/GaugeWidget.hpp"

class PlaygroundApplication3: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        unsigned long timeTick=0;
        unsigned long fixTimeTimeout=-1;
        const unsigned long SetTimeTimeout=5000; 
        bool showTime = true;
        bool timeIsSet = false;
        int hour=0;
        int minute=0;
    public:
        const char *AppName() override { return "Playground3 test"; };

        GaugeWidget * hourGauge = nullptr;
        GaugeWidget * minuteGauge = nullptr;

        PlaygroundApplication3();
        ~PlaygroundApplication3();
        bool Tick();
};

#endif
