#ifndef __LUNOKIOT__APPLICATION__LAMP__
#define __LUNOKIOT__APPLICATION__LAMP__

#include "../system/Application.hpp"

class LampApplication : public LunokIoTApplication {
    public:
        LampApplication();
        ~LampApplication();
        bool Tick();
};

#endif
