#include <Arduino.h>
#include <ArduinoNvs.h>
#include <LilyGoWatch.h>

extern TTGOClass *ttgo; // access to ttgo specific libs
extern TFT_eSPI *tft;

#include "Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/UI.hpp"
#include "../app/LogView.hpp"

#include "../app/Watchface2.hpp"

#include "SystemEvents.hpp"

#include <functional>

SemaphoreHandle_t lAppStack = xSemaphoreCreateMutex();
// elegant https://stackoverflow.com/questions/10722858/how-to-create-an-array-of-classes-types
typedef LunokIoTApplication* WatchfaceMaker();
template <class WFA> LunokIoTApplication* MakeWatch() { return new WFA; }
WatchfaceMaker* Watchfaces[] = { MakeWatch<Watchface2Application> };
LunokIoTApplication *GetWatchFace() { return Watchfaces[0](); } // hardcoded by now
void LaunchWatchface(bool animation) { LaunchApplication(GetWatchFace(),animation); }

extern TFT_eSprite *overlay; // the overlay, usefull to draw out the UI

LunokIoTApplication *currentApplication = nullptr; // ptr to get current foreground application

size_t lastAppsOffset=0;
const char  * lastAppsName[LUNOKIOT_MAX_LAST_APPS] = { nullptr }; 
unsigned long lastAppsTimestamp[LUNOKIOT_MAX_LAST_APPS] = { 0 }; 
TFT_eSprite * lastApps[LUNOKIOT_MAX_LAST_APPS] = { nullptr };

// get a capture of current application
TFT_eSprite *LunokIoTApplication::ScreenCapture() { return DuplicateSprite(canvas); }
LunokIoTApplication::LunokIoTApplication() {
    canvas = new TFT_eSprite(tft);
    if ( nullptr == canvas ) { 
        lUILog("LunokIoTApplication: %p ERROR: unable to create their own canvas!!!\n", this);
        //LaunchWatchface(false);
        return;
    }
    canvas->setColorDepth(16);
    if ( NULL == canvas->createSprite(TFT_WIDTH, TFT_HEIGHT) ) {
        lUILog("LunokIoTApplication: %p ERROR: unable to create canvas sprite\n", this);
        delete canvas;
        canvas=nullptr;
        //LaunchWatchface(false);
        return;
    };
    lUILog("LunokIoTApplication: %p begin\n", this);
}

LunokIoTApplication::~LunokIoTApplication() {

    if ( nullptr != this->canvas ) {
        canvas->deleteSprite();
        delete canvas;
        this->canvas = nullptr;
    }
    lUILog("LunokIoTApplication: %p ends\n", this);
}

bool LunokIoTApplication::Tick() { return false; } // true to notify to UI the full screen redraw

