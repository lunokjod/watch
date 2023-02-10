#ifndef __LUNOKIOT__NOTIFICATIONS_APP__
#define __LUNOKIOT__NOTIFICATIONS_APP__

#include "../UI/AppTemplate.hpp"

class NotificacionsApplication: public TemplateApplication {
    private:
        unsigned long nextRedraw=0;
        char *msgfrom=nullptr;
        char *msgtitle=nullptr;
        char *msgbody=nullptr;
    public:
        const char *AppName() override { return "Notification"; };
        NotificacionsApplication(const char *subject=nullptr,const char *from=nullptr,const char *body=nullptr);
        ~NotificacionsApplication();
        bool Tick();
};

#endif
