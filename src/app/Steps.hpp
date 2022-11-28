#ifndef __LUNOKIOT__STEPS_APP__
#define __LUNOKIOT__STEPS_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"

void InstallStepManager();

class StepsApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
    public:
        void CreateStats();
        ButtonImageXBMWidget * btnSetup = nullptr;
        ButtonImageXBMWidget * btnBack = nullptr;
        GraphWidget * weekGraph = nullptr;
        StepsApplication();
        ~StepsApplication();
        bool Tick();
};

#endif
