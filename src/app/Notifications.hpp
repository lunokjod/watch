#ifndef __LUNOKIOT__NOTIFICATIONS_APP__
#define __LUNOKIOT__NOTIFICATIONS_APP__

#include "../system/Application.hpp"

class NotificacionsApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
    public:
        const char *AppName() override { return "Notifications"; };
        NotificacionsApplication();
        ~NotificacionsApplication();
        bool Tick();
};

#endif
