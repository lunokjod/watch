#ifndef __LUNOKIOT__APPLICATION__OTAUPDATE__
#define __LUNOKIOT__APPLICATION__OTAUPDATE__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class OTAUpdateApplication : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
        ButtonImageXBMWidget *updateBtn=nullptr;
    public:
        OTAUpdateApplication();
        ~OTAUpdateApplication();
        bool Tick();
};

#endif
