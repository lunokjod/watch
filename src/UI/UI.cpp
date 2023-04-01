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


#include <LilyGoWatch.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_event.h>
#include <esp_event_base.h>
#include <functional>

#include "lunokiot_config.hpp"
#include "UI.hpp"
#include "BootSplash.hpp"
#include "../system/SystemEvents.hpp"
#include "../system/Application.hpp"
//#include "representation/Point2D.hpp"
#include "widgets/CanvasWidget.hpp"
#include "../app/LogView.hpp"

#ifdef LILYGO_WATCH_2020_V3
#include <driver/i2s.h>
#include "AudioFileSourcePROGMEM.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#endif

#include "../app/TaskSwitcher.hpp"

#include "../system/Application.hpp"
#include "../lunokIoT.hpp"

extern SoftwareKeyboard *keyboardInstance;

extern TTGOClass *ttgo; // ttgo library shit ;)
extern TFT_eSPI * tft;
bool UIlongTap=false; // used to trigger only once the long tap event
bool UILongTapOverride=false; // disable long tap to task switcher feature
extern const PROGMEM uint8_t screenshoot_sound_start[] asm("_binary_asset_screenshoot_sound_mp3_start");
extern const PROGMEM uint8_t screenshoot_sound_end[] asm("_binary_asset_screenshoot_sound_mp3_end");

// default theme and palette
const TemplateColorPalette * currentColorPalette = &DefaultThemeColorPalette;
const TemplateThemeScheme * currentThemeScheme = &DefaultThemeScheme;

bool touched=false; // is screen touch?
int16_t touchX=0;   // thumb coords
int16_t touchY=0;
double touchDragAngle=0;    // angle of drag
double touchDragDistance=0; // distance
int16_t touchDragVectorX=0; // XY vectors
int16_t touchDragVectorY=0;
unsigned long touchDownTimeMS=0; // time push

bool directDraw =false; // don't refresh screen (application-driven)
uint32_t FPS=0;         // current FPS

//bool UIAlertLedEnabled = false;
int16_t downTouchX=120;
int16_t downTouchY=120;

//size_t screenShootCurrentImageX=0;
//size_t screenShootCurrentImageY=0;


//std::list<TFT_eSprite *>ScreenShots;

uint8_t pendingNotifications = 0;

SemaphoreHandle_t UISemaphore = xSemaphoreCreateMutex();

const unsigned long UITimeout = 20*1000;

unsigned long UINextTimeout = 0;
bool UIRunning = true;

/* Event source task related definitions */
ESP_EVENT_DEFINE_BASE(UI_EVENTS);

// Event loops
esp_event_loop_handle_t uiEventloopHandle;
TFT_eSprite *screenShootCanvas = nullptr;
bool screenShootInProgress = false; // @TODO watchface must show it


SemaphoreHandle_t ScreenSemaphore = xSemaphoreCreateMutex();

