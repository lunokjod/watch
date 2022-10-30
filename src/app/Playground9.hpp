#ifndef __LUNOKIOT__PLAYGROUND9_APP__
#define __LUNOKIOT__PLAYGROUND9_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"

class PlaygroundApplication9: public LunokIoTApplication {
    private:
        uint16_t colorMap[16] = { 0 };
        size_t colorOffset=0;
        unsigned long nextRedraw=0;
    public:
        PlaygroundApplication9();
        ~PlaygroundApplication9();
        bool Tick();
};

#endif
