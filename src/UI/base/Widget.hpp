#ifndef __LUNOKIOT__WIDGET__BASE__
#define __LUNOKIOT__WIDGET__BASE__

#include <libraries/TFT_eSPI/TFT_eSPI.h>
//extern TFT_eSPI *tft;

class Drawable {
    public:
        static const uint32_t MASK_COLOR=0x0100; // RGB565 0x0100 //RGB888 0x002000
        //static const uint32_t MASK_COLOR2=tft->color565(0x01,0x20,0x00);
        virtual void DirectDraw(int16_t x,int16_t y)=0;
        virtual void DrawTo(TFT_eSprite * endCanvas, int16_t x=0, int16_t y=0,int32_t maskColor=MASK_COLOR)=0;
};

#endif