void ScreenWake() {
    bool done = xSemaphoreTake(ScreenSemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
    if (false == done) {
        lEvLog("Unable to obtain the Screen Lock!\n");
        return;
    }
    if ( false == ttgo->bl->isOn() ) {
        LoT().CpuSpeed(240);
        if ( ttgo->rtc->isValid() ) { ttgo->rtc->syncToSystem(); }
        ttgo->displayWakeup();
        UINextTimeout = millis()+UITimeout;
        #if defined(LILYGO_WATCH_2020_V2) || defined(LILYGO_WATCH_2020_V3)
        ttgo->touchWakup();
        #endif
        //esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_CONTINUE,nullptr, 0, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
        //delay(1); // get time to queue digest ;)
        //SystemEventBootEnd(); // perform a ready (and if all is ok, launch watchface)
        ttgo->bl->on();
        ttgo->tft->fillScreen(TFT_BLACK); // cleanup, better than show old watchface time! (missinformation)
        FPS = MAXFPS;
    }
    xSemaphoreGive(ScreenSemaphore);
}

void ScreenSleep() {
    bool done = xSemaphoreTake(ScreenSemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
    if (false == done) {
        lEvLog("Unable to obtain the Screen Lock!\n");
        return;
    }
    if ( true == ttgo->bl->isOn() ) {
        ttgo->bl->off();
        lUILog("Put screen to sleep now\n");
        ttgo->displaySleep();
        delay(1);
        ttgo->touchToSleep();
        Serial.flush();
        //esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_STOP,nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        LoT().CpuSpeed(80);
    }
    xSemaphoreGive(ScreenSemaphore);
}

/*
static void UIAnchor2DChange(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // @TODO recovered object contains uninitialized data ¿maybe a copy constructor?
    //return;
    Point2D *anchor = reinterpret_cast<Point2D *>(event_data);
    //Point2D *anchor = (Point2D *)event_data;
    int16_t deltaX = 0;
    int16_t deltaY = 0;
    anchor->GetDelta(deltaX,deltaY);
    lUILog("UI: Anchor(%p) Changed X: %d Y: %d\n",anchor, deltaX, deltaY);
}*/

void TakeScreenShootSound() {
#ifdef LUNOKIOT_SILENT_BOOT
    lUILog("Audio: Not initialized due Silent boot is enabled\n");
    delay(150);
    return;
#endif
#ifdef LILYGO_WATCH_2020_V3
    // Audio fanfare x'D
    lUILog("Audio: Initialize\n");
    ttgo->enableAudio();

    // from https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/examples/UnitTest/HardwareTest/HardwareTest.ino
    AudioGeneratorMP3 *mp3;
    AudioFileSourcePROGMEM *file;
    AudioOutputI2S *out;
    AudioFileSourceID3 *id3;
    // file = new AudioFileSourcePROGMEM(image, sizeof(image));
    file = new AudioFileSourcePROGMEM(screenshoot_sound_start, (uint32_t)(screenshoot_sound_end-screenshoot_sound_start));
    id3 = new AudioFileSourceID3(file);
    out = new AudioOutputI2S();
    out->SetPinout(TWATCH_DAC_IIS_BCK, TWATCH_DAC_IIS_WS, TWATCH_DAC_IIS_DOUT);
    lUILog("Audio: MP3 Screenshoot\n");
    mp3 = new AudioGeneratorMP3();
    mp3->begin(id3, out);
    while (true) {
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();
            }
        } else {
            lUILog("Audio: MP3 done\n");
            break;
        }
    }
    delete file;
    delete id3;
    delete out;
    delete mp3;

    ttgo->disableAudio();

    i2s_driver_uninstall(I2S_NUM_0);
    delay(2);
    ttgo->shake();
#endif

}

TFT_eSprite * ShearSprite(TFT_eSprite *view, TransformationMatrix transform) {
    // https://www.mathsisfun.com/algebra/matrix-transform.html
    TFT_eSprite * canvas = new TFT_eSprite(ttgo->tft); // build new sprite
    if ( nullptr == canvas ) { return nullptr; }
    canvas->setColorDepth(view->getColorDepth());
    if ( nullptr == canvas->createSprite(view->width(), view->height()) ) {
        delete canvas;
        return nullptr;
    }
    for(int y=0;y<view->height();y++) {
        for(int x=0;x<view->width();x++) {
            uint16_t color = view->readPixel(x,y);
            int nx=transform.a*x+transform.b*y;
            int ny=transform.c*x+transform.d*y;
            canvas->drawPixel(nx,ny,color);
        }
    }
    return canvas;
    /*
    for (let i = 0; i < shape.pts.length; i++) {
        let pt = shape.pts[i]
        let x = a * pt[0] + b * pt[1]
        let y = c * pt[0] + d * pt[1]
        newPts.push({ x: x, y: y })
    }*/
}
// reduce the size of image using float value (0.5=50%)
TFT_eSprite * ScaleSprite(TFT_eSprite *view, float divisor) {
    if ( nullptr == view ) { return nullptr; }
    //if ( 1.0 == divisor ) { return DuplicateSprite(view); } // is the same!!! @TODO Duplicate invert colors :( invert colors
    if ( divisor < 0.0 ) { divisor=0.05; } // dont allow 0 scale
    int16_t nh = view->height()*divisor; // calculate new size
    int16_t nw = view->width()*divisor;

    TFT_eSprite * canvas = new TFT_eSprite(ttgo->tft); // build new sprite
    if ( nullptr == canvas ) { return nullptr; }
    canvas->setColorDepth(view->getColorDepth());
    if ( nullptr == canvas->createSprite(nw, nh) ) {
        delete canvas;
        return nullptr;
    }
    //canvas->fillSprite(TFT_RED);//CanvasWidget::MASK_COLOR);

    //Serial.printf("ScaleSprite: H: %d W: %d (divisor: %f) Calculated H: %d W: %d\n",
    //     view->height(),view->width(), divisor, canvas->height(),canvas->width());

    // iterate whole paint searching for pixels
    for(float y=0;y<nh;y++) {
        int32_t ry = int32_t(y/divisor);
        for(float x=0;x<nw;x++) {
            int32_t rx = int32_t(x/divisor);

            uint16_t originalColor = view->readPixel(rx,ry);
            canvas->drawPixel(x,y,originalColor);
            //Serial.printf("ScaleSprite: X: %f Y: %f cX: %d cY: %d COLOR: 0x%04x\n",x,y,rx,ry,originalColor);
        }
    }
    return canvas;
}

