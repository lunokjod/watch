#ifndef __LUNOKIOT__PLAYGROUND0_APP__
#define __LUNOKIOT__PLAYGROUND0_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"

class PlaygroundApplication0: public LunokIoTApplication {
    public:
        const char *AppName() override { return "Playground0 test"; };
        unsigned long nextRedraw=0;
        PlaygroundApplication0();
        ~PlaygroundApplication0();
        bool Tick();
};

#endif
