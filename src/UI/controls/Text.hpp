#ifndef __LUNOKIOT__CONTROL_TEXT_CLASS__
#define __LUNOKIOT__CONTROL_TEXT_CLASS__

#include "../../lunokIoT.hpp"
#include "base/Control.hpp"
#include <LilyGoWatch.h>

namespace LuI {

    // defines root of any screen element
    class Text: public Control {
        protected:
            const uint32_t imageTextWidth=0;
            const uint32_t imageTextHeight=0;
            TFT_eSprite * imageTextCanvas=nullptr;
            const GFXfont *font=nullptr;
            const char * text=nullptr;
            uint16_t color=TFT_WHITE; 
            bool swapColor=false;
        public:
            ~Text();
            Text(IN char * what, IN uint16_t color=TFT_WHITE,IN bool swap=false, IN GFXfont *font=&FreeMonoBold9pt7b);
            void Refresh(bool swap=false);
    };
};


#endif
