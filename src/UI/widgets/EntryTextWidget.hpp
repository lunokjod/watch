#ifndef __LUNOKIOT__UI__ENTRYTEXT___
#define __LUNOKIOT__UI__ENTRYTEXT___

#include "ButtonTextWidget.hpp"
#include <functional>
#include "../UI.hpp"
#include "../../app/LogView.hpp"

class EntryTextWidget: public ButtonTextWidget {
    public:
        char *ptrToText=nullptr;
        //TFT_eSprite * lastCanvas=nullptr;
        EntryTextWidget(int16_t x,int16_t y, int16_t h, int16_t w, const char *text=nullptr);
        void InternalRedraw() override;
        void DrawTo(TFT_eSprite * endCanvas) override;
        void ShowKeyboard();
        void HideKeyboard(char *newValue=nullptr);
        ~EntryTextWidget() {
            HideKeyboard();
            if ( nullptr != ptrToText ) { free(ptrToText); }
        }
};

#endif
