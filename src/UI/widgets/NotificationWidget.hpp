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

#ifndef __LUNOKIOT__UI__NOTIFICATION___
#define __LUNOKIOT__UI__NOTIFICATION___
#include <Arduino.h>
#include <functional>
//#include <libraries/TFT_eSPI/TFT_eSPI.h>

#include "lunokiot_config.hpp"

#include "CanvasWidget.hpp"

#include "ButtonWidget.hpp"

class NotificationWidget: public ButtonWidget {
    public:
        //SemaphoreHandle_t changeData = NULL;
        static const unsigned long ShowTime = 8000;
        static const unsigned long DissolveTime = 3000;
        unsigned long ctime=0;
        unsigned long dtime=0;
        bool byteSwap = false;
        CanvasWidget * buffer = nullptr;
        char * message=nullptr;
        uint32_t color=0x0;
        bool Interact(bool touch, int16_t tx,int16_t ty);
        NotificationWidget(UICallback notifyTo, char *message, uint32_t color=TFT_DARKGREY); //Get16BitFromRGB(0x353e45));
        void DrawTo(TFT_eSprite * endCanvas);
        bool isRead = false;
        ~NotificationWidget();
        int16_t x,y;
        void NotificationDraw();
        void Show();
};
#endif
