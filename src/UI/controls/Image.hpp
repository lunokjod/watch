#ifndef __LUNOKIOT__CONTROL_IMAGE_CLASS__
#define __LUNOKIOT__CONTROL_IMAGE_CLASS__

#include "../../lunokIoT.hpp"
#include "base/Control.hpp"
#include <LilyGoWatch.h>
#include "base/Container.hpp"

namespace LuI {

    // defines root of any screen element
    class Image: public Container {
        protected:
            const uint32_t imageWidth=0;
            const uint32_t imageHeight=0;
            const unsigned char *imageData=nullptr;
            TFT_eSprite * imageCanvas=nullptr;
        public:
            ~Image();
            Image(IN uint32_t width, IN uint32_t height, IN unsigned char *data,bool swap=false, LuI_Layout layout=LuI_Vertical_Layout, size_t childs=0);
            void Refresh(bool swap=false);
    };
};


#endif
