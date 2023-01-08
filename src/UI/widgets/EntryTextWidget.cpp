#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"
#include "EntryTextWidget.hpp"
#include "ButtonTextWidget.hpp"
#include "../../app/LogView.hpp"
#include "../UI.hpp"
#include "CanvasWidget.hpp"
#include <functional>

EntryTextWidget::EntryTextWidget(int16_t x,int16_t y, int16_t h, int16_t w, 
            const char *text) 
            : ButtonTextWidget(x,y,h,w,[](void *unused){

            },text,TFT_BLACK,TFT_WHITE,true) {
    lUIDeepLog("%s new %p\n",__PRETTY_FUNCTION__,this);
    InternalRedraw();
}

void EntryTextWidget::InternalRedraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    ButtonWidget::InternalRedraw(); // call button normal draw
    // add the text
    if ( nullptr != label ) {
        buffer->canvas->setTextFont(0);
        buffer->canvas->setTextSize(3);
        buffer->canvas->setTextColor(btnTextColor);
        buffer->canvas->setTextDatum(CC_DATUM);
        buffer->canvas->drawString(label,(w/2),(h/2));
    }
    buffer->canvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
}
