#ifndef __LUNOKIOT__PLAYGROUND11_APP__
#define __LUNOKIOT__PLAYGROUND11_APP__

#include "../UI/AppTemplate.hpp"
#include "../system/Datasources/perceptron.hpp"

class PlaygroundApplication11: public TemplateApplication {
        unsigned long nextRedraw=0;
    public:
        const char *AppName() override { return "Playground11 test"; };
        PlaygroundApplication11();
        ~PlaygroundApplication11();
        bool Tick();
        const size_t PPoolSize=16;
        Perceptron *ppool=nullptr;
};

#endif
