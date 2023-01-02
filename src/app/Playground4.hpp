#ifndef __LUNOKIOT__PLAYGROUND4_APP__
#define __LUNOKIOT__PLAYGROUND4_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/ScrollViewWidget.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include <list>

class PlaygroundApplication4: public LunokIoTApplication {
    public:
        std::list<TFT_eSprite *>Thumbnails;
        const char *AppName() override { return "Playground4 test"; };

        CanvasWidget * testBackground = nullptr;
        ScrollViewWidget * test=nullptr;
        unsigned long nextRedraw=0;
        PlaygroundApplication4();
        ~PlaygroundApplication4();
        bool Tick();
};

#endif
