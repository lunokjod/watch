#ifndef __LUNOKIOT__NOTIFICATIONS_APP__
#define __LUNOKIOT__NOTIFICATIONS_APP__
#include <Arduino_JSON.h>

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class NotificacionsApplication: public TemplateApplication {
    private:
        unsigned long nextRedraw=0;
        JSONVar notification;
        bool parsedJSON=false;
        ButtonImageXBMWidget * btnMarkRead=nullptr;
    public:
        const char *AppName() override { return "Notification"; };
        NotificacionsApplication();
        ~NotificacionsApplication();
        bool Tick();
};

#endif
