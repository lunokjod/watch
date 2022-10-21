#ifndef __LUNOKIOT__WATCHFACE_APP__
#define __LUNOKIOT__WATCHFACE_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"
#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/ButtonWidget.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/NotificationWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../UI/representation/Point2D.hpp"
#include "../UI/widgets/GraphWidget.hpp"
#include "../UI/widgets/GaugeWidget.hpp"
#include "../system/SystemEvents.hpp"

class WatchfaceApplication: public LunokIoTApplication {
    public:
        //TaskHandle_t NTPTaskHandler = NULL;
        CanvasWidget * backlightCanvas = nullptr;
        CanvasWidget * backgroundCanvas = nullptr;
        CanvasWidget * marksCanvas = nullptr;
        //TFT_eSprite * marksCanvasCache = nullptr;
        NetworkTaskDescriptor * ntpTask = nullptr;
        NetworkTaskDescriptor * weatherTask = nullptr;
        CanvasWidget * watchFaceCanvas;
        //CanvasWidget * dayNightCanvas;
        //CanvasWidget * dayNightCacheCanvas;
        //CanvasWidget * batteryCanvas;
        //CanvasWidget * batteryCacheCanvas;
        //CanvasWidget * usbCanvas;

        //CanvasWidget * weatherCanvas;


        CanvasWidget * hourHandCanvas = nullptr;
        TFT_eSprite * hourClockHandCache = nullptr;
        CanvasWidget * minuteHandCanvas = nullptr;
        TFT_eSprite * minuteClockHandCache = nullptr;
        //CanvasWidget * secondHandCanvas;

        ActiveRect * bottomRightButton = nullptr;
        ~WatchfaceApplication();
        WatchfaceApplication();
        static void NTPSync(void * data);
        static void GetSecureNetworkWeather();
        static void ParseWeatherData();
        bool Tick();
        bool wifiEnabled = false;
        int cycleHour = 0;
        int lastCycleHour = -1;
        int lastBattPC = 0;
        unsigned long lastBattPCTimeMs = 100;
        static void FreeRTOSEventReceived(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
        bool alreadyWorking = false;

        struct tm timeinfo;

        int lastSec=-1;
        int lastMin=-1;
        int lastHour=-1;
        double subsTouchDragAngle = 0;
        double lastSphereRotation=-1;
        bool lastTouched = false;
        unsigned long remainingBattTimeS;
        void PrepareDataLayer();

};

#endif
