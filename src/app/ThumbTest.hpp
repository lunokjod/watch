#ifndef __LUNOKIOT__APPLICATION__THUMB__
#define __LUNOKIOT__APPLICATION__THUMB__

#include "../UI/AppTemplate.hpp"
#include "../system/Datasources/perceptron.hpp"

class ThumbTest : public LunokIoTApplication {
    private:
        unsigned long nextRefresh=0;
        unsigned long oneSecond=0; // trigger clear pad
        void RedrawMe();
        int16_t boxX=TFT_WIDTH; // get the box of draw
        int16_t boxY=TFT_HEIGHT;
        int16_t boxH=0;
        int16_t boxW=0;
        bool triggered=false; // to launch the recognizer
        Perceptron *learnerP0=nullptr;
        int learn=-1; // begin with learn 'yes'
    public:
        const char *AppName() override { return "Thumb test"; };
        ThumbTest();
        ~ThumbTest();
        bool Tick();
};

#endif
