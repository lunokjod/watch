#ifndef __LUNOKIOT__UI__ENTRYTEXT___
#define __LUNOKIOT__UI__ENTRYTEXT___

#include "ButtonTextWidget.hpp"
#include <functional>
#include "../UI.hpp"

class EntryTextWidget: public ButtonTextWidget {
    public:
        EntryTextWidget(int16_t x,int16_t y, int16_t h, int16_t w, const char *text=nullptr);
        void InternalRedraw() override;
};

#endif
