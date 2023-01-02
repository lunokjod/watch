#ifndef __LUNOKIOT__APPLICATION__SUPPORT__
#define __LUNOKIOT__APPLICATION__SUPPORT__

#include <Arduino.h>
//#include <TTGO.h>
//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include <LilyGoWatch.h>
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
        virtual bool isWatchface() { return false; }
        virtual const char *AppName() = 0; //{ return "NO NAME APP"; };
};


class LaunchApplicationDescriptor {
    public:
        LunokIoTApplication *instance;
        bool animation;
};

/*
 * Push the application to foreground
 */
void LaunchApplication(LunokIoTApplication *instance,bool animation=true,bool synced=false);
void LaunchWatchface(bool animation=true);
void LaunchApplicationTaskSync(LaunchApplicationDescriptor * appDescriptor,bool synched=false);
#endif
