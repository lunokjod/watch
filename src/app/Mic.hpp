#ifndef __LUNOKIOT__APPLICATION__MIC__
#define __LUNOKIOT__APPLICATION__MIC__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/GraphWidget.hpp"
#include <driver/i2s.h>

class MicApplication : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
        unsigned long graphRedraw=0;
    public:
        const char *AppName() override { return "Mic recorder"; };
        bool recThread = true;
        bool recThreadDead = false;
        GraphWidget * audioWaveGraph;
        MicApplication();
        ~MicApplication();
        bool Tick();
        static void _RecordThread(void *args);
};

#endif
