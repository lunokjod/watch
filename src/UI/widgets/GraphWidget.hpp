#ifndef __LUNOKIOT__GRAPH__WIDGET___HEADER__
#define __LUNOKIOT__GRAPH__WIDGET___HEADER__
#include <Arduino.h>
#include "lunokiot_config.hpp"
#include "CanvasWidget.hpp"

class GraphWidget : public CanvasWidget{
    private:
    public:
        int64_t minValue;
        int64_t maxValue;
        uint32_t markColor;
        uint32_t backgroundColor;
        uint32_t outColor;

        int64_t lastValue;
        CanvasWidget *graph;
        GraphWidget(int16_t h=100, int16_t w=220, int64_t minValue=0, int64_t maxValue=100, uint32_t markColor=TFT_YELLOW, uint32_t backgroundColor=CanvasWidget::MASK_COLOR, uint32_t outColor=TFT_RED);
        bool PushValue(int64_t value);
        void DrawTo(TFT_eSprite * endCanvas, int16_t x=0, int16_t y=0);
        ~GraphWidget();
};

#endif
