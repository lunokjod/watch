#ifndef __LUNOKIOT__UI__SCROLLVIEW___
#define __LUNOKIOT__UI__SCROLLVIEW___
//ScrollViewWidget.hpp
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include <functional>

class ScrollViewWidget: public ActiveRect, public CanvasWidget {
    public:
        uint32_t offsetX=0;
        uint32_t offsetY=0;
        uint32_t btnBackgroundColor;
        virtual ~ScrollViewWidget();
        void BuildCanvas(int16_t h, int16_t w);
        //bool CheckBounds();
        CanvasWidget * backroundView;
        CanvasWidget *_img;
        ScrollViewWidget(int16_t x,int16_t y, int16_t h, int16_t w, uint32_t btnBackgroundColor, CanvasWidget *view);
        void DrawTo(TFT_eSprite * endCanvas);
};
#endif
