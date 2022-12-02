#ifndef __LUNOKIOT__APPLICATION__MIC__
#define __LUNOKIOT__APPLICATION__MIC__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/GraphWidget.hpp"
#include <driver/i2s.h>

class MicApplication : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
    public:
        i2s_config_t i2s_config;
        i2s_pin_config_t i2s_cfg;
        GraphWidget * audioWaveGraph;
        MicApplication();
        ~MicApplication();
        bool Tick();
};

#endif
