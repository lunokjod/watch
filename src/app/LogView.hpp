#ifndef __LUNOKIOT__LOGVIEW_APP__
#define __LUNOKIOT__LOGVIEW_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "Watchface.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"

void lLog(const char *fmt, ...);

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
