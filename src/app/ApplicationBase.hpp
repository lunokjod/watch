#ifndef __LUNOKIOT__APPLICATION__BASE__
#define __LUNOKIOT__APPLICATION__BASE__

#include "../UI/AppTemplate.hpp"

class ApplicationBase : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
    public:
        const char *AppName() override { return "NOT SET"; };
        ApplicationBase();
        ~ApplicationBase();
        bool Tick();
};

#endif
