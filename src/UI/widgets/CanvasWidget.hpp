#ifndef __LUNOKIOT__CANVAS__WIDGET___HEADER__
#define __LUNOKIOT__CANVAS__WIDGET___HEADER__
#include <Arduino.h>
#include <LilyGoWatch.h>
//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include "../base/Widget.hpp"

class CanvasWidget : public Drawable {
    public:
        int8_t colorDepth=16;
        TFT_eSprite *canvas=nullptr; // must be nullptr on creation
        // BRG
        CanvasWidget(int16_t h, int16_t w, int8_t colorDepth=16);
        bool RebuildCanvas(int16_t h=0, int16_t w=0, int8_t colorDepth=16);
        void DirectDraw(int16_t x,int16_t y);
        void DrawTo(TFT_eSprite * endCanvas, int16_t x=0, int16_t y=0,int32_t maskColor=MASK_COLOR);
        virtual ~CanvasWidget();
};

#endif
