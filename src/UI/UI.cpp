#include "UI.hpp"
#include "BootSplash.hpp"
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_event.h>
#include <esp_event_base.h>

#include "../system/SystemEvents.hpp"

#include "../system/Application.hpp"
#include "representation/Point2D.hpp"

#include "widgets/CanvasWidget.hpp"
#include <functional>
bool touched=false;
int16_t touchX=0;
int16_t touchY=0;
double touchDragAngle=0;
double touchDragDistance=0;
int16_t touchDragVectorX=0;
int16_t touchDragVectorY=0;
uint32_t FPS=0;
//bool UIAlertLedEnabled = false;
int16_t downTouchX=120;
int16_t downTouchY=120;
TFT_eSprite *overlay = nullptr;

SemaphoreHandle_t UISemaphore = NULL;

const unsigned long UITimeout = 20*1000;

unsigned long UINextTimeout = 0;
bool UIRunning = true;

/* Event source task related definitions */
ESP_EVENT_DEFINE_BASE(UI_EVENTS);

// Event loops
esp_event_loop_handle_t uiEventloopHandle;



void ScreenWake() {
    if ( false == ttgo->bl->isOn() ) {
        //ttgo->rtc->syncToSystem();
        ttgo->displayWakeup();
        UINextTimeout = millis()+UITimeout;
        ttgo->touchWakup();
        esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_CONTINUE,nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        delay(1); // get time to queue digest ;)
        SystemEventBootEnd(); // perform a ready (and if all is ok, launch watchface)
        ttgo->bl->on();
        if ( ttgo->power->isVBUSPlug() ) {
            ttgo->setBrightness(255);
        } //else { ttgo->setBrightness(30); }

        FPS = MAXFPS;
        UINextTimeout = millis()+UITimeout;
    }
}

void ScreenSleep() {
    if ( true == ttgo->bl->isOn() ) {
        ttgo->bl->off();
        Serial.println("UI: Put screen to sleep now");
        ttgo->displaySleep();
        delay(50);
        ttgo->touchToSleep();
        Serial.flush();
        esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_STOP,nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);

        

        //setCpuFrequencyMhz(80);
        //ttgo->rtc->syncToSystem();
        //delay(50);
        //@TODO send hint for suspend main CPU via system user-defined event loop like LunokIoTEventloopTask
    }
}


static void UIAnchor2DChange(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // @TODO recovered object contains uninitialized data ¿maybe a copy constructor?
    //return;
    Point2D *anchor = reinterpret_cast<Point2D *>(event_data);
    //Point2D *anchor = (Point2D *)event_data;
    int16_t deltaX = 0;
    int16_t deltaY = 0;
    anchor->GetDelta(deltaX,deltaY);
    Serial.printf("UI: Anchor(%p) Changed X: %d Y: %d\n",anchor, deltaX, deltaY);
}

