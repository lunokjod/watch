#ifndef __LUNOKIOT__UI__VALUE_SELECTOR___
#define __LUNOKIOT__UI__VALUE_SELECTOR___

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include "../UI.hpp"

class ValueSelector: public ActiveRect, public CanvasWidget {
    public:
        bool showsign;
        int16_t x;
        int16_t y;
        int32_t valMin;
        int32_t valMax;
        int32_t selectedValue=0;
        uint32_t backgroundColor;
        uint32_t textColor=ThCol(text);
        CanvasWidget * buffer=nullptr;
        virtual ~ValueSelector();
        bool Interact(bool touch, int16_t tx,int16_t ty);
        ValueSelector(int16_t x,int16_t y, int16_t h, int16_t w,int32_t valMin=0,int32_t valMax=100,uint32_t backgroundColor=ThCol(background),bool showsign=false);
        void DrawTo(TFT_eSprite * endCanvas);
};
typedef class ValueSelector MujiValue; // thanks man! :-*

#endif
