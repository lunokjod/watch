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

#ifndef __LUNOKIOT__WATCHFACE_ALWAYS_ON__
#define __LUNOKIOT__WATCHFACE_ALWAYS_ON__

#include "../system/Application.hpp"

class WatchfaceAlwaysOn : public LunokIoTApplication {
    private:
        unsigned long nextRefresh=0;
    public:
        const char *AppName() override { return "Always on"; };
        WatchfaceAlwaysOn();
        ~WatchfaceAlwaysOn();
        virtual const bool isWatchface() { return true; }
        virtual const bool mustShowAsTask() { return false; } // false=Not show in task switcher (menus and intermediate views must be hidden)
        bool Tick();
};

#endif