uint16_t ColorSwap(uint16_t colorToSwap) {
    // twist color from RGB to BRG for draw on canvas under certain circumstances
    double r = ((colorToSwap >> 11) & 0x1F) / 31.0; // red   0.0 .. 1.0
    double g = ((colorToSwap >> 5) & 0x3F) / 63.0;  // green 0.0 .. 1.0
    double b = (colorToSwap & 0x1F) / 31.0;         // blue  0.0 .. 1.0
    uint8_t rC=255*r;
    uint8_t gC=255*g;
    uint8_t bC=255*b;
    return ttgo->tft->color565(rC,gC,bC);
}

uint16_t ByteSwap(uint16_t colorToSwap) {
    uint8_t wf = (colorToSwap % 256);
    uint8_t wb = ((colorToSwap / 256) % 256);
    uint16_t fcolor = (wf*256)+wb;
    return fcolor;
}

TFT_eSprite * ScaleSpriteMAX(TFT_eSprite *view, float divisor, int16_t maxW, int16_t maxH) {
    int16_t nh = view->height()*divisor; // calculate new size
    int16_t nw = view->width()*divisor;

    TFT_eSprite * canvas = new TFT_eSprite(ttgo->tft); // build new sprite
    canvas->setColorDepth(view->getColorDepth());
    canvas->createSprite(nw, nh);
    canvas->fillSprite(TFT_RED);//CanvasWidget::MASK_COLOR);

    //Serial.printf("ScaleSprite: H: %d W: %d (divisor: %f) Calculated H: %d W: %d\n",
    //     view->height(),view->width(), divisor, canvas->height(),canvas->width());

    for(float y=0;y<nh;y++) {
        if ( y > maxH ) { break; }
        int32_t ry = int32_t(y/divisor);
        for(float x=0;x<nw;x++) {
            if ( x > maxW ) { break; }
            int32_t rx = int32_t(x/divisor);

            uint16_t originalColor = view->readPixel(rx,ry);
            canvas->drawPixel(x,y,originalColor);
            //Serial.printf("ScaleSprite: X: %f Y: %f cX: %d cY: %d COLOR: 0x%04x\n",x,y,rx,ry,originalColor);
        }
    }
    return canvas;
}
/*
TFT_eSprite *TakeScreenShoot() {
    TFT_eSprite *appView = currentApplication->canvas;
    if ( nullptr == appView ) { return nullptr; }
    TFT_eSprite *myCopy = DuplicateSprite(appView);
    ttgo->setBrightness(0);
    ttgo->tft->fillScreen(TFT_WHITE);
    ttgo->setBrightness(255);
    ttgo->tft->fillScreen(TFT_BLACK);
    ttgo->setBrightness(0);
    delay(10);
    ttgo->tft->fillScreen(TFT_WHITE);
    ttgo->setBrightness(255);
    // show thumbnail centered on screen
    TFT_eSprite *scaledImg = ScaleSprite(appView,0.8);
    //TFT_eSprite *imgCopy = DuplicateSprite(scaledImg);
    //scaledImg->pushSprite((TFT_WIDTH/2)-(imgCopy->width()/2),(TFT_HEIGHT/2)-(imgCopy->width()/2));
    scaledImg->pushSprite((TFT_WIDTH/2)-(scaledImg->width()/2),(TFT_HEIGHT/2)-(scaledImg->width()/2));
    scaledImg->deleteSprite();
    delete scaledImg;
    //imgCopy->deleteSprite();
    //delete imgCopy;
    delay(200);
    TakeScreenShootSound();
#ifdef LILYGO_WATCH_2020_V3
    ttgo->shake();
#endif
    return myCopy;
}
*/
TFT_eSprite * DuplicateSprite(TFT_eSprite *view) {
    screenShootCanvas = new TFT_eSprite(ttgo->tft);
    if ( nullptr == screenShootCanvas ) { return nullptr; }
    screenShootCanvas->setColorDepth(view->getColorDepth());
    screenShootCanvas->createSprite(view->width(),view->height());
    screenShootCanvas->fillSprite(CanvasWidget::MASK_COLOR);
    view->pushRotated(screenShootCanvas,0,CanvasWidget::MASK_COLOR);
    return screenShootCanvas;
}

