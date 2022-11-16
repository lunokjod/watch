#include <Arduino.h>
#include <ArduinoNvs.h>

#include <libraries/TFT_eSPI/TFT_eSPI.h>
extern TFT_eSPI *tft;

#include "Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/UI.hpp"
#include "../app/LogView.hpp"

extern TTGOClass *ttgo; // access to ttgo specific libs

extern TFT_eSprite *overlay; // the overlay, usefull to draw out the UI

LunokIoTApplication *currentApplication = nullptr; // ptr to get current foreground application

// get a capture of current application
TFT_eSprite *LunokIoTApplication::NewScreenShoot() { return DuplicateSprite(canvas); }
LunokIoTApplication::LunokIoTApplication() {
#ifdef LUNOKIOT_DEBUG
    // trying to reduce leaks
    lastApplicationHeapFree = ESP.getFreeHeap();
    lastApplicationPSRAMFree = ESP.getFreePsram();
#endif
    canvas = new TFT_eSprite(tft);
    canvas->setColorDepth(16);
    canvas->createSprite(TFT_WIDTH, TFT_HEIGHT);
    canvas->fillSprite(ThCol(background));
    lUILog("LunokIoTApplication: %p created\n", this);
    overlay->fillSprite(TFT_TRANSPARENT);
}

LunokIoTApplication::~LunokIoTApplication() {

    if ( nullptr != this->canvas ) {
        canvas->deleteSprite();
        delete canvas;
        this->canvas = nullptr;
    }
    lUILog("LunokIoTApplication: %p deleted\n", this);
#ifdef LUNOKIOT_DEBUG
    // In multitask environment is complicated to automate this... but works as advice that something went wrong
    if ( this->lastApplicationHeapFree != ESP.getFreeHeap()) { 
        lUILog("LunokIoTApplication: WARNING: Heap leak? differs %d byte\n",this->lastApplicationHeapFree-ESP.getFreeHeap());
    }
    if ( this->lastApplicationPSRAMFree != ESP.getFreePsram()) { 
        lUILog("LunokIoTApplication: WARNING: PSRAM leak? differs %d byte\n",this->lastApplicationPSRAMFree-ESP.getFreePsram());
    }
#endif
}

bool LunokIoTApplication::Tick() { return false; } // true to notify to UI the full screen redraw

class LaunchApplicationDescriptor {
    public:
        LunokIoTApplication *instance;
        bool animation;
};

void KillApplicationTask(void * data) {
    lUILog("KillApplicationTask: %p closing...\n", data);
    LunokIoTApplication *instance = (LunokIoTApplication *)data;
    delete instance;
    lUILog("KillApplicationTask: %p has gone\n", data);
    vTaskDelete(NULL);
}

void LaunchApplicationTask(void * data) {
    LaunchApplicationDescriptor * dataDesc =  (LaunchApplicationDescriptor *)data;

    LunokIoTApplication *instance = dataDesc->instance; // get app instance loaded
    bool animation = dataDesc->animation;

    //delay(10); // do a little delay to launch it (usefull to get a bit of 'yeld()') and launcher call task ends

    if( xSemaphoreTake( UISemaphore, portMAX_DELAY) == pdTRUE )  { // can use LaunchApplication in any thread 
        LunokIoTApplication * ptrToCurrent = currentApplication;

        if ( nullptr == instance ) { // this situation is indeed as prior to screen sleep
            lUILog("Application: None\n");
            currentApplication = nullptr;     // no one driving now x'D
            ttgo->tft->fillScreen(TFT_BLACK); // at this point, only system is working, the UI is dead in a "null application"
        } else {
           lUILog("Application: %p goes to front\n", instance);
            FPS=MAXFPS; // reset refresh rate
            currentApplication = instance; // set as main app
            uint8_t userBright = NVS.getInt("lBright");
            if ( userBright != 0 ) { ttgo->setBrightness(userBright); } // reset the user brightness
            TFT_eSprite *appView = instance->canvas;
            if ( animation ) { // Launch new app effect (zoom out)
                for(float scale=0.1;scale<0.4;scale+=0.04) {
                    TFT_eSprite *scaledImg = ScaleSprite(appView,scale);
                    //lEvLog("Application: Splash scale: %f pxsize: %d\n",scale,scaledImg->width());
                    scaledImg->pushSprite((TFT_WIDTH-scaledImg->width())/2,(TFT_HEIGHT-scaledImg->width())/2);
                    scaledImg->deleteSprite();
                    delete scaledImg;
                }
                for(float scale=0.4;scale<0.7;scale+=0.15) {
                    TFT_eSprite *scaledImg = ScaleSprite(appView,scale);
                    //lEvLog("Application: Splash scale: %f pxsize: %d\n",scale,scaledImg->width());
                    scaledImg->pushSprite((TFT_WIDTH-scaledImg->width())/2,(TFT_HEIGHT-scaledImg->width())/2);
                    scaledImg->deleteSprite();
                    delete scaledImg;
                }
            }
            // push full image
            appView->pushSprite(0,0);
        }
        xSemaphoreGive( UISemaphore ); // free

        if ( nullptr != ptrToCurrent ) {
            // kill app in other thread (some apps takes much time)
            xTaskCreate(KillApplicationTask, "", LUNOKIOT_TASK_STACK_SIZE,(void*)ptrToCurrent, uxTaskPriorityGet(NULL), nullptr);
        }
    }
    vTaskDelete(NULL); // get out of my cicken!!!
}

void LaunchApplication(LunokIoTApplication *instance, bool animation) {
    if ( instance == currentApplication) { // rare but possible (avoid: app is destroyed and pointer to invalid memory)
        lUILog("Application: %p Already running, stop launch\n", currentApplication);
        return;
    }
    LaunchApplicationDescriptor * thisLaunch = new LaunchApplicationDescriptor();
    thisLaunch->instance = instance;
    thisLaunch->animation = animation;
    // launch a task guarantee free the PC (program counter CPU register) of caller object, and made possible a object in "this" context to destroy itself :)
    xTaskCreate(LaunchApplicationTask, "", LUNOKIOT_TASK_STACK_SIZE,(void*)thisLaunch, uxTaskPriorityGet(NULL), nullptr);
}
