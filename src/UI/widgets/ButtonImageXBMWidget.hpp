#ifndef __LUNOKIOT__UI__BUTTONIMAGE___
#define __LUNOKIOT__UI__BUTTONIMAGE___

#include "ButtonWidget.hpp"
#include <functional>
#include "../UI.hpp"
class ButtonImageXBMWidget: public ButtonWidget {
    public:
        uint32_t xbmColor;
        const uint8_t *XBMBitmapPtr;
        int16_t xbmH;
        int16_t xbmW;
        void InternalRedraw() override;
        ButtonImageXBMWidget(int16_t x,int16_t y, int16_t h, int16_t w, 
                        UICallback notifyTo, 
                        const uint8_t *XBMBitmapPtr,
                        int16_t xbmH, int16_t xbmW, uint16_t xbmColor=ThCol(text),
                        uint32_t btnBackgroundColor=ThCol(button), bool borders=true);
};
#endif
