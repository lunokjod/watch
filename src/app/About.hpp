#ifndef __LUNOKIOT__ABOUT_APP__
#define __LUNOKIOT__ABOUT_APP__
#include <Arduino.h>
//#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../UI/widgets/CanvasWidget.hpp"

class AboutApplication: public TemplateApplication {
    private:
        unsigned long drawOneSecond=0;
        unsigned long nextRedraw=0;
        unsigned long nextScrollRedraw=0;
        uint8_t counterLines=0;
    public:
        const char *AppName() override { return "About"; };
        TFT_eSprite * textBuffer = nullptr;
        CanvasWidget * colorBuffer = nullptr;
        CanvasWidget * perspectiveTextBuffer = nullptr;
        AboutApplication();
        ~AboutApplication();
        bool Tick();
};

#endif
