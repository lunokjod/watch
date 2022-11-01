#ifndef __LUNOKIOT__WATCHFACE_APP__
#define __LUNOKIOT__WATCHFACE_APP__
#include "lunokiot_config.hpp"

#include <Arduino.h>
#include <LilyGoWatch.h>

#include "../system/Application.hpp"

#include "../UI/activator/ActiveRect.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"

#include "../system/SystemEvents.hpp"
#include "../system/Network.hpp"


class WatchfaceApplication: public LunokIoTApplication {
    public:
        bool showOverlay=false;
        GraphWidget * uploadMonitor = nullptr;
        //TaskHandle_t NTPTaskHandler = NULL;
        CanvasWidget * backlightCanvas = nullptr;
        CanvasWidget * backgroundCanvas = nullptr;
        CanvasWidget * marksCanvas = nullptr;
        //TFT_eSprite * marksCanvasCache = nullptr;
        static NetworkTaskDescriptor * ntpTask;
        static NetworkTaskDescriptor * geoIPTask;
        static NetworkTaskDescriptor * weatherTask;
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

        ActiveRect * topRightButton = nullptr;
        ActiveRect * bottomRightButton = nullptr;
        ~WatchfaceApplication();
        WatchfaceApplication();
        static void NTPSync(void * data);
        static bool GetSecureNetworkWeather();
        static bool ParseWeatherData();
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