static void UIEventScreenRefresh(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // touch data handling
    static bool oldTouchState = false;

    int16_t newTouchX,newTouchY;
    bool newTouch = ttgo->getTouch(newTouchX,newTouchY);
    if ( ( !oldTouchState ) && ( newTouch ) ) { // thumb in
        touchDragAngle = 0;
        touchDragDistance = 0;
        touchDragVectorX=0;
        touchDragVectorY=0;

        downTouchX = newTouchX;
        downTouchY = newTouchY;
    } else if ( ( oldTouchState ) && ( newTouch ) ) { // meanwhile have thumb...
        // obtain vector
        touchDragVectorX = (newTouchX-downTouchX);
        touchDragVectorY = (newTouchY-downTouchY);
        // obtain distance between two points
        touchDragDistance = sqrt(pow(newTouchX - downTouchX, 2) + pow(newTouchY - downTouchY, 2) * 1.0);
        // Obtain the angle from vector in degrees
        double radAngle = atan2(touchDragVectorY, touchDragVectorX);
        touchDragAngle = radAngle * (180/3.14);
    } else {
        /*
        touchDragAngle = 0;
        touchDragDistance = 0;
        touchDragVectorX=0;
        touchDragVectorY=0;
        */
        //downTouchX=0;
        //downTouchY=0;
    }
    oldTouchState = touched;
    // make public the touch current data
    touched = newTouch;
    touchX = newTouchX;
    touchY = newTouchY;
    // an app can choice destroy the overlay if want, the system generates new one
    if ( nullptr == overlay ) {
        overlay = new TFT_eSprite(ttgo->tft);
        overlay->setColorDepth(16);
        overlay->createSprite(TFT_WIDTH, TFT_HEIGHT);
        overlay->fillSprite(CanvasWidget::MASK_COLOR);
        Serial.printf("UI: new overlay generated %p\n", overlay);
    }
    // try to get UI lock
    bool changes = false;
    if( xSemaphoreTake( UISemaphore, LUNOKIOT_UI_SHORT_WAIT) == pdTRUE )  {
        if ( nullptr != currentApplication ) {
            TFT_eSprite *appView = currentApplication->GetCanvas();
            if ( nullptr != appView ) {
                // perform the call to the app logic
                changes = currentApplication->Tick();
                if ( changes ) { // Tick() returned true
                    appView->setPivot((TFT_WIDTH/2),(TFT_HEIGHT/2));
                    if ( nullptr != overlay )  {
                        overlay->setPivot((TFT_WIDTH/2),(TFT_HEIGHT/2));
                        overlay->pushRotated(appView,0,CanvasWidget::MASK_COLOR);
                    }
                    appView->pushSprite(0,0);
                }
            }
        }
        xSemaphoreGive( UISemaphore );
    }
}

bool lastAlarmLedStatus = false;
uint32_t alarmLedColor = TFT_RED;

void AlertLedEnable(bool enabled, uint32_t ledColor, uint32_t x, uint32_t y) {
    
    alarmLedColor = ledColor;
    if ( false == enabled ) { alarmLedColor = overlay->color24to16(0x2c0000); }

    //overlay->fillCircle(120,22,6,alarmLedColor); // on buffer
    ttgo->tft->fillCircle(120,25,6,alarmLedColor); // on screen NOW

    /*
    if ( ( lastAlarmLedStatus ) && ( false == enabled)) {
        overlay->drawCircle(120,20,10,ledColor);
        overlay->drawCircle(120,20,16,ledColor);
    } else {
        overlay->drawCircle(120,20,10,TFT_BLACK);
        overlay->drawCircle(120,20,16,TFT_BLACK);
    }
    */
    lastAlarmLedStatus = enabled;
}

static void UIEventStop(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    UIRunning = false;
}
static void UIEventContinue(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    UIRunning = true;
}

static void UIEventScreenTimeout(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // touch restart the timeout
    if ( touched ) {
        UINextTimeout = millis()+UITimeout;
        return;
    }
    if ( ttgo->power->isVBUSPlug() ) {
        UINextTimeout = millis()+UITimeout;
        return;
    }

    // on timeout?
    if ( UINextTimeout > millis() ) { return; }

    // get a nap!
    if ( ttgo->bl->isOn() ) {
        ScreenSleep();
        DoSleep();
    }
}

static void UIReadyEvent(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    if ( UI_EVENTS != base ) {
        Serial.printf("UI: uiEventReady: received unknown event: %d discarding event\n", base);
        return;
    }
    if ( UI_EVENT_READY != id ) {
        Serial.printf("UI: uiEventReady: received unknown id: %d discarding event\n", id);
        return;
    }
#ifdef LUNOKIOT_DEBUG_UI
    Serial.println("lunokIoT: UI event loop running");
#endif
    ttgo->setBrightness(255);
}

static void UITickTask(void* args) {
    FPS = MAXFPS;
    while(true) {
        delay(1000/FPS);
         if ( false == UIRunning ) { continue; }
        if ( systemSleep ) { continue; }
        if ( false == ttgo->bl->isOn() ) { continue; } // do not send UI tick when screen is off
        esp_err_t what = esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_TICK, nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
        if ( ESP_ERR_TIMEOUT == what ) {
            FPS--;
            if ( FPS < 1 ) { FPS =1; }
            Serial.printf("UI: Tick timeout! (fps: %d)\n",FPS);
        }
    }
    vTaskDelete(NULL); // unreachable code
}



