#ifndef __LUNOKIOT__BLEMONITORDEVICE_APP__
#define __LUNOKIOT__BLEMONITORDEVICE_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "Watchface.hpp"
#include "../system/Network.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class BLEDeviceMonitorApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        int rotateVal = 0;
        uint16_t backgroundColor;
        NimBLEAddress addr;
    public:
        ButtonImageXBMWidget * btnBack = nullptr;
        BLEDeviceMonitorApplication(uint16_t baseColor, NimBLEAddress addr);
        ~BLEDeviceMonitorApplication();
        bool Tick();
};

#endif
