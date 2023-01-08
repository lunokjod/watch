#ifndef __LUNOKIOT__PLAYGROUND10_APP__
#define __LUNOKIOT__PLAYGROUND10_APP__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/EntryTextWidget.hpp"

class PlaygroundApplication10: public TemplateApplication {
        unsigned long nextRedraw=0;
        EntryTextWidget * testEntryText=nullptr;
    public:
        const char *AppName() override { return "Playground10 test"; };
        PlaygroundApplication10();
        ~PlaygroundApplication10();
        bool Tick();
};

#endif
