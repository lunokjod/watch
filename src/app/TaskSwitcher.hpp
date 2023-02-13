#ifndef __LUNOKIOT__APPLICATION__LASTAPPS__
#define __LUNOKIOT__APPLICATION__LASTAPPS__

#include "../system/Application.hpp"

class TaskSwitcher : public LunokIoTApplication {
    private:
        const float MINWAVE=0.22;
        const float MAXWAVE=0.33;
        //bool longTap=false;
        unsigned long nextRefresh=0;
        bool dirty=false;
        float currentScale=MAXWAVE-0.1;
        bool scaleDirection=true; //up or down
        int selectedOffset=-1;
        //const char  * names[LUNOKIOT_MAX_LAST_APPS] = { nullptr }; 
        //TFT_eSprite * captures[LUNOKIOT_MAX_LAST_APPS] = { nullptr };
    public:
        const char *AppName() override { return "Task switcher"; };
        TaskSwitcher();
        ~TaskSwitcher();
        const bool mustShowAsTask() override { return false; }
        bool Tick() override;
};

#endif
