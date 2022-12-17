#ifndef __LUNOKIOT__APPLICATION__OTAUPDATE__
#define __LUNOKIOT__APPLICATION__OTAUPDATE__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GaugeWidget.hpp"
#include <esp_https_ota.h>

class OTAUpdateApplication : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
        unsigned long nextProgressbarRefresh=0;
        ButtonImageXBMWidget *updateBtn=nullptr;
        GaugeWidget * progressBar=nullptr;
        int imageSize=0;
        int imageDownloaded=0;
        esp_https_ota_handle_t https_ota_handle = NULL;
        char percentAsString[5] = "0%";
    public:
        OTAUpdateApplication();
        ~OTAUpdateApplication();
        bool Tick();
};

#endif
