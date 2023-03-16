#ifndef __LUNOKIOT__BUTTON_CLASS__
#define __LUNOKIOT__BUTTON_CLASS__
#include "../../lunokIoT.hpp"
#include "base/Container.hpp"
#include "../UI.hpp"

namespace LuI {
    // defines a button
    class Button : public Container {
        protected:
            uint16_t color=ThCol(button);
        public:
            void Refresh(bool swap=false) override;
            Button(LuI_Layout layout, size_t childs,uint16_t color=ThCol(button));
            void EventHandler() override;
    };
};

#endif
