#ifndef __LUNOKIOT__UI__BUTTONTEXT___
#define __LUNOKIOT__UI__BUTTONTEXT___

#include "ButtonWidget.hpp"
#include <functional>
#include "../UI.hpp"

class ButtonTextWidget: public ButtonWidget {
    public:
        ButtonTextWidget(int16_t x,int16_t y, int16_t h, int16_t w, UICallback notifyTo=nullptr, const char *text=nullptr, uint32_t btnTextColor=ThCol(text), uint32_t btnBackgroundColor=ThCol(button), bool borders=true);
        uint32_t btnTextColor;
        void InternalRedraw() override;
        const char * label=nullptr;
};

#endif
