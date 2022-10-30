#ifndef __LUNOKIOT__PlaygroundApplication6_APP__
#define __LUNOKIOT__PlaygroundApplication6_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"

class PlaygroundApplication7: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
    public:
        PlaygroundApplication7();
        ~PlaygroundApplication7();
        bool Tick();
};

#endif
