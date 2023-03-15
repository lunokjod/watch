#ifndef __LUNOKIOT__CONTROL_BASE_CLASS__
#define __LUNOKIOT__CONTROL_BASE_CLASS__

#include "../../../lunokIoT.hpp"
#include <LilyGoWatch.h>

namespace LuI {

    // defines root of any screen element
    class Control {
        protected:
            TFT_eSprite *canvas=nullptr;
            Control *parent=nullptr;
            uint32_t height=0;
            uint32_t width=0;
        public:
            TFT_eSprite * GetCanvas();
            bool Attach(INOUT Control *control);
            virtual ~Control();
            Control(uint32_t w=100, uint32_t h=100);
            void SetSize(uint32_t w, uint32_t h);
            void SetWidth(uint32_t w);
            void SetHeight(uint32_t h);
            virtual void Refresh();
    };
};


#endif
