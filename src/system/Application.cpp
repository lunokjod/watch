#include <Arduino.h>
#include <LilyGoWatch.h>

#include "Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"

extern TTGOClass *ttgo;
extern TFT_eSprite *overlay;

LunokIoTApplication::LunokIoTApplication() {
    this->lastApplicationHeapFree = ESP.getFreeHeap();
    this->lastApplicationPSRAMFree = ESP.getFreePsram();
    this->canvas = new TFT_eSprite(ttgo->tft);
    this->canvas->setColorDepth(16);
    this->canvas->createSprite(TFT_WIDTH, TFT_HEIGHT);
    this->canvas->fillSprite(CanvasWidget::MASK_COLOR);
    Serial.printf("LunokIoTApplication: %p created\n", this);
}

LunokIoTApplication::~LunokIoTApplication() {
    Serial.printf("LunokIoTApplication: %p destroyed\n", this);
    if ( nullptr != this->canvas ) {
        canvas->deleteSprite();
        delete this->canvas;
        this->canvas = nullptr;
    }
    if ( this->lastApplicationHeapFree != ESP.getFreeHeap()) { 
        Serial.printf("LunokIoTApplication: WARNING: Heap leak? differs %d byte\n",this->lastApplicationHeapFree-ESP.getFreeHeap());
    }
    if ( this->lastApplicationPSRAMFree != ESP.getFreePsram()) { 
        Serial.printf("LunokIoTApplication: WARNING: PSRAM leak? differs %d byte\n",this->lastApplicationPSRAMFree-ESP.getFreePsram());
    }
}

TFT_eSprite * LunokIoTApplication::GetCanvas() {
    return this->canvas;
}

bool LunokIoTApplication::Tick() { return false; }

LunokIoTApplication *currentApplication = nullptr;

void LaunchApplicationTask(void * data) {
    LunokIoTApplication *instance = (LunokIoTApplication *)data;
    delay(10); // do a little delay to launch it (usefull to allow a bit of 'yeld()' )
    if( xSemaphoreTake( UISemaphore, portMAX_DELAY) == pdTRUE )  {
        if ( nullptr != currentApplication ) {
            Serial.printf("LaunchApplicationTask: %p closing to run: %p\n", currentApplication,instance);
            LunokIoTApplication *ptrOldApp = currentApplication;
            currentApplication = nullptr;
            if ( nullptr != ptrOldApp ) {
                delete ptrOldApp;
            }
            /*  Don't destroy overlay between apps (performance test)
            if ( nullptr != overlay ) {
                overlay->deleteSprite();
                delete overlay;
                overlay = nullptr;
            }*/
            if ( nullptr != overlay ) { // destroy overlay content between apps
                overlay->fillSprite(CanvasWidget::MASK_COLOR);
            }

        }
        if ( nullptr == data ) {
            Serial.println("Application: None");
            currentApplication = nullptr;
            ttgo->tft->fillScreen(TFT_BLACK);
        } else {
            Serial.printf("Application: %p launched!\n", instance);
            FPS=MAXFPS; // reset refresh rate
            currentApplication = instance;
        }
        xSemaphoreGive( UISemaphore );
    } else {
        Serial.println("Application: ERROR: Unable to get application semaphore");
    }
    vTaskDelete(NULL);
}

void LaunchApplication(LunokIoTApplication *instance) {
    if ( instance == currentApplication) {
        Serial.printf("Application: %p Already running\n", currentApplication);
        return;
    }
    xTaskCreate(LaunchApplicationTask, "", LUNOKIOT_TASK_STACK_SIZE,(void*)instance, uxTaskPriorityGet(NULL), nullptr);
}
