#ifndef __LUNOKIOT__SHUTDOWN_APP__
#define __LUNOKIOT__SHUTDOWN_APP__

#include "../system/Application.hpp"

class ShutdownApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw;
        unsigned long timeFromBegin;
    public:
        ShutdownApplication();
        bool Tick();
};

#endif
