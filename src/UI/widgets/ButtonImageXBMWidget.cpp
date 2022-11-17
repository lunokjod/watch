#include <Arduino.h>

#include <libraries/TFT_eSPI/TFT_eSPI.h>
//extern TFT_eSPI *tft;
#include "lunokiot_config.hpp"
#include "ButtonImageXBMWidget.hpp"
#include "../../app/LogView.hpp"

#include "CanvasWidget.hpp"
#include <functional>

ButtonImageXBMWidget::ButtonImageXBMWidget(int16_t x, int16_t y, int16_t h, int16_t w, 
                                    std::function<void ()> notifyTo, 
                                    const uint8_t *XBMBitmapPtr,
                                    int16_t xbmH, int16_t xbmW, uint16_t xbmColor,
                                    uint32_t btnBackgroundColor,
                                    bool borders)
                        : XBMBitmapPtr(XBMBitmapPtr), xbmH(xbmH), xbmW(xbmW), xbmColor(xbmColor),
                                ButtonWidget(x,y,h,w,notifyTo, btnBackgroundColor, borders) {
    lUIDeepLog("%s new %p\n",__PRETTY_FUNCTION__,this);
    InternalRedraw();

}

void ButtonImageXBMWidget::InternalRedraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    ButtonWidget::InternalRedraw(); // call button normal draw

    // add the icon
    uint32_t imgColor = xbmColor;
    if ( enabled ) {
        buffer->canvas->drawXBitmap(((w/2)-(xbmW/2))+1,((h/2)-(xbmH/2))+1,XBMBitmapPtr, xbmW, xbmH, ThCol(shadow));
    } else {
        imgColor = TFT_DARKGREY;
    }
    buffer->canvas->drawXBitmap((w/2)-(xbmW/2),(h/2)-(xbmH/2),XBMBitmapPtr, xbmW, xbmH, imgColor);
    buffer->canvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
}
