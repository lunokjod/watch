#ifndef __LUNOKIOT__PLAYGROUND1_APP__
#define __LUNOKIOT__PLAYGROUND1_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/NotificationWidget.hpp"
#include "../UI/widgets/CanvasWidget.hpp"

class PlaygroundApplication1: public LunokIoTApplication {
    public:
        CanvasWidget *background_image=nullptr;
        NotificationWidget *testNotification=nullptr;
        NotificationWidget *testNotification1=nullptr;
        unsigned long nextRedraw=0;
        PlaygroundApplication1();
        ~PlaygroundApplication1();
        bool Tick();
};

#endif
