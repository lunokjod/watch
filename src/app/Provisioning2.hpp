#ifndef __LUNOKIOT__PROVISIONING2_APP__
#define __LUNOKIOT__PROVISIONING2_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class Provisioning2Application: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
    public:
        ButtonImageXBMWidget * clearProvBtn=nullptr;
        ButtonImageXBMWidget * backBtn=nullptr;
        ButtonImageXBMWidget * startProvBtn=nullptr;
        Provisioning2Application();
        ~Provisioning2Application();
        bool Tick();
};

#endif
