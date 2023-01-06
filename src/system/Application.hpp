#ifndef __LUNOKIOT__APPLICATION__SUPPORT__
#define __LUNOKIOT__APPLICATION__SUPPORT__

#include <Arduino.h>
//#include <TTGO.h>
//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include <LilyGoWatch.h>
#include <functional>

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
        TFT_eSprite *ScreenCapture();
        virtual const bool isWatchface() { return false; } // used to determine if the app must be replaced with watchface when PEK button
        virtual const char *AppName() = 0; // app title
        virtual const bool mustShowAsTask() { return true; } // false=Not show in task switcher (menus and intermediate views must be hidden)
        virtual void LowMemory(); // called when system fails to allocate memory (malloc)
        //virtual RunApplicationCallback GetRunCallback();
};

typedef std::function<LunokIoTApplication*()> RunApplicationCallback;

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