// This task destroy the last app in a second thread trying to maintain the user experience
void KillApplicationTaskSync(LunokIoTApplication *instance) {
    if ( nullptr != instance ) {
        void * where=(void *)instance;
        lUILog("KillApplicationTask: %p '%s' closing...\n", where,instance->AppName());
        delete instance;
        delay(80); // do idle_task time to free heap
        lUILog("KillApplicationTask: %p has gone\n", where);
        //FreeSpace();
    }

}
void KillApplicationTask(void * data) {
    if ( nullptr != data ) {
        LunokIoTApplication *instance = (LunokIoTApplication *)data;
        KillApplicationTaskSync(instance);
    }
    vTaskDelete(NULL);
}
void LaunchApplicationTask(void * data) {
    LaunchApplicationDescriptor * dataDesc =  (LaunchApplicationDescriptor *)data;
    LaunchApplicationTaskSync(dataDesc,false);
    vTaskDelete(NULL); // get out of my cicken!!!
}
void LaunchApplicationTaskSync(LaunchApplicationDescriptor * appDescriptor,bool synched) {
//    LaunchApplicationDescriptor * dataDesc =  (LaunchApplicationDescriptor *)data;

    LunokIoTApplication *instance = appDescriptor->instance; // get app instance loaded
    bool animation = appDescriptor->animation;
    delete appDescriptor;
    delay(20); // do a little delay to launch it (usefull to get a bit of 'yeld()') and launcher call task ends

    if( xSemaphoreTake( UISemaphore, portMAX_DELAY) == pdTRUE )  { // can use LaunchApplication in any thread 
        LunokIoTApplication * ptrToCurrent = currentApplication;

        if ( nullptr == instance ) { // this situation is indeed as prior to screen sleep
            lUILog("Application: None\n");
            currentApplication = nullptr;     // no one driving now x'D
            ttgo->tft->fillScreen(TFT_BLACK); // at this point, only system is working, the UI is dead in a "null application"
        } else {
           lUILog("Application: %p '%s' goes to front\n", instance,instance->AppName());
            FPS=MAXFPS; // reset refresh rate
            currentApplication = instance; // set as main app
            uint8_t userBright = NVS.getInt("lBright");
            if ( userBright != 0 ) { ttgo->setBrightness(userBright); } // reset the user brightness
            TFT_eSprite *appView = instance->canvas;
            if ( nullptr != appView ) {
                if ( animation ) { // Launch new app effect (zoom out)
                    //taskDISABLE_INTERRUPTS();
                    //vTaskSuspendAll();
                    //taskENTER_CRITICAL(NULL);
                    for(float scale=0.1;scale<0.4;scale+=0.04) {
                        TFT_eSprite *scaledImg = ScaleSprite(appView,scale);
                        if ( nullptr != scaledImg ) {
                            //lEvLog("Application: Splash scale: %f pxsize: %d\n",scale,scaledImg->width());
                            scaledImg->pushSprite((TFT_WIDTH-scaledImg->width())/2,(TFT_HEIGHT-scaledImg->height())/2);
                            scaledImg->deleteSprite();
                            delete scaledImg;
                        }
                    }
                    for(float scale=0.4;scale<0.7;scale+=0.15) {
                        TFT_eSprite *scaledImg = ScaleSprite(appView,scale);
                        if ( nullptr != scaledImg ) {
                            //lEvLog("Application: Splash scale: %f pxsize: %d\n",scale,scaledImg->width());
                            scaledImg->pushSprite((TFT_WIDTH-scaledImg->width())/2,(TFT_HEIGHT-scaledImg->height())/2);
                            scaledImg->deleteSprite();
                            delete scaledImg;
                        }
                    }
                    //taskEXIT_CRITICAL(NULL);
                    //xTaskResumeAll();
                    //taskENABLE_INTERRUPTS();
                }
                // push full image
                appView->pushSprite(0,0);
            }
        }
        xSemaphoreGive( UISemaphore ); // free

        // take screenshoot for the task switcher
        if ( nullptr == ptrToCurrent ) { return; }
        if ( ptrToCurrent->mustShowAsTask() ) {
            if( xSemaphoreTake( lAppStack, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS) == pdTRUE )  {
                // SEARCH DUPLICATES
                const char  * name = ptrToCurrent->AppName();
                int searchOffset = LUNOKIOT_MAX_LAST_APPS; // must be signed
                bool alreadyExists=false;
                while ( searchOffset > 0 ) {
                    searchOffset--;
                    if ( 0 == lastAppsTimestamp[searchOffset] ) { continue; } // no last seen (probably empty)
                    if ( nullptr == lastAppsName[searchOffset] ) { continue; } // have a name?
                    if ( 0 != strcmp(name,lastAppsName[searchOffset]) ) { continue; } // is the same app?
                    alreadyExists=true;
                    // destroy last screenshoot (if have any)
                    if ( nullptr != lastApps[searchOffset] ) {
                        lastApps[searchOffset]->deleteSprite();
                        delete lastApps[searchOffset];
                        lastApps[searchOffset]=nullptr;
                        delay(80); // maybe is supersticious, but, bring idle task a little time to perform the free() is better for us :P
                    }
                    // get updated state screenshoot
                    //TFT_eSprite *lastView = ScaleSprite(ptrToCurrent->canvas,0.333);
                    TFT_eSprite * lastView = ptrToCurrent->ScreenCapture();
                    if ( nullptr == lastView ) { // uops! must destroy this entry :(
                        lastAppsName[searchOffset] = nullptr;
                        lastAppsTimestamp[searchOffset]=0;
                        continue;
                    }
                    // update current app state
                    lastApps[searchOffset] = lastView;
                    lUILog("%p '%s' already on the apps stack (offset: %d) view updated\n",ptrToCurrent,name,searchOffset);

                    // check the timeout
                    if ( millis() > (lastAppsTimestamp[searchOffset]+(LUNOKIOT_RECENT_APPS_TIMEOUT_S*1000))) {
                        lUILog("%p '%s' timeout in app stack (offset: %u)\n",ptrToCurrent,name,searchOffset);
                        lastAppsName[searchOffset] = nullptr; // free entry due timeout
                        if ( nullptr != lastApps[searchOffset] ) {
                            lastApps[searchOffset]->deleteSprite();
                            delete lastApps[searchOffset];
                            lastApps[searchOffset]=nullptr;
                        }
                        lastAppsTimestamp[searchOffset]=0;
                        delay(80);
                        continue;
                    }
                    // otherwise, receive more keepalive time due has been seen now
                    lastAppsTimestamp[searchOffset] = millis();
                    lUILog("%p '%s' timeout updated (offset: %u)\n",ptrToCurrent,name,searchOffset);
                }

                if ( false == alreadyExists ) { // is new app
                    //TFT_eSprite *lastView = ScaleSprite(ptrToCurrent->canvas,0.333);
                    TFT_eSprite * lastView = ptrToCurrent->ScreenCapture();
                    if ( nullptr != lastView ) {
                        // add to "last apps stack"
                        lUILog("%p '%s' added to app stack (offset: %u)\n",ptrToCurrent,name,lastAppsOffset);
                        lastApps[lastAppsOffset] = lastView;
                        lastAppsName[lastAppsOffset] = name;
                        lastAppsTimestamp[lastAppsOffset] = millis();
                    }
                    lastAppsOffset++;
                    if ( lastAppsOffset > LUNOKIOT_MAX_LAST_APPS-1) { lastAppsOffset=0; }
                }
                xSemaphoreGive( lAppStack );
                delay(20);
            }
        }

        if (synched) {
            KillApplicationTaskSync(ptrToCurrent);
        } else {
            // kill app in other thread (some apps takes much time)
            //xTaskCreatePinnedToCore(KillApplicationTask, "", LUNOKIOT_TINY_STACK_SIZE,(void*)ptrToCurrent, uxTaskPriorityGet(NULL), nullptr,0);
            xTaskCreatePinnedToCore(KillApplicationTask, "", LUNOKIOT_TINY_STACK_SIZE,(void*)ptrToCurrent, uxTaskPriorityGet(NULL), nullptr,0);
        }
        delay(80);
        FreeSpace();
    }
}

