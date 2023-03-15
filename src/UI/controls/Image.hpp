#ifndef __LUNOKIOT__CONTROL_IMAGE_CLASS__
#define __LUNOKIOT__CONTROL_IMAGE_CLASS__

#include "../../lunokIoT.hpp"
#include "base/Control.hpp"
#include <LilyGoWatch.h>

namespace LuI {

    // defines root of any screen element
    class Image: public Control {
        protected:
            const uint32_t imageWidth=0;
            const uint32_t imageHeight=0;
            const unsigned char *imageData=nullptr;
            TFT_eSprite * imageCanvas=nullptr;
        public:
            ~Image();
            Image(IN uint32_t width, IN uint32_t height, IN unsigned char *data);
            void Refresh();
    };
};


#endif
