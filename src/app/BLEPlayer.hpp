#ifndef __LUNOKIOT__APPLICATION__BLEPLAYER__
#define __LUNOKIOT__APPLICATION__BLEPLAYER__
#include <Arduino_JSON.h>
#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class BLEPlayerApplication : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
        JSONVar lastPlayerInfoEntry;
        bool parsedInfoJSON=false;
        JSONVar lastPlayerStateEntry;
        bool parsedStateJSON=false;
        long difference=0;
        int dur=-1;
        int n=-1;
        int c=-1;
        int repeat=0;
        int shuffle=0;
        int position=-1;
        ButtonImageXBMWidget * playBtn = nullptr;
        ButtonImageXBMWidget * pauseBtn = nullptr;
        unsigned long nextDBRefresh=0;
        void DBRefresh();
    public:
        const char *AppName() override { return "BLE Player"; };
        BLEPlayerApplication();
        ~BLEPlayerApplication();
        bool Tick();
};

#endif
