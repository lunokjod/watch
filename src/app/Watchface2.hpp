#ifndef __LUNOKIOT__WATCHFACE2_APP__
#define __LUNOKIOT__WATCHFACE2_APP__

#include <Arduino.h>
#include "../system/Network.hpp"
#include "../system/Application.hpp"
#include "../UI/activator/ActiveRect.hpp"
#include "../UI/widgets/CanvasZWidget.hpp"

class Watchface2Application: public LunokIoTApplication {
    protected:
        unsigned long nextRefresh=0;
        CanvasZWidget * colorBuffer = nullptr;
        CanvasZWidget * outherSphere = nullptr;
        CanvasZWidget * innerSphere = nullptr;
        CanvasZWidget * hourHandCanvas = nullptr;
        CanvasZWidget * minuteHandCanvas = nullptr;
        ActiveRect * bottomRightButton = nullptr;
        int16_t middleX;
        int16_t middleY;
        int16_t radius;
        const int16_t margin = 5;
        const float DEGREE_HRS = (360/24);
        int markAngle=0;
        void Handlers();
    public:
        bool wifiEnabled = false;
        static void FreeRTOSEventReceived(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
        bool GetSecureNetworkWeather();
        bool ParseWeatherData();
        static NetworkTaskDescriptor * ntpTask;
        static NetworkTaskDescriptor * geoIPTask;
        static NetworkTaskDescriptor * weatherTask;
        Watchface2Application();
        virtual ~Watchface2Application();
        bool Tick();
};

#endif
