#ifndef __LUNOKIOT__LOGVIEW_APP__
#define __LUNOKIOT__LOGVIEW_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "Watchface.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"

void lLog(const char *fmt, ...);

// other log flavours

#ifdef LUNOKIOT_DEBUG_APPLICATION
#define lAppLog(...) { lLog("[APP] "); lLog(__VA_ARGS__); } 
#else 
#define lAppLog(...);
#endif


#ifdef LUNOKIOT_DEBUG_EVENTS
#define lEvLog(...) { lLog("[EV] "); lLog(__VA_ARGS__); } 
#else 
#define lEvLog(...);
#endif

#ifdef LUNOKIOT_DEBUG_NETWORK
#define lNetLog(...) { lLog("[NET] "); lLog(__VA_ARGS__); } 
#else 
#define lNetLog(...);
#endif

#ifdef LUNOKIOT_DEBUG_UI
#define lUILog(...) { lLog("[UI] "); lLog(__VA_ARGS__); } 
#else 
#define lUILog(...);
#endif

class LogViewApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
    public:
        static TFT_eSprite * LogTextBuffer;
        static const uint8_t TextHeight=9;
        ButtonImageXBMWidget * btnBack = nullptr;
        LogViewApplication();
        ~LogViewApplication();
        bool Tick();
};

#endif
