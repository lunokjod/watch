#ifndef __LUNOKIOT__PLAYGROUND2_APP__
#define __LUNOKIOT__PLAYGROUND2_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"

class PlaygroundApplication2: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        unsigned long gameTick=0;
        unsigned long shootTime=0;
        unsigned long nextShootTime=0;
    public:
        const char *AppName() override { return "Playground2 test"; };

        CanvasWidget *bullets=nullptr;
        CanvasWidget *spaceShip=nullptr;
        CanvasWidget *buffer=nullptr;
        PlaygroundApplication2();
        ~PlaygroundApplication2();
        bool Tick();
};

#endif
