#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"
#include "ButtonWidget.hpp"

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include <functional>

ButtonWidget::~ButtonWidget() {

}
ButtonWidget::ButtonWidget(int16_t x, int16_t y, int16_t h, int16_t w, std::function<void ()> notifyTo, uint32_t btnBackgroundColor, bool borders)
                : ActiveRect(x,y,h,w,notifyTo), CanvasWidget(h,w), btnBackgroundColor(btnBackgroundColor), borders(borders) {
}
bool ButtonWidget::CheckBounds() {
    if ( nullptr == canvas) { return false; }

    if ( ( ActiveRect::w != canvas->width() ) || ( ActiveRect::h != canvas->height() ) ) {
        Serial.printf("ButtonWidget: New size for %p requested: Regenerating canvas...\n", this);
        RebuildCanvas(ActiveRect::h,ActiveRect::w);
        return true;
    }
    return false;
}

void ButtonWidget::DrawTo(TFT_eSprite * endCanvas) {
    if ( nullptr == canvas) { return; }
    CheckBounds();

    const int32_t radius = 8;
    int32_t border = 2;
    uint32_t buttonColor = btnBackgroundColor;

    if ( borders ) {
        // why 0? remember, every object has their own canvas, their coordinates are relative to their own canvas :)
        canvas->fillRoundRect( 0, 0,ActiveRect::w,ActiveRect::h,radius,0x0);

        border = 3;
        if ( false == enabled ) { border = 4; }
        if ( false == enabled ) { buttonColor = ttgo->tft->color24to16(0x494949); } //ttgo->tft->color565(164,128,128); }
        canvas->fillRoundRect( border, border,(ActiveRect::w-(border*2)),(ActiveRect::h-(border*2)),radius,buttonColor);
    } else {
        // clear is in case of no borders (avoid glitch with lastInteraction)
        // @TODO replace with fillsprite
        canvas->fillRoundRect( 0, 0,ActiveRect::w,ActiveRect::h,radius,CanvasWidget::MASK_COLOR);
    }

    if ( lastInteraction ) {
        border = 6;
        uint16_t lightColor = canvas->alphaBlend(128,buttonColor,TFT_WHITE);
        canvas->fillRoundRect( border, border,(ActiveRect::w-(border*2)),(ActiveRect::h-(border*2)),radius,lightColor);
    }
    if ( nullptr != endCanvas ) {
        CanvasWidget::DrawTo(endCanvas,x,y);
    }
}