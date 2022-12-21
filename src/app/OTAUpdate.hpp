#ifndef __LUNOKIOT__APPLICATION__OTAUPDATE__
#define __LUNOKIOT__APPLICATION__OTAUPDATE__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GaugeWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"
#include <esp_https_ota.h>

class OTAUpdateApplication : public TemplateApplication {
    private:
        uint32_t backgroundColor=ThCol(background);
        unsigned long nextRefresh=0;
        //unsigned long nextProgressbarRefresh=0;
        ButtonImageXBMWidget *updateBtn=nullptr;
        GaugeWidget * progressBar=nullptr;
        //GraphWidget * OTADownloadSpeed=nullptr;
        int imageSize=0;
        int imageDownloaded=0;
        char percentAsString[5] = "0%";
        char *bufferForText = nullptr;

    public:
        OTAUpdateApplication();
        ~OTAUpdateApplication();
        bool Tick();
};

#endif
