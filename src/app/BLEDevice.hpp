#ifndef __LUNOKIOT__BLEMONITORDEVICE_APP__
#define __LUNOKIOT__BLEMONITORDEVICE_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>

#include "../system/Network.hpp"
#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonTextWidget.hpp"

class BLEDeviceMonitorApplication: public TemplateApplication {
    private:
        unsigned long nextRedraw=0;
        int rotateVal = 0;
        uint16_t backgroundColor;
        char *deviceMAC=nullptr;
        ButtonTextWidget *pairBtn=nullptr;
        // copy of lBLEDevice
        char * devName = nullptr;
        unsigned long firstSeen = 0; // @TODO use time_t instead
        unsigned long lastSeen = 0;
        size_t seenCount = 0;
        int rssi = 0;
        int8_t txPower = 0;
        double distance = -1;

    public:
        const char *AppName() override { return "BLE device inspector"; };
        BLEDeviceMonitorApplication(uint16_t baseColor, char *devMAC);
        ~BLEDeviceMonitorApplication();
        bool Tick();
};

#endif
