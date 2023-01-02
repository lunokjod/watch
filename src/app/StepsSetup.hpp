#ifndef __LUNOKIOT__STEPSSETUP_APP__
#define __LUNOKIOT__STEPS_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"
#include "../UI/widgets/ValueSelector.hpp"
#include "../UI/widgets/SwitchWidget.hpp"

class StepsSetupApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        unsigned long nextSpinStep=0;
    public:
        const char *AppName() override { return "Step settings"; };
        ButtonImageXBMWidget * btnBack = nullptr;
        SwitchWidget * genderSelect = nullptr;
        ValueSelector * tallValue = nullptr;
        StepsSetupApplication();
        ~StepsSetupApplication();
        bool Tick();
};

#endif
