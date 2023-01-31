#ifndef __LUNOKIOT__STEPWATCH_APP__
#define __LUNOKIOT__STEPWATCH_APP__
/*
 * This app explains the KVO funcionality (callback when bus event received)
 */

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../system/Application.hpp"

class StopwatchApplication: public TemplateApplication {
    private:
        unsigned long nextRedraw=0;
        ButtonImageXBMWidget * resetBtn = nullptr;
        ButtonImageXBMWidget * pauseBtn = nullptr;
        ButtonImageXBMWidget * startBtn = nullptr;
        unsigned long nextBlink=0;
        bool blink=false;
    public:
        const char *AppName() override { return "Stopwatch"; };
        static unsigned long pauseTime;
        static unsigned long starTime;
        StopwatchApplication();
        ~StopwatchApplication();
        bool Tick();
};

#endif
