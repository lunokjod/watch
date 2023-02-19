#ifndef __LUNOKIOT__APPLICATION__LAMP__
#define __LUNOKIOT__APPLICATION__LAMP__

#include "../system/Application.hpp"

class LampApplication : public LunokIoTApplication {
    public:
        const char *AppName() override { return "Lamp"; };
        LampApplication();
        //~LampApplication();
        const bool mustShowAsTask() override { return false; }
        bool Tick();
};

#endif
