#ifndef __LUNOKIOT__UI__BUTTON___
#define __LUNOKIOT__UI__BUTTON___

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include <functional>

class ButtonWidget: public ActiveRect, public CanvasWidget {
    public:
        bool borders=true; // want borders or only ActiveRect?
        uint32_t btnBackgroundColor; // optional color
        virtual ~ButtonWidget();
        void BuildCanvas(int16_t h, int16_t w);
        bool CheckBounds(); // rebuild the canvas if is changed (future animations/transitions) don't need to call directly
        ButtonWidget(int16_t x,int16_t y, int16_t h, int16_t w, std::function<void ()> notifyTo, uint32_t btnBackgroundColor=ThCol(button), bool borders=true);
        void DrawTo(TFT_eSprite * endCanvas);
        void DirectDraw();
};
#endif
