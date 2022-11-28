#ifndef __LUNOKIOT__APPLICATION__SUPPORT__
#define __LUNOKIOT__APPLICATION__SUPPORT__

#include <Arduino.h>
#include <libraries/TFT_eSPI/TFT_eSPI.h>

/*
 * Basic application with UI
 */
class LunokIoTApplication {
    public:
        TFT_eSprite *canvas; // application buffer (all must draw here)
        // build canvas
        LunokIoTApplication();
        virtual ~LunokIoTApplication();
        virtual bool Tick(); // the UI callback to manage input and screen
        TFT_eSprite *NewScreenShoot();
#ifdef LUNOKIOT_DEBUG
        uint32_t lastApplicationHeapFree = 0;
        uint32_t lastApplicationPSRAMFree = 0;
#endif
};


/*
 * Push the application to foreground
 */
void LaunchApplication(LunokIoTApplication *instance,bool animation=true);
void LaunchWatchface(bool animation=true);

#endif
