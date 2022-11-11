#ifndef __LUNOKIOT__UI__SWITCH___
#define __LUNOKIOT__UI__SWITCH___

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include <functional>
#include "../UI.hpp"

class SwitchWidget: public ActiveRect, public CanvasWidget {
    public:
        CanvasWidget * buffer;
        bool switchEnabled=false;
        uint32_t switchColor;
        const int32_t switchHeight = 20; //@TODO this must define canvas sizes
        SwitchWidget(int16_t x, int16_t y, std::function<void ()> notifyTo, uint32_t switchColor=ThCol(button));
        virtual ~SwitchWidget();
        virtual void DrawTo(TFT_eSprite * endCanvas);
        bool Interact(bool touch, int16_t tx,int16_t ty);
};
#endif
