#include <Arduino.h>
#include <LilyGoWatch.h>

#include "Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/UI.hpp"
extern TTGOClass *ttgo; // access to ttgo specific libs

extern TFT_eSprite *overlay; // the overlay, usefull to draw out the UI
//TFT_eSprite * LunokIoTApplication::lastStateCanvas = TFT_eSprite::createSprite(TFT_HEIGHT,TFT_WIDTH);

LunokIoTApplication::LunokIoTApplication() {
#ifdef LUNOKIOT_DEBUG
    // trying to reduce leaks
    this->lastApplicationHeapFree = ESP.getFreeHeap();
    this->lastApplicationPSRAMFree = ESP.getFreePsram();
#endif
    this->canvas = new TFT_eSprite(ttgo->tft);
    this->canvas->setColorDepth(16);
    this->canvas->createSprite(TFT_WIDTH, TFT_HEIGHT);
    this->canvas->fillSprite(CanvasWidget::MASK_COLOR);
#ifdef LUNOKIOT_DEBUG_UI
    Serial.printf("LunokIoTApplication: %p created\n", this);
#endif
    overlay->fillSprite(TFT_TRANSPARENT);
}

LunokIoTApplication::~LunokIoTApplication() {

    if ( nullptr != this->canvas ) {
        canvas->deleteSprite();
        delete canvas;
        this->canvas = nullptr;
    }
#ifdef LUNOKIOT_DEBUG_UI
    Serial.printf("LunokIoTApplication: %p deleted\n", this);
#endif
#ifdef LUNOKIOT_DEBUG
    // In multitask environment is complicated to automate this... but works as advice that something went wrong
    if ( this->lastApplicationHeapFree != ESP.getFreeHeap()) { 
        Serial.printf("LunokIoTApplication: WARNING: Heap leak? differs %d byte\n",this->lastApplicationHeapFree-ESP.getFreeHeap());
    }
    if ( this->lastApplicationPSRAMFree != ESP.getFreePsram()) { 
        Serial.printf("LunokIoTApplication: WARNING: PSRAM leak? differs %d byte\n",this->lastApplicationPSRAMFree-ESP.getFreePsram());
    }
#endif
}

TFT_eSprite * LunokIoTApplication::GetCanvas() { return this->canvas; } // almost unused, I use ->canvas by slacker x'D

bool LunokIoTApplication::Tick() { return false; } // true to notify to UI the full screen redraw

LunokIoTApplication *currentApplication = nullptr;

void LaunchApplicationTask(void * data) {
    LunokIoTApplication *instance = (LunokIoTApplication *)data; // get app instance loaded
    delay(10); // do a little delay to launch it (usefull to get a bit of 'yeld()') and launcher call task ends

    if( xSemaphoreTake( UISemaphore, portMAX_DELAY) == pdTRUE )  { // can use LaunchApplication in any thread 

        if ( nullptr != currentApplication ) {
            Serial.printf("LaunchApplicationTask: %p closing to run: %p\n", currentApplication,instance);
            LunokIoTApplication *ptrOldApp = currentApplication;
            currentApplication = nullptr;
            if ( nullptr != ptrOldApp ) { // @TODO if this delete is optional, can get some grade of "multi-app"
                delete ptrOldApp;
            }
        }

        if ( nullptr == data ) { // this situation is indeed as prior to screen sleep
            Serial.println("Application: None");
            currentApplication = nullptr;     // no one driving now x'D
            ttgo->tft->fillScreen(TFT_BLACK); // at this point, only system is working, the UI is dead in a "null application"
        } else {
            Serial.printf("Application: %p goes to front\n", instance);
            FPS=MAXFPS; // reset refresh rate
            currentApplication = instance;
        }
        xSemaphoreGive( UISemaphore );
    } else {
        Serial.println("Application: ERROR: Unable to get application semaphore, stop launch");
    }
    vTaskDelete(NULL); // get out of my cicken!!!
}

void LaunchApplication(LunokIoTApplication *instance) {
    if ( instance == currentApplication) { // rare but possible (avoid: app is destroyed and pointer to invalid memory)
        Serial.printf("Application: %p Already running, stop launch\n", currentApplication);
        return;
    }
    // launch a task guarantee free the PC (program counter CPU register) of caller object, and made possible a object in "this" context to destroy itself :)
    xTaskCreate(LaunchApplicationTask, "", LUNOKIOT_TASK_STACK_SIZE,(void*)instance, uxTaskPriorityGet(NULL), nullptr);
}
