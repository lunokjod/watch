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

#ifndef __LUNOKIOT__IATEST_APP__
#define __LUNOKIOT__IATEST_APP__

#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/NotificationWidget.hpp"

class IATestApplication: public LunokIoTApplication {
    public:
        const char *AppName() override { return "IA Test"; };
        NotificationWidget * testButton[5]= { nullptr };
        CanvasWidget * watchFacebackground = nullptr; // contains the back of the clock
        CanvasWidget * compositeBuffer = nullptr; // for image composite

        CanvasWidget * circleNotifications = nullptr;

        CanvasWidget * accelSphere = nullptr;
        CanvasWidget * compositeBuffer150 = nullptr;

        CanvasWidget * eyeLensBuffer = nullptr;


        IATestApplication();
        bool Tick();
};

#endif
