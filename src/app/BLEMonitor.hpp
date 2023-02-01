#ifndef __LUNOKIOT__BLEMONITOR_APP__
#define __LUNOKIOT__BLEMONITOR_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"
//#include "../system/Network.hpp"
//#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class BLEMonitorApplication: public TemplateApplication {
    private:
        unsigned long nextRedraw=0;
        int rotateVal = 0;
    public:
        const char *AppName() override { return "BLE Monitor"; };
        BLEMonitorApplication();
        ~BLEMonitorApplication();
        bool Tick();
};

#endif
