#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"
#include "SwitchWidget.hpp"

#include "../../app/LogView.hpp"

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include <functional>
SwitchWidget::~SwitchWidget() {
    delete buffer;
}
SwitchWidget::SwitchWidget(int16_t x, int16_t y, std::function<void ()> notifyTo, uint32_t switchColor)
                : ActiveRect(x,y,64,64,notifyTo), CanvasWidget(64,64), switchColor(switchColor) {
    lUIDeepLog("%s new %p\n",__PRETTY_FUNCTION__,this);

    buffer = new CanvasWidget(64,64);
    InternalRedraw();
}
void SwitchWidget::InternalRedraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    buffer->canvas->fillSprite(CanvasWidget::MASK_COLOR);
    int16_t border = 2;
    buffer->canvas->fillRoundRect(0,22,64,20,10,ThCol(dark));
    buffer->canvas->fillRoundRect(border,22+border,64-(border*2),20-(border*2),8, switchColor);
    const int32_t dotSize = 8;
    if ( switchEnabled ) {

        int16_t enabledColor = TFT_DARKGREY;
        if ( enabled ) { enabledColor = TFT_WHITE; }

        uint32_t lightColor = canvas->alphaBlend(128,switchColor,enabledColor);
        buffer->canvas->fillCircle(64-20,32,switchHeight,ThCol(shadow));
        buffer->canvas->fillCircle(64-20,32,switchHeight-border,lightColor);
        uint32_t moreLightColor = canvas->alphaBlend(128,lightColor,enabledColor);
        buffer->canvas->fillCircle(64-20,32,dotSize,moreLightColor); // decorative dot remarks true
    } else {
        uint32_t darkColor = canvas->alphaBlend(128,switchColor,ThCol(shadow));
        buffer->canvas->fillCircle(20,32,switchHeight,ThCol(shadow));
        buffer->canvas->fillCircle(20,32,switchHeight-border,darkColor);
        buffer->canvas->fillCircle(20,32,dotSize,ThCol(darken)); // decorative dot remarks false
    }
    buffer->canvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
}
void SwitchWidget::DirectDraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    buffer->canvas->pushSprite(x,y,Drawable::MASK_COLOR);
}

void SwitchWidget::DrawTo(TFT_eSprite * endCanvas) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    if ( nullptr != endCanvas ) { CanvasWidget::DrawTo(endCanvas,x,y); }
}

bool SwitchWidget::Interact(bool touch, int16_t tx,int16_t ty) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    bool interact = ActiveRect::Interact(touch,tx, ty);
    if (interact != lastSwitchValue ) {
        if ( lastSwitchValue ) { switchEnabled = !switchEnabled; }
        lastSwitchValue = interact;
        InternalRedraw();
        DirectDraw();
    }
    return interact;
}
