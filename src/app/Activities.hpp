#ifndef __LUNOKIOT__ACTIVITIES_APP__
#define __LUNOKIOT__ACTIVITIES_APP__

#include "../system/Application.hpp"

class ActivitiesApplication: public LunokIoTApplication {
    private:
        size_t counter;
        TFT_eSprite *giroLog;
        unsigned long nextSampleGiro; 
        unsigned long nextRedraw;
        int16_t angle;
    public:
        ActivitiesApplication();
        bool Tick();
};

#endif
