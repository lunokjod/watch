#ifndef __LUNOKIOT__PLAYGROUND5_APP__
#define __LUNOKIOT__PLAYGROUND5_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/CanvasZWidget.hpp"

class PlaygroundApplication5: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        CanvasZWidget *imageDeepTest=nullptr;
        float increment=0.05;
    public:
        PlaygroundApplication5();
        ~PlaygroundApplication5();
        bool Tick();
};

#endif
