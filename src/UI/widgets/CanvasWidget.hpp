#ifndef __LUNOKIOT__CANVAS__WIDGET___HEADER__
#define __LUNOKIOT__CANVAS__WIDGET___HEADER__

#include <libraries/TFT_eSPI/TFT_eSPI.h>

class CanvasWidget {
    public:
        static const uint32_t MASK_COLOR=0x100; // ttgo->tft->color565(0x01,0x20,0x00)
        TFT_eSprite *canvas=nullptr; // must be nullptr on creation
        // BRG
        CanvasWidget(int16_t h, int16_t w);
        void RebuildCanvas(int16_t h=0, int16_t w=0);
        void DirectDraw(int16_t x,int16_t y);
        void DrawTo(TFT_eSprite * endCanvas, int16_t x=0, int16_t y=0,int32_t maskColor=MASK_COLOR);
        virtual ~CanvasWidget();
};

#endif
