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
        bool borders=true;
        uint32_t btnBackgroundColor;
        virtual ~ButtonWidget();
        void BuildCanvas(int16_t h, int16_t w);
        bool CheckBounds();
        ButtonWidget(int16_t x,int16_t y, int16_t h, int16_t w, std::function<void ()> notifyTo, uint32_t btnBackgroundColor=ttgo->tft->color24to16(0x353e45), bool borders=true);
        void DrawTo(TFT_eSprite * endCanvas);
};
#endif
