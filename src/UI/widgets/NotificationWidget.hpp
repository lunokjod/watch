#ifndef __LUNOKIOT__UI__NOTIFICATION___
#define __LUNOKIOT__UI__NOTIFICATION___
#include <Arduino.h>
#include <functional>
#include <libraries/TFT_eSPI/TFT_eSPI.h>

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
        NotificationWidget(UICallback notifyTo, char *message, uint32_t color=tft->color24to16(0x353e45));
        void DrawTo(TFT_eSprite * endCanvas);
        bool isRead = false;
        ~NotificationWidget();
        int16_t x,y;
        void NotificationDraw();
        void Show();
};
#endif