extern SemaphoreHandle_t I2cMutex;

SemaphoreHandle_t UIDrawProcess = xSemaphoreCreateMutex();

/*
 * Update touch and screen 
 */
static void UIEventScreenRefresh(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // touch data handling
    static bool oldTouchState = false;

    int16_t newTouchX,newTouchY;
    bool newTouch=false;
    if( xSemaphoreTake( I2cMutex, LUNOKIOT_UI_SHORT_WAIT) == pdTRUE )  {
        newTouch = ttgo->getTouch(newTouchX,newTouchY);
        xSemaphoreGive( I2cMutex );
    } else { return; } // don't mess me 
    bool updateCoords=true;
    if ( ( !oldTouchState ) && ( newTouch ) ) { // thumb in
        touchDragAngle = 0;
        touchDragDistance = 0;
        touchDragVectorX=0;
        touchDragVectorY=0;
        touchDownTimeMS = millis();

        downTouchX = newTouchX;
        downTouchY = newTouchY;
    } else if ( ( oldTouchState ) && ( newTouch ) ) { // meanwhile have thumb...
        // obtain vector
        touchDragVectorX = (newTouchX-downTouchX);
        touchDragVectorY = (newTouchY-downTouchY);
        // obtain distance between two points
        touchDragDistance = sqrt(pow(touchDragVectorX, 2) + pow(touchDragVectorY, 2) * 1.0);
        // Obtain the angle from vector in degrees
        double radAngle = atan2(touchDragVectorY, touchDragVectorX);
        touchDragAngle = radAngle * (180/3.14);
    } else {
        touchDownTimeMS=0;
        updateCoords=false;
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
    if ( updateCoords ) {
        touchX = newTouchX;
        touchY = newTouchY;
    }

    // try to get UI lock
    bool changes = false;
    if( xSemaphoreTake( UISemaphore, LUNOKIOT_UI_SHORT_WAIT) == pdTRUE )  {
        if ( nullptr == currentApplication ) {
            // no app, nothing to do
            xSemaphoreGive( UISemaphore );
            xSemaphoreGive( UIDrawProcess );
            return;
        }
        // get the current app view
        TFT_eSprite *appView = currentApplication->canvas;
        if ( nullptr == appView ) {
            lUILog("Unable to get app canvas!\n");
            xSemaphoreGive( UISemaphore );
            xSemaphoreGive( UIDrawProcess );
            return;
        } // do nothing
        
        // verify long tap to launch TaskSwitcher
        if ( 0 == touchDownTimeMS ) { // other way to detect touch :P
            UIlongTap=false;
        } else { 
            if  ( 0.0 == touchDragDistance ) {
                // touch full-system reactors, long tap to show the task list
                unsigned long pushTime = millis() - touchDownTimeMS; // how much time?
                if (( false == UILongTapOverride ) && ( pushTime > LUNOKIOT_TOUCH_LONG_MS )&&( false == UIlongTap )) {
                    UIlongTap=true; // locked until thumb up
                    lEvLog("UI: Touchpad long tap\n");
                    /*
                    // simply decorative, notifiyng the user about the action
                    ttgo->tft->fillCircle(touchX,touchY,70,TFT_WHITE);
                    //ttgo->tft->fillCircle(touchX,touchY,50,TFT_WHITE);
                    ttgo->tft->drawCircle(touchX,touchY,80,TFT_WHITE);
                    */
                    // announce haptic on launch the long tap task
                    #ifdef LILYGO_WATCH_2020_V3
                        //ttgo->motor->onec(40);
                        //delay(100);
                        ttgo->motor->onec(80);
                        //delay(200);
                    #endif

                    //@TODO here is a good place to draw a circular menu and use the finger as arrow
                    // Up tasks
                    // Down appointments
                    // Left email
                    // Right notifications...
                    // and more...

                    xSemaphoreGive( UISemaphore );
                    
                    if ( ( false == UILongTapOverride ) && ( nullptr != currentApplication ) && ( 0 == strcmp(currentApplication->AppName(),"Task switcher" ) ) ) {
                        LaunchWatchface(); // return to user configured watchface
                    } else { LaunchApplication(new TaskSwitcher()); }
                    xSemaphoreGive( UIDrawProcess );
                    return;
                }
            }
        }
        // if screen-keyboard is enabled, send the Tick like normal application
        if ( nullptr != keyboardInstance ) {
            changes = keyboardInstance->Tick();
        } else {
            if ( false == UIlongTap ) { // only if no long tap in progress
                // perform the call to the app logic
                changes = currentApplication->Tick();
            }
        }
        if ( directDraw ) { changes=false; } // directDraw overrides application normal redraw

        if ( changes ) {
            if ( nullptr != keyboardInstance ) {
                keyboardInstance->canvas->pushSprite(0,0);
            } else {
                appView->pushSprite(0,0); // push appView to tft
            }
        }
        xSemaphoreGive( UISemaphore );
    }
    xSemaphoreGive( UIDrawProcess );
}

static void UIEventStop(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    lUILog("UIEventStop\n");
    UIRunning = false;
}
static void UIEventContinue(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    lUILog("UIEventContinue\n");
    UIRunning = true;
}

static void UIEventScreenTimeout(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // touch restart the timeout
    if ( touched ) {
        UINextTimeout = millis()+UITimeout;
        return;
    }
    // if vbus get energy don't sleep
    if ( vbusPresent ) {
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
    lUILog("lunokIoT: UI event loop running\n");
}

TaskHandle_t UITickTaskHandler = NULL;              // save the handler
static void UITickTask(void* args) {
    FPS = MAXFPS;
    TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
    while(true) {
        const TickType_t LUNOKIOT_UI_TICK_TIME = (FPS / portTICK_PERIOD_MS); // try to get the promised speed
        BaseType_t isDelayed = xTaskDelayUntil( &nextCheck, LUNOKIOT_UI_TICK_TIME ); // wait a ittle bit

        if ( false == UIRunning ) { continue; }
        if ( systemSleep ) { continue; }
        if ( false == ttgo->bl->isOn() ) { continue; } // do not send UI tick when screen is off

        esp_err_t what = esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_TICK, nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
        // Dynamic FPS... the apps receives less events per second
        if (( ESP_OK != what )||( isDelayed )) {
            FPS--;
            if ( FPS < 1 ) { FPS=1; }
            //if ( false == directDraw ) { lUILog("UI: Tick delay! (fps: %d)\n",FPS); }
        }
    }
    lEvLog("UITickTask: is dead!!!\n");
    vTaskDelete(NULL);
}

StaticTask_t UITickBuffer;
StackType_t UITickStack[LUNOKIOT_TINY_STACK_SIZE];
void UITickStart() { // @TODO THIS CAN BE REPLACED BY A Ticker.attach_ms
    if ( NULL == UITickTaskHandler ) {
        UITickTaskHandler = xTaskCreateStaticPinnedToCore( UITickTask,
                                                "lUITick",
                                                LUNOKIOT_TINY_STACK_SIZE,
                                                nullptr,
                                                tskIDLE_PRIORITY,
                                                UITickStack,
                                                &UITickBuffer,
                                                1);

        //BaseType_t res = xTaskCreatePinnedToCore(UITickTask, "lUITick", LUNOKIOT_APP_STACK_SIZE, NULL,uxTaskPriorityGet(NULL), &UITickTaskHandler,1);
        lUILog("Tick enabled\n");
    }
}

void UITickEnd() { // @TODO this can be replaced by a Ticker stop
    if ( NULL != UITickTaskHandler ) {
        vTaskDelete(UITickTaskHandler);
        UITickTaskHandler=NULL;
        lUILog("Tick disabled (app loses Tick now)\n");
    }
}

static void UITickStartCallback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    UITickStart();
}

static void UITickStopCallback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    UITickEnd();
}

