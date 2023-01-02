#ifndef __LUNOKIOT__PLAYGROUND8_APP__
#define __LUNOKIOT__CHANGEME_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"

class PlaygroundApplication8: public LunokIoTApplication {
    public:
        const char *AppName() override { return "Playground8 test"; };
        const size_t TOTALPIXELS = 175*135; // video size
        uint16_t *colorBuffer = nullptr;
        uint16_t *videoPtr = nullptr;
        PlaygroundApplication8();
        ~PlaygroundApplication8();
        bool Tick();
};

#endif
