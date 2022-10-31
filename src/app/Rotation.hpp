#ifndef __LUNOKIOT__ROTATION_APP__
#define __LUNOKIOT__ROTATION_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/ButtonWidget.hpp"
class RotationApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        CanvasWidget *buffer=nullptr;
        ButtonWidget * up=nullptr;
        ButtonWidget * right=nullptr;
        ButtonWidget * down=nullptr;
        ButtonWidget * left=nullptr;
    public:
        RotationApplication();
        ~RotationApplication();
        bool Tick();
};

#endif
