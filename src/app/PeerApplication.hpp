#ifndef __LUNOKIOT__APPLICATION__PEER__
#define __LUNOKIOT__APPLICATION__PEER__

#include "../UI/AppTemplate.hpp"
#include <esp_now.h>
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/CanvasWidget.hpp"

class PeerApplication : public TemplateApplication {
    private:
        int16_t currentScroll=0;
        unsigned long nextRefresh=0;
        //unsigned long nextTimestampSend=0;
        ButtonImageXBMWidget * sendHelloBtn;
        ButtonImageXBMWidget * iconPaletteBtn;
        CanvasWidget * scrollZone;
        CanvasWidget * colorCanvas;

    public:
        PeerApplication();
        ~PeerApplication();
        static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
        static void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
        bool Tick();
};

#endif
