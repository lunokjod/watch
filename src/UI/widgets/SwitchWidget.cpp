#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"
#include "SwitchWidget.hpp"

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include <functional>
SwitchWidget::~SwitchWidget() {
    if (nullptr != buffer ) {
        delete buffer;
        buffer = nullptr;
    }

}
SwitchWidget::SwitchWidget(int16_t x, int16_t y, std::function<void ()> notifyTo, uint32_t switchColor)
                : ActiveRect(x,y,64,64,notifyTo), CanvasWidget(64,64), switchColor(switchColor) {

    buffer = new CanvasWidget(64,64);
}

void SwitchWidget::DrawTo(TFT_eSprite * endCanvas) {
    if ( nullptr == buffer->canvas) { return; }
    buffer->canvas->fillSprite(CanvasWidget::MASK_COLOR);

    int16_t border = 2;
    buffer->canvas->fillRoundRect(0,22,64,20,10,TFT_BLACK);
    buffer->canvas->fillRoundRect(border,22+border,64-(border*2),20-(border*2),8, switchColor);

    const int32_t dotSize = 8;
    if ( switchEnabled ) {
        int16_t enabledColor = TFT_DARKGREY;
        if ( enabled ) {
            enabledColor = TFT_WHITE;
        }
        uint32_t lightColor = canvas->alphaBlend(128,switchColor,enabledColor);
        buffer->canvas->fillCircle(64-20,32,switchHeight,TFT_BLACK);
        buffer->canvas->fillCircle(64-20,32,switchHeight-border,lightColor);
        uint32_t moreLightColor = canvas->alphaBlend(128,lightColor,enabledColor);
        buffer->canvas->fillCircle(64-20,32,dotSize,moreLightColor); // decorative dot remarks true
    } else {
        uint32_t darkColor = canvas->alphaBlend(128,switchColor,TFT_BLACK);
        buffer->canvas->fillCircle(20,32,switchHeight,TFT_BLACK);
        buffer->canvas->fillCircle(20,32,switchHeight-border,darkColor);
        //buffer->canvas->fillCircle(20,32,dotSize,TFT_BLACK);
    }

    canvas->fillSprite(CanvasWidget::MASK_COLOR);
    buffer->canvas->pushRotated(canvas,0,CanvasWidget::MASK_COLOR);

    if ( nullptr != endCanvas ) {
        CanvasWidget::DrawTo(endCanvas,x,y);
    }
}

bool SwitchWidget::Interact(bool touch, int16_t tx,int16_t ty) {
    bool interact = ActiveRect::Interact(touch,tx, ty);
    if ( interact ) { switchEnabled = !switchEnabled; }
    return interact;
    /*
    // this work must be done before launch the callback
    if ( touch ) {
        bool inrect = ActiveRect::InRect(tx,ty,x,y,h,w);
        if ( inrect ) { switchEnabled = !switchEnabled; Serial.println("CHANGE"); }
    }
    return ActiveRect::Interact(touch,tx, ty);
    */
}
