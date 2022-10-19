#ifndef __LUNOKIOT__SIMPLEWATCHFACE_APP__
#define __LUNOKIOT__SIMPLEWATCHFACE_APP__
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

class SimpleWatchfaceApplication: public LunokIoTApplication {
    public:
        CanvasWidget * backgroundCanvas;
        CanvasWidget * watchFaceCanvas;
        CanvasWidget * hourHandCanvas;
        CanvasWidget * minuteHandCanvas;
        CanvasWidget * secondHandCanvas;
        SimpleWatchfaceApplication();
        static void NTPSync(void * data);
        bool Tick();
};

#endif
