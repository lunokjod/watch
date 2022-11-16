#ifndef __LUNOKIOT__CHANGEME_APP__
#define __LUNOKIOT__CHANGEME_APP__

#include <Arduino.h>
#include "../system/Application.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class TemplateApplication: public LunokIoTApplication {
    protected:
        unsigned long nextRefresh=0;
        ButtonImageXBMWidget * btnBack = nullptr;
    public:
        TemplateApplication();
        virtual ~TemplateApplication();
        bool Tick();
};

#endif