/*
 * UIStart begin the UI loop, to handle UI events
 */
void UIStart() {

    // create the UI event loop
    esp_event_loop_args_t uiEventloopConfig = {
        .queue_size = 10,   // so much, but with multitask delays... maybe this is the most easy
        .task_name = "uiTask", // task will be created
        .task_priority = tskIDLE_PRIORITY+4, // priorize UI over rest of system (responsive UI)
        .task_stack_size = LUNOKIOT_APP_STACK_SIZE, // normal arduinofw setup task stack
        .task_core_id = 1, //tskNO_AFFINITY // PRO_CPU // APP_CPU
    };

    UINextTimeout = millis()+UITimeout; // poweroff the screen when no activity timeout

    // Create the event loops
    esp_err_t uiLoopRc = esp_event_loop_create(&uiEventloopConfig, &uiEventloopHandle);
    if ( ESP_OK !=  uiLoopRc ) {
        lUILog("ERROR: Unable to create event loop: '%s' ABORTING UI!\n",esp_err_to_name(uiLoopRc));
        return;
    }
    // Register the handler for task iteration event. Notice that the same handler is used for handling event on different loops.
    // The loop handle is provided as an argument in order for this example to display the loop the handler is being run on.
    uiLoopRc = esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_TICK, UIEventScreenTimeout, nullptr, NULL);
    uiLoopRc = esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_READY, UIReadyEvent, nullptr, NULL);
    uiLoopRc = esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_TICK, UIEventScreenRefresh, nullptr, NULL);
    //    esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_ANCHOR2D_CHANGE, UIAnchor2DChange, nullptr, NULL);

    // Useless if is sended later than whake x'D
    //    esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_CONTINUE, UIEventContinue, nullptr, NULL);
    //    esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_STOP, UIEventStop, nullptr, NULL);
    

    uiLoopRc = esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_READY, UITickStartCallback, nullptr, NULL);

    // disable ticks
    uiLoopRc = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_LIGHTSLEEP, UITickStopCallback, nullptr, NULL);
    // enable ticks
    uiLoopRc = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_WAKE, UITickStartCallback, nullptr, NULL);
    //esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_READY, UITickStartCallback, nullptr, NULL);

    uiLoopRc = esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_READY,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);

}
/*
void _UINotifyPoint2DChange(Point2D *point) { // @TODO react with Point2D
    esp_err_t what = esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_ANCHOR2D_CHANGE, (void*)point, sizeof(point), LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
    if ( ESP_ERR_TIMEOUT == what ) {
        //Serial.println("UI: Poin2D: Change Notification timeout");
    }
}
*/
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
    if (r > 0) {
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

/*

*/