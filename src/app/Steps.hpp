#ifndef __LUNOKIOT__STEPS_APP__
#define __LUNOKIOT__STEPS_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"

void InstallStepManager();

class StepsApplication: public TemplateApplication {
    private:
        unsigned long nextRedraw=0;
    public:
        const char *AppName() override { return "Step counter"; };
        void CreateStats();
        ButtonImageXBMWidget * btnSetup = nullptr;
        //ButtonImageXBMWidget * btnBack = nullptr;
        GraphWidget * weekGraph = nullptr;
        GraphWidget * activityGraph = nullptr;
        StepsApplication();
        ~StepsApplication();
        bool Tick();
};

#endif
