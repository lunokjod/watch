//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//
// LunokWatch is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later 
// version.
//
// LunokWatch is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
// details.
//
// You should have received a copy of the GNU General Public License along with 
// LunokWatch. If not, see <https://www.gnu.org/licenses/>. 
//

#ifndef __LUNOKIOT__BUTTON_CLASS__
#define __LUNOKIOT__BUTTON_CLASS__
#include "../../lunokIoT.hpp"
#include "base/Container.hpp"
#include "../UI.hpp"

namespace LuI {
    #define NO_DECORATION false
    // defines a button
    class Button : public Container {
        protected:
            uint16_t color=ThCol(button);
            bool decorations=true;
        public:
            void Refresh(bool direct=false) override;
            Button(LuI_Layout layout=LuI_Vertical_Layout,size_t childs=1,bool decorations=true,uint16_t color=ThCol(button));
            void EventHandler() override;
    };
};

#endif
