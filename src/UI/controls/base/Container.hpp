#ifndef __LUNOKIOT__CONTAINER_BASE_CLASS__
#define __LUNOKIOT__CONTAINER_BASE_CLASS__
#include "../../../lunokIoT.hpp"
#include "Control.hpp"
#define LuI_Layout bool
#define LuI_Horizonal_Layout true
#define LuI_Vertical_Layout false

namespace LuI {
    // defines a container
    class Container : public Control {
        protected:
            bool layout=LuI_Vertical_Layout;
            size_t currentChildSlot=0;
            size_t childNumber=1;
            Control ** children;
            float *quotaValues=nullptr;
            //bool parentRefresh=true;
        public:
            uint8_t border=0;
            bool AddChild(INOUT Control *control,IN float quota=1.0);
            Container(LuI_Layout layout=LuI_Vertical_Layout, size_t childs=1);
            ~Container();
            void EventHandler() override;
            void Refresh(bool swap=false) override;
    };
};

#endif
