#ifndef __LUNOKIOT__PLAYGROUND6_APP__
#define __LUNOKIOT__PLAYGROUND6_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include <list>
#include "../UI/widgets/CanvasWidget.hpp"

class PlaygroundApplication6: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        size_t offset=0;
        std::list<TFT_eSprite *>Thumbnails;
        const float thumnailScale=0.333;
        CanvasWidget * buffer=nullptr;
    public:
        PlaygroundApplication6();
        ~PlaygroundApplication6();
        bool Tick();
};

#endif