void UIStart() {
    //ttgo->tft->setSwapBytes(true);
    if ( NULL == UISemaphore ) { UISemaphore = xSemaphoreCreateMutex(); }

    // create the UI event loop
    esp_event_loop_args_t uiEventloopConfig = {
        .queue_size = 5,
        .task_name = "uiTask", // task will be created
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = LUNOKIOT_APP_STACK_SIZE,
        .task_core_id = tskNO_AFFINITY
    };
    // Create the event loops
    ESP_ERROR_CHECK(esp_event_loop_create(&uiEventloopConfig, &uiEventloopHandle));

    // Register the handler for task iteration event. Notice that the same handler is used for handling event on different loops.
    // The loop handle is provided as an argument in order for this example to display the loop the handler is being run on.
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_TICK, UIEventScreenTimeout, nullptr, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_READY, UIReadyEvent, nullptr, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_TICK, UIEventScreenRefresh, nullptr, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_ANCHOR2D_CHANGE, UIAnchor2DChange, nullptr, NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_CONTINUE, UIEventContinue, nullptr, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_STOP, UIEventStop, nullptr, NULL));


    // Create the event source task with the same priority as the current task
    xTaskCreate(UITickTask, "UITick", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);

    esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_READY,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
    UINextTimeout = millis()+UITimeout;

}

void _UINotifyPoint2DChange(Point2D *point) { // @TODO react with Point2D
    esp_err_t what = esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_ANCHOR2D_CHANGE, (void*)point, sizeof(point), LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
    if ( ESP_ERR_TIMEOUT == what ) {
        //Serial.println("UI: Poin2D: Change Notification timeout");
    }
}
/*


// Implementing Mid-Point Circle Drawing Algorithm
void midPointCircleDraw(int x_centre, int y_centre, int r) {
    int x = r, y = 0;
     
    // Printing the initial point on the axes
    // after translation
    printf("(%d, %d) ", x + x_centre, y + y_centre);
     
    // When radius is zero only a single
    // point will be printed
    if (r > 0)
    {
        printf("(%d, %d) ", x + x_centre, -y + y_centre);
        printf("(%d, %d) ", y + x_centre, x + y_centre);
        printf("(%d, %d)\n", -y + x_centre, x + y_centre);
    }
     
    // Initialising the value of P
    int P = 1 - r;
    while (x > y)
    {
        y++;
         
        // Mid-point is inside or on the perimeter
        if (P <= 0)
            P = P + 2*y + 1;
             
        // Mid-point is outside the perimeter
        else
        {
            x--;
            P = P + 2*y - 2*x + 1;
        }
         
        // All the perimeter points have already been printed
        if (x < y)
            break;
         
        // Printing the generated point and its reflection
        // in the other octants after translation
        printf("(%d, %d) ", x + x_centre, y + y_centre);
        printf("(%d, %d) ", -x + x_centre, y + y_centre);
        printf("(%d, %d) ", x + x_centre, -y + y_centre);
        printf("(%d, %d)\n", -x + x_centre, -y + y_centre);
         
        // If the generated point is on the line x = y then
        // the perimeter points have already been printed
        if (x != y)
        {
            printf("(%d, %d) ", y + x_centre, x + y_centre);
            printf("(%d, %d) ", -y + x_centre, x + y_centre);
            printf("(%d, %d) ", y + x_centre, -x + y_centre);
            printf("(%d, %d)\n", -y + x_centre, -x + y_centre);
        }
    }
}
*/


