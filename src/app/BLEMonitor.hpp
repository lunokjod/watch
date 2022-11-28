#ifndef __LUNOKIOT__BLEMONITOR_APP__
#define __LUNOKIOT__BLEMONITOR_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../system/Network.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class BLEMonitorApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        int rotateVal = 0;
    public:
        ButtonImageXBMWidget * btnBack = nullptr;
        BLEMonitorApplication();
        ~BLEMonitorApplication();
        bool Tick();
};

#endif
