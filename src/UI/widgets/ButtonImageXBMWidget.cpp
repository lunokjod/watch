#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"
#include "ButtonImageXBMWidget.hpp"

#include "../activator/ActiveRect.hpp"
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
    _img = new CanvasWidget(xbmH+1,xbmW+1);
    _img->canvas->drawXBitmap(1,1,XBMBitmapPtr, xbmW, xbmH, TFT_BLACK);
    _img->canvas->drawXBitmap(0,0,XBMBitmapPtr, xbmW, xbmH, xbmColor);
}
ButtonImageXBMWidget::~ButtonImageXBMWidget() {
    if ( nullptr != _img ) {
        delete _img;
        _img = nullptr;
    }

}
void ButtonImageXBMWidget::DrawTo(TFT_eSprite * endCanvas) {
    if ( nullptr == endCanvas) { return; }
    if ( nullptr == canvas) { return; }

    if ( lastEnabled != enabled ) {
        _img->canvas->fillSprite(CanvasWidget::MASK_COLOR);
        uint32_t imgColor = xbmColor;
        if ( enabled ) {
            _img->canvas->drawXBitmap(1,1,XBMBitmapPtr, xbmW, xbmH, TFT_BLACK);
        } else {
            imgColor = TFT_DARKGREY;
        }
        _img->canvas->drawXBitmap(0,0,XBMBitmapPtr, xbmW, xbmH, imgColor);
        
        lastEnabled = enabled;
    }
    /*
    uint16_t iconColor = ttgo->tft->color24to16(0xffffff);
    int16_t cx = (ClearProvisioningButton->x + (ClearProvisioningButton->GetCanvas()->height()/2)-(img_trash_48_height/2));
    int16_t cy = (ClearProvisioningButton->y + (ClearProvisioningButton->GetCanvas()->width()/2)-(img_trash_48_width/2));
    this->GetCanvas()->drawXBitmap(cx,cy,img_trash_48_bits, img_trash_48_width, img_trash_48_height,iconColor);
    */
    if ( lastInteraction ) { 
        // button pushed
    }
    //ttgo->tft->setSwapBytes(true);
    ButtonWidget::DrawTo(canvas);
    _img->canvas->pushRotated(canvas,0,CanvasWidget::MASK_COLOR);
    if ( nullptr != endCanvas ){

        CanvasWidget::DrawTo(endCanvas,x,y);
    }
    //ttgo->tft->setSwapBytes(false);
}