void LaunchApplication(LunokIoTApplication *instance, bool animation,bool synced) {
    UINextTimeout = millis()+UITimeout;  // dont allow screen sleep
    /*
    if ( nullptr != instance ) {
        if ( instance == currentApplication) { // rare but possible (avoid: app is destroyed and pointer to invalid memory)
            lUILog("Application: %p Already running, ignoring launch\n", currentApplication);
            return;
        }
    }*/

    if ( ( nullptr != currentApplication ) && (nullptr != instance) ) {
        if ( 0 == strcmp( currentApplication->AppName(),instance->AppName() )) {
            lUILog("Application: %p Already running, ignoring re-launch of '%s'\n", currentApplication,currentApplication->AppName());
            delete(instance);
            return;
        }
    }


    LaunchApplicationDescriptor * thisLaunch = new LaunchApplicationDescriptor();
    thisLaunch->instance = instance;
    thisLaunch->animation = animation;
    if ( synced ) {
        LaunchApplicationTaskSync(thisLaunch,true); // forced sync
    } else {
        // apps receive the worst priority
        // launch a task guarantee free the PC (program counter CPU register) of caller object, and made possible a object in "this" context to destroy itself :)
        xTaskCreatePinnedToCore(LaunchApplicationTask, "", LUNOKIOT_TASK_STACK_SIZE,(void*)thisLaunch, tskIDLE_PRIORITY-1, nullptr,0);
    }
}

void LunokIoTApplication::LowMemory() {
    lAppLog("LunokIoTApplication: LOW MEMORY received\n");
    // free last apps!!
    int searchOffset = LUNOKIOT_MAX_LAST_APPS; // must be signed
    while ( searchOffset > 0 ) {
        searchOffset--;
        if ( nullptr != lastApps[searchOffset]) {
            lastApps[searchOffset]->deleteSprite();
            delete lastApps[searchOffset];
            lastApps[searchOffset]=nullptr;
        }

        if ( nullptr != lastAppsName[searchOffset]) {
            lastAppsName[searchOffset]=nullptr;
        }
        if ( 0 != lastAppsTimestamp[searchOffset]) {
            lastAppsTimestamp[searchOffset]=0;
        }
    }
    lastAppsOffset=0;
    /*
    if ( nullptr != canvas ) {
        // try to regenerate canvas (move memory) with a bit of lucky, the new location is a better place
        canvas->deleteSprite();
        void * oldcanvasPtr = canvas;
        delete canvas;
        canvas=nullptr;
        
        delay(80); // do little yeld (idle task frees memory)

        canvas = new TFT_eSprite(tft);
        if ( nullptr == canvas ) {
            lAppLog("ERROR: canvas cannot be regenerated: App must die :(\n");
            return;
        }
        canvas->setColorDepth(16); // try to experiment with different color depth
        if ( NULL == canvas->createSprite(TFT_WIDTH, TFT_HEIGHT) ) {
            lAppLog("ERROR: Unable to create new canvas: App must die :(\n");
            delete canvas;
            canvas=nullptr;
            return;
        }
        lAppLog("Regenerated canvas from: %p to new location: %p\n",oldcanvasPtr,canvas);
    }*/
}

/*
RunApplicationCallback LunokIoTApplication::GetRunCallback() {
    return [](void *unused){ return new LunokIoTApplication(); };
}
*/