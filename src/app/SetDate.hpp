#ifndef __LUNOKIOT__SET_DATE_APP__
#define __LUNOKIOT__SET_DATE_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/ValueSelector.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class SetDateApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        //unsigned long nextSelectorTick=0;
        ValueSelector * day = nullptr;
        ValueSelector * month = nullptr;
        ButtonImageXBMWidget * showTimeButton = nullptr;
        ButtonImageXBMWidget * setDateButton = nullptr;
        ButtonImageXBMWidget * backButton = nullptr;
    public:
        const char *AppName() override { return "Set date"; };
        SetDateApplication();
        ~SetDateApplication();
        bool Tick();
};

#endif
