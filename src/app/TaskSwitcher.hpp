#ifndef __LUNOKIOT__APPLICATION__LASTAPPS__
#define __LUNOKIOT__APPLICATION__LASTAPPS__

#include "../system/Application.hpp"

class TaskSwitcher : public LunokIoTApplication {
    private:
        //bool longTap=false;
        unsigned long nextRefresh=0;
        bool dirty=false;
        //const char  * names[LUNOKIOT_MAX_LAST_APPS] = { nullptr }; 
        TFT_eSprite * captures[LUNOKIOT_MAX_LAST_APPS] = { nullptr };
    public:
        const char *AppName() override { return "Task switcher"; };
        TaskSwitcher();
        ~TaskSwitcher();
        bool Tick();
};

#endif
