#ifndef __LUNOKIOT__IATEST_APP__
#define __LUNOKIOT__IATEST_APP__

#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/NotificationWidget.hpp"

class IATestApplication: public LunokIoTApplication {
    public:
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
