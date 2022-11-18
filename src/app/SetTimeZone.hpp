#ifndef __LUNOKIOT__SETTIMEZONE_APP__
#define __LUNOKIOT__SETTIMEZONE_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "Watchface.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../UI/widgets/ValueSelector.hpp"

class SetTimeZoneApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        //unsigned long nextSelectorTick=0;
    public:
        ValueSelector * timezoneGMTSelector = nullptr;
        ButtonImageXBMWidget * btnBack = nullptr;
        ButtonImageXBMWidget * btnSetGMT = nullptr;
        SwitchWidget * switchDaylight = nullptr;
        SetTimeZoneApplication();
        ~SetTimeZoneApplication();
        bool Tick();
};

#endif
