#ifndef __LUNOKIOT__PROVISIONING2_APP__
#define __LUNOKIOT__PROVISIONING2_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include <qrcode.h>

class Provisioning2Application: public TemplateApplication {
    public:
        const char *AppName() override { return "WiFi provisioning"; };
        unsigned long nextRedraw=0;
        char *pop=nullptr;
        char *service_key=nullptr;
        char service_name[32];
        bool provisioningStarted = false;
        bool qrVisible = false;
        bool useBluetooth = false;

        TFT_eSprite * currentQRRendered = nullptr;
        QRCode currentQR;
        uint8_t *currentQRData = nullptr;;

        void GenerateQRCode();
        SwitchWidget * wifiOrBLE = nullptr;
        ButtonImageXBMWidget * clearProvBtn=nullptr;
        ButtonImageXBMWidget * startProvBtn=nullptr;
        Provisioning2Application();
        ~Provisioning2Application();
        bool Tick();
        void GenerateCredentials();
};

#endif
