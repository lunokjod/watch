#ifndef __LUNOKIOT__CANVAS__WIDGET___HEADER__
#define __LUNOKIOT__CANVAS__WIDGET___HEADER__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"


class CanvasWidget {
    private:
    public:
        static const uint32_t MASK_COLOR=0x100; // ttgo->tft->color565(0x01,0x20,0x00)
        TFT_eSprite *canvas = nullptr; // create on constructor
        //unsigned long lastRefresh=0; // next forced redraw

        // BRG
        CanvasWidget(int16_t h=0, int16_t w=0);
        void RebuildCanvas(int16_t h=0, int16_t w=0);
        TFT_eSprite *GetCanvas();
        void DrawTo(TFT_eSprite * endCanvas, int16_t x=0, int16_t y=0,int32_t maskColor=MASK_COLOR);
        virtual ~CanvasWidget();
};

#endif
