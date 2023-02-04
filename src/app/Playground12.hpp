#ifndef __LUNOKIOT__PLAYGROUND12_APP__
#define __LUNOKIOT__PLAYGROUND12_APP__

#include "../UI/AppTemplate.hpp"

class PlaygroundApplication12: public TemplateApplication {
        unsigned long nextRedraw=0;
    public:
        const char *AppName() override { return "Playground12 test"; };
        PlaygroundApplication12();
        ~PlaygroundApplication12();
        bool Tick();
};

#endif
