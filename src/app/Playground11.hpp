#ifndef __LUNOKIOT__PLAYGROUND11_APP__
#define __LUNOKIOT__PLAYGROUND11_APP__

#include "../UI/AppTemplate.hpp"
#include "../UI/representation/Point2D.hpp"
/*
 * Pretends demonstrate the use of anchors and animators
 */
class PlaygroundApplication11: public TemplateApplication {
        unsigned long nextRedraw=0;
    public:
        const char *AppName() override { return "Playground11 test"; };
        PlaygroundApplication11();
        ~PlaygroundApplication11();
        bool Tick();
        Point2D *pointOne=nullptr;
        Point2D *pointTwo=nullptr;
};

#endif