// Implementing Mid-Point Circle Drawing Algorithm
void DescribeCircle(int x_centre, int y_centre, int r, DescribeCircleCallback callback, void *payload) {
    int x = r, y = 0;
    int step = 0;
    bool stop = false;
    // Printing the initial point on the axes
    // after translation
    {
        int curx = x + x_centre;
        int cury = y + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
        double radAngle = atan2(fromCenterX, fromCenterY);
        double dangle = radAngle * (180/3.14);
        int angle = int(180-dangle) % 360;
        stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
        if ( false == stop ) { return; }
        step++;
    }
    //printf("(%d, %d) ", x + x_centre, y + y_centre);
    
    // When radius is zero only a single
    // point will be printed
    if (r > 0)
    {
        {
            int curx = x + x_centre;
            int cury = -y + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/3.14);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; }
            step++;
        }
        //printf("(%d, %d) ", x + x_centre, -y + y_centre);
        {
            int curx = y + x_centre;
            int cury = x + y_centre;
            //int fromCenterX = (curx-120);
            //int fromCenterY = (cury-120);
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/3.14);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; }
            step++;
        }
        //printf("(%d, %d) ", y + x_centre, x + y_centre);
        {
            int curx = -y + x_centre;
            int cury = x + y_centre;
            //int fromCenterX = (curx-120);
            //int fromCenterY = (cury-120);
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/3.14);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; }
            step++;
        }
        //printf("(%d, %d)\n", -y + x_centre, x + y_centre);
    }
     
    // Initialising the value of P
    int P = 1 - r;
    while (x > y)
    {
        y++;
         
        // Mid-point is inside or on the perimeter
        if (P <= 0)
            P = P + 2*y + 1;
             
        // Mid-point is outside the perimeter
        else
        {
            x--;
            P = P + 2*y - 2*x + 1;
        }
         
        // All the perimeter points have already been printed
        if (x < y)
            break;
         
        // Printing the generated point and its reflection
        // in the other octants after translation
        {
            int curx = x + x_centre;
            int cury = y + y_centre;
            //int fromCenterX = (curx-120);
            //int fromCenterY = (cury-120);
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/3.14);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; } // end angle serch
            step++;
        }
        //printf("(%d, %d) ", x + x_centre, y + y_centre);
        {
            int curx = -x + x_centre;
            int cury = y + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/3.14);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; } // end angle serch
            step++;
        }
        //printf("(%d, %d) ", -x + x_centre, y + y_centre);
        {
            int curx = x + x_centre;
            int cury = -y + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/3.14);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; } // end angle serch
            step++;
        }
        //printf("(%d, %d) ", x + x_centre, -y + y_centre);
        {
            int curx = -x + x_centre;
            int cury = -y + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/3.14);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; } // end angle serch
            step++;
        }
        //printf("(%d, %d)\n", -x + x_centre, -y + y_centre);
         
        // If the generated point is on the line x = y then
        // the perimeter points have already been printed
        if (x != y)
        {
            {
                int curx = y + x_centre;
                int cury = x + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
                double radAngle = atan2(fromCenterX, fromCenterY);
                double dangle = radAngle * (180/3.14);
                int angle = int(180-dangle) % 360;
                stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
                if ( false == stop ) { return; } // end angle serch
                step++;
            }
            //printf("(%d, %d) ", y + x_centre, x + y_centre);
            {
                int curx = -y + x_centre;
                int cury = x + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);

                double radAngle = atan2(fromCenterX, fromCenterY);
                double dangle = radAngle * (180/3.14);
                int angle = int(180-dangle) % 360;
                stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
                if ( false == stop ) { return; } // end angle serch
                step++;
            }
            //printf("(%d, %d) ", -y + x_centre, x + y_centre);
            {
                int curx = y + x_centre;
                int cury = -x + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
                double radAngle = atan2(fromCenterX, fromCenterY);
                double dangle = radAngle * (180/3.14);
                int angle = int(180-dangle) % 360;
                stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
                if ( false == stop ) { return; } // end angle serch
                step++;
            }
            //printf("(%d, %d) ", y + x_centre, -x + y_centre);
            {
                int curx = -y + x_centre;
                int cury =  -x + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
                double radAngle = atan2(fromCenterX, fromCenterY);
                double dangle = radAngle * (180/3.14);
                int angle = int(180-dangle) % 360;
                stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
                if ( false == stop ) { return; } // end angle serch
                step++;
            }
            //printf("(%d, %d)\n", -y + x_centre, -x + y_centre);
        }
    }
}
