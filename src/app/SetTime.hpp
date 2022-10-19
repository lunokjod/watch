#ifndef __LUNOKIOT__SET_TIME_APP__
#define __LUNOKIOT__SET_TIME_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/ValueSelector.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class SetTimeApplication: public LunokIoTApplication {
    private:
        bool blinkTime = true;
        unsigned long nextBlinkTime=0;
        unsigned long nextRedraw=0;
        ValueSelector * hour = nullptr;
        ValueSelector * minute = nullptr;
        ButtonImageXBMWidget * setTimeButton = nullptr;
        ButtonImageXBMWidget * backButton = nullptr;
    public:
        SetTimeApplication();
        ~SetTimeApplication();
        bool Tick();
};

#endif
