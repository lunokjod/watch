#ifndef __LUNOKIOT__APPLICATION__PLAYGROUND13__
#define __LUNOKIOT__APPLICATION__PLAYGROUND13__

#include "../UI/AppTemplate.hpp"
#include <esp_now.h>

class PlaygroundApplication13 : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
    public:
        const char *AppName() override { return "Playground13"; };
        PlaygroundApplication13();
        ~PlaygroundApplication13();
        bool Tick();
        static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
        static void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
};

#endif
