#ifndef __LUNOKIOT__UI__BUTTONIMAGE___
#define __LUNOKIOT__UI__BUTTONIMAGE___

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"

#include "../activator/ActiveRect.hpp"
#include "ButtonWidget.hpp"
#include "CanvasWidget.hpp"
#include <functional>

class ButtonImageXBMWidget: public ButtonWidget {
    public:
        CanvasWidget *_img;
        uint32_t xbmColor;
        const uint8_t *XBMBitmapPtr;
        int16_t xbmH;
        int16_t xbmW;
        bool lastEnabled = true;
        virtual ~ButtonImageXBMWidget();
        ButtonImageXBMWidget(int16_t x,int16_t y, int16_t h, int16_t w, 
                        std::function<void ()> notifyTo, 
                        const uint8_t *XBMBitmapPtr,
                        int16_t xbmH, int16_t xbmW, uint16_t xbmColor=ThCol(text),
                        uint32_t btnBackgroundColor=ThCol(button), bool borders=true);
        void DrawTo(TFT_eSprite * endCanvas);
};
#endif
