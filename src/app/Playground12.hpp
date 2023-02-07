#ifndef __LUNOKIOT__PLAYGROUND12_APP__
#define __LUNOKIOT__PLAYGROUND12_APP__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/GaugeWidget.hpp"

class PlaygroundApplication12: public TemplateApplication {
        unsigned long nextRedraw=0;
        GaugeWidget * wristGauge = nullptr;
        int16_t lastAngle=0;
        int16_t levelCorrectAngle=0;
        int16_t levelDriftAngle=20;
        int16_t currentLevel=0;
        bool playerWin=false;
        const int16_t aMIN=90;
        const int16_t aMAX=250;
        uint8_t nearby=0;
        unsigned long nextHapticBeat=0;
    public:
        const char *AppName() override { return "Playground12 test"; };
        PlaygroundApplication12();
        ~PlaygroundApplication12();
        bool Tick();
};

#endif
