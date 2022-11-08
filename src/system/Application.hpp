#ifndef __LUNOKIOT__APPLICATION__SUPPORT__
#define __LUNOKIOT__APPLICATION__SUPPORT__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Application.hpp"

/*
 * Basic application with UI
 */
class LunokIoTApplication {
    public:
        TFT_eSprite *canvas=nullptr; // application buffer

        LunokIoTApplication();
        virtual ~LunokIoTApplication();
        
        TFT_eSprite *GetCanvas(); // almost unused... but is a good practice

        virtual bool Tick(); // the UI callback to manage input and screen

#ifdef LUNOKIOT_DEBUG
        uint32_t lastApplicationHeapFree = 0;
        uint32_t lastApplicationPSRAMFree = 0;
#endif
};


/*
 * Push the application to foreground
 */
void LaunchApplication(LunokIoTApplication *instance,bool animation=true);

#endif
