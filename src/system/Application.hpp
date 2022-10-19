#ifndef __LUNOKIOT__APPLICATION__SUPPORT__
#define __LUNOKIOT__APPLICATION__SUPPORT__

#include <Arduino.h>
#include <LilyGoWatch.h>


class LunokIoTApplication {
    public:
        TFT_eSprite *canvas=nullptr;
        LunokIoTApplication();
        virtual ~LunokIoTApplication();
        TFT_eSprite *GetCanvas();
        virtual bool Tick();
        uint32_t lastApplicationHeapFree = 0;
        uint32_t lastApplicationPSRAMFree = 0;
};


void LaunchApplication(LunokIoTApplication *instance);

#endif
