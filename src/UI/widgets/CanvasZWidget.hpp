#ifndef __LUNOKIOT__CANVAS__Z_WIDGET___HEADER__
#define __LUNOKIOT__CANVAS__Z_WIDGET___HEADER__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"
#include "CanvasWidget.hpp"


class CanvasZWidget : public CanvasWidget {
    private:
        float z=1; // don't touch this value direcly, only for read (@TODO getter and private)
    public:
        float GetScale() { return z; } // read only, use draw to set again
        CanvasZWidget(int16_t h=0, int16_t w=0, float z=1.0);
        void DrawTo(TFT_eSprite * endCanvas, int16_t x=0, int16_t y=0, float z=1.0, bool centered=false, int32_t maskColor=MASK_COLOR);
        virtual ~CanvasZWidget();
    /*
        static const uint32_t MASK_COLOR=0x100; // ttgo->tft->color565(0x01,0x20,0x00)
        TFT_eSprite *canvas = nullptr;
        unsigned long lastRefresh=0;
        // BRG
        void RebuildCanvas(int16_t h=0, int16_t w=0);
        TFT_eSprite *GetCanvas();
        */
};

#endif
