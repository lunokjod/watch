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

#include <Arduino.h>
#include <ArduinoNvs.h>
#include <LilyGoWatch.h>
#include "../lunokIoT.hpp"
extern TTGOClass *ttgo; // access to ttgo specific libs

#include "Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/UI.hpp"

#include "../UI/transition/ZoomOut.hpp" // animation
#include "../UI/transition/Fade.hpp" // animation
#include "../UI/transition/Displace.hpp" // animation
#include "../UI/transition/Stripes.hpp" // animation
#include "../UI/transition/UpDown.hpp" // animation
#include "../UI/transition/Chess.hpp" // animation
#include "../UI/transition/Flip.hpp" // animation

#include "../app/LogView.hpp"

#include "../app/Watchface2.hpp"
#include "../app/WatchfaceSquare.hpp"
#include "../app/WatchfaceAlwaysOn.hpp"
#include "../app/WatchfaceDot.hpp"
#include "../app/WatchfaceBasic.hpp"

#include "SystemEvents.hpp"
#include <esp_task_wdt.h>
#include <functional>
#include <string.h>
#include "Datasources/database.hpp"

//extern Database * systemDatabase;
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../resources.hpp"
//#include "../static/img_trash_32.xbm"
//#include "../static/img_back_32.xbm"
//#include "../static/img_check_32.xbm"

#include "../system/Datasources/perceptron.hpp"
extern TTGOClass *ttgo;
extern bool UILongTapOverride;


SemaphoreHandle_t lAppStack = xSemaphoreCreateMutex();
// elegant https://stackoverflow.com/questions/10722858/how-to-create-an-array-of-classes-types
// MakeWatch<WatchfaceAlwaysOn>,
WatchfaceMaker* Watchfaces[] = { MakeWatch<Watchface2Application>,MakeWatch<WatchfaceSquare>,
                                MakeWatch<WatchfaceDotApplication>,MakeWatch<WatchfaceBasic> };

size_t WatchFacesAvailiable() { return sizeof(Watchfaces)/sizeof(WatchfaceMaker*); }

LunokIoTApplication *GetWatchFaceFromOffset(size_t off) { return Watchfaces[off](); }
LunokIoTApplication *GetWatchFace() { return GetWatchFaceFromOffset(LoT().selectedWatchFace); }

LunokIoTApplication *currentApplication = nullptr; // ptr to get current foreground application

void LaunchWatchface(bool animation, bool forced) {
    bool launch=false;
    if ( nullptr == currentApplication ) {
        launch=true;
    } else if ( false == currentApplication->isWatchface() ) {
        launch=true;
    }
    //xSemaphoreGive( UISemaphore );
    if ( forced ) { launch = true; }
    //lLog("@DEBUG LAUNCH WATCHFACE\n");
    if (launch) { LaunchApplication(GetWatchFace(),animation); }
    //lLog("@DEBUG LAUNCH WATCHFACE ENDS\n");
}

size_t lastAppsOffset=0;
const char  * lastAppsName[LUNOKIOT_MAX_LAST_APPS] = { nullptr }; 
unsigned long lastAppsTimestamp[LUNOKIOT_MAX_LAST_APPS] = { 0 }; 
TFT_eSprite * lastApps[LUNOKIOT_MAX_LAST_APPS] = { nullptr };

// get a capture of current application
//TFT_eSprite *LunokIoTApplication::ScreenCapture() { return DuplicateSprite(canvas); }

LunokIoTApplication::LunokIoTApplication() {
    esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_APP_LAUNCH, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    //delay(120); // makes the timer launch animation possible :(
    //taskYIELD(); // force context switch
    directDraw=false;
    UILongTapOverride=false;
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

bool LunokIoTApplication::Tick() {
    if ( dirty ) {
        dirty=false;
        return true;
    } // true to notify to UI the full screen redraw
    return false;
}

// This task destroy the last app in a second thread trying to maintain the user experience
void KillApplicationTaskSync(LunokIoTApplication *instance) {
    if ( nullptr != instance ) {
        lUILog("KillApplicationTask: %p '%s' closing...\n", instance,instance->AppName());
        delete instance;
        lUILog("KillApplicationTask: %p has gone\n", instance);
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
    LaunchApplicationTaskSync(dataDesc,true);
    vTaskDelete(NULL); // get out of my cicken!!!
}
void LaunchApplicationTaskSync(LaunchApplicationDescriptor * appDescriptor,bool synched) {
//    LaunchApplicationDescriptor * dataDesc =  (LaunchApplicationDescriptor *)data;
    esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_APP_LAUNCH_END, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    LunokIoTApplication *instance = appDescriptor->instance; // get app instance loaded
    bool animation = appDescriptor->animation;
    delete appDescriptor;
    //lLog("@DEBUG TAKE SEMAPHORE TASK SYNC\n");
    xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    LunokIoTApplication * ptrToCurrent = currentApplication;
    if ( nullptr == instance ) { // this situation is indeed as prior to screen sleep
        lUILog("Application: None\n");
        currentApplication = nullptr;     // no one driving now x'D
        tft->fillScreen(TFT_BLACK); // at this point, only system is working, the UI is in a dead-end in a "null application"
        // UI is dead beyond here, no app on foreground, only external events or timed triggers can solve this point :)
    } else {
        lUILog("Application: %p '%s' goes to front\n", instance,instance->AppName());
        FPS=MAXFPS; // reset refresh rate
        currentApplication = instance; // set as main app
        TFT_eSprite *appView = instance->canvas;
        if ( ( nullptr != appView ) && (nullptr != ptrToCurrent)) {
            if ( animation ) {
                // only if two canvas are ready
                if ( ( nullptr != ptrToCurrent->canvas ) && ( nullptr != appView ) ) {
                    // no datbase use during the transition (speed up)
                    if ( nullptr != systemDatabase ) { systemDatabase->Lock(); }
                        // temporal disable watchdog for me :)
                        esp_err_t susbcribed = esp_task_wdt_status(NULL);
                        if ( ESP_OK == susbcribed) { esp_task_wdt_delete(NULL); }

                        // set better priority
                        UBaseType_t myPriority = uxTaskPriorityGet(NULL);
                        vTaskPrioritySet(NULL,UITRANSITIONPRIORITY);
                        //taskYIELD();
                        static long transitionSelected = 0;
                        lUILog("Application: %p '%s' Transition: %ld\n", instance,instance->AppName(),transitionSelected);
                        //lUILog("@TODO DEBUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUG TRANSITION\n");
                        //transitionSelected=6; //@TODO DEBUG
                        // run this part fast as possible
                        if ( 0 == transitionSelected ) { ZoomOutTransition(ptrToCurrent->canvas,appView); }
                        else if ( 1 == transitionSelected ) { FadeTransition(ptrToCurrent->canvas,appView); }
                        else if ( 2 == transitionSelected ) { DisplaceTransition(ptrToCurrent->canvas,appView); }
                        else if ( 3 == transitionSelected ) { StripeTransition(ptrToCurrent->canvas,appView); }
                        else if ( 4 == transitionSelected ) { UpDownTransition(ptrToCurrent->canvas,appView); }
                        else if ( 5 == transitionSelected ) { ChessTransition(ptrToCurrent->canvas,appView); }
                        else if ( 6 == transitionSelected ) { FlipTransition(ptrToCurrent->canvas,appView); }
                        transitionSelected++;
                        if ( transitionSelected > 6 ) { transitionSelected = 0; }

                        // restore my priority
                        vTaskPrioritySet(NULL,myPriority);

                        // restore watchdog
                        esp_task_wdt_reset();
                        if ( ESP_OK == susbcribed) { esp_task_wdt_add(NULL); }
                        esp_task_wdt_reset();
                        //taskYIELD();
                    if ( nullptr != systemDatabase ) { systemDatabase->UnLock(); }
                }
            }
        }
        if ( false == currentApplication->isWatchface() ) { // don't log watchfaces!
            char logMsg[255] = { 0 }; 
            sprintf(logMsg,"Application: %p:%s",currentApplication,currentApplication->AppName());
            SqlLog(logMsg);
        }
    }
    xSemaphoreGive( UISemaphore ); // free
    UIRefresh();
    /*
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
                    //delay(80); // maybe is supersticious, but, bring idle task a little time to perform the free() is better for us :P
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
                    //delay(80);
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
            //delay(20);
        }
    }*/
    if ( nullptr != ptrToCurrent ) {
        //if (synched) {
        //    KillApplicationTaskSync(ptrToCurrent);
        //} else {
            // kill app in other thread (some apps takes much time)
            xTaskCreatePinnedToCore(KillApplicationTask, "lkillApp", LUNOKIOT_QUERY_STACK_SIZE,(void*)ptrToCurrent, tskIDLE_PRIORITY, nullptr,SYSTEMCORE);
        //}
    }
    FreeSpace();
}

//Ticker LogAppRun;
void LaunchApplication(LunokIoTApplication *instance, bool animation,bool synced,bool force) {
    UINextTimeout = millis()+UITimeout;  // dont allow screen sleep
    //@TODO MUTEX APP CHANGE
    if ( false == force ) {
        //lLog("@DEBUG TAKE SEMAPHORE LAUNCH APPPPPPP\n");
        //xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        if ( nullptr != instance ) {
            if ( instance == currentApplication) { // rare but possible (avoid: app is destroyed and pointer to invalid memory)
                //xSemaphoreGive( UISemaphore );
                lUILog("Application: %p Already running, ignoring launch\n", currentApplication);
                return;
            }
        }
        if ( ( nullptr != currentApplication ) && (nullptr != instance) ) {
            if ( 0 == strcmp( currentApplication->AppName(),instance->AppName() )) {
                //xSemaphoreGive( UISemaphore );
                lUILog("Application: %p Already running, ignoring re-launch of '%s'\n", currentApplication,currentApplication->AppName());
                delete(instance);
                return;
            }
        }
        //xSemaphoreGive( UISemaphore );
        //lLog("@DEBUG TAKE SEMAPHORE LAUNCH OFFF APPPPPPP\n");
    }

    LaunchApplicationDescriptor * thisLaunch = new LaunchApplicationDescriptor();
    thisLaunch->instance = instance;
    thisLaunch->animation = animation;
    if ( synced ) {
        LaunchApplicationTaskSync(thisLaunch,true); // forced sync
    } else {
        // apps receive the worst priority
        // launch a task guarantee free the PC (program counter CPU register) of caller object, and made possible a object in "this" context to destroy itself :)
        xTaskCreatePinnedToCore(LaunchApplicationTask, "", LUNOKIOT_TASK_STACK_SIZE,(void*)thisLaunch, tskIDLE_PRIORITY, nullptr,APPLICATIONCORE);
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
    /* @TODO maybe building before delete... but this must be increment the fragmentation x'D
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
#define KEYBOARD_KILL_DELAY 80
#include "../UI/widgets/EntryTextWidget.hpp"
SoftwareKeyboard *keyboardInstance=nullptr;
Ticker destroyKeyboard;

SoftwareKeyboard::SoftwareKeyboard(void * destinationWidget) : destinationWidget(destinationWidget) {
    textEntry = (char*)malloc(256); // buffer
    textEntry[0] = 0; // end of line
}
SoftwareKeyboard::~SoftwareKeyboard() {
    if ( nullptr != textEntry ) { free(textEntry); }
}

SoftwareNumericKeyboard::SoftwareNumericKeyboard(void * destinationWidget) {
    this->destinationWidget=destinationWidget;
    if ( nullptr == destinationWidget ) { return; }
    //if ( nullptr == fromCanvas ) { return; }
    EntryTextWidget * sendTo = (EntryTextWidget *)destinationWidget;
    int16_t tX = sendTo->GetX();
    int16_t tY = sendTo->GetY();
    int16_t tW = sendTo->GetW();
    int16_t tH = sendTo->GetH();
    //if( xSemaphoreTake( UISemaphore, portMAX_DELAY) == pdTRUE )  {
    //appCopy = DuplicateSprite(fromCanvas);
    //    xSemaphoreGive( UISemaphore );
    //}

    const int16_t ButtonsOffset = 40;
    const int16_t ButtonsHeightSize = 50;
    const int16_t ButtonsWidthSize = 80;
    const int16_t XOffset=0;
    
    UICallback numberButtonCallback = [&,this](void *payload) {
        ButtonTextWidget * pushedBtn = (ButtonTextWidget *)payload;
        //@TODO Bug here, the predefined values already can contain dot
        if (0 == strcmp(".",pushedBtn->label)) {
            //lLog("IS DOT!\n");
            if ( true == this->dotAdded ) { return; } // the number already have a dot, maintain until reset
            this->dotAdded =true;
        }
        char * textEntryBack = (char*)malloc(256); // buffer
        strcpy(textEntryBack,textEntry);
        //lLog("VALUE ADDED: '%s'\n",pushedBtn->label);
        sprintf(textEntry,"%s%s",textEntryBack,pushedBtn->label);
        free(textEntryBack);
    };

    seven = new ButtonTextWidget(XOffset,ButtonsOffset,ButtonsHeightSize,ButtonsWidthSize,numberButtonCallback,"7",TFT_WHITE,CanvasWidget::MASK_COLOR,false);
    seven->paramCallback=seven;
    eight = new ButtonTextWidget(XOffset+ButtonsWidthSize,ButtonsOffset,ButtonsHeightSize,ButtonsWidthSize,numberButtonCallback,"8",TFT_WHITE,CanvasWidget::MASK_COLOR,false);
    eight->paramCallback=eight;
    nine = new ButtonTextWidget(XOffset+(ButtonsWidthSize*2),ButtonsOffset,ButtonsHeightSize,ButtonsWidthSize,numberButtonCallback,"9",TFT_WHITE,CanvasWidget::MASK_COLOR,false);
    nine->paramCallback=nine;

    four = new ButtonTextWidget(XOffset,ButtonsOffset+(ButtonsHeightSize),ButtonsHeightSize,ButtonsWidthSize,numberButtonCallback,"4",TFT_WHITE,CanvasWidget::MASK_COLOR,false);
    four->paramCallback=four;
    five = new ButtonTextWidget(XOffset+ButtonsWidthSize,ButtonsOffset+(ButtonsHeightSize),ButtonsHeightSize,ButtonsWidthSize,numberButtonCallback,"5",TFT_WHITE,CanvasWidget::MASK_COLOR,false);
    five->paramCallback=five;
    six = new ButtonTextWidget(XOffset+(ButtonsWidthSize*2),ButtonsOffset+(ButtonsHeightSize),ButtonsHeightSize,ButtonsWidthSize,numberButtonCallback,"6",TFT_WHITE,CanvasWidget::MASK_COLOR,false);
    six->paramCallback=six;

    one = new ButtonTextWidget(XOffset,ButtonsOffset+(ButtonsHeightSize*2),ButtonsHeightSize,ButtonsWidthSize,numberButtonCallback,"1",TFT_WHITE,CanvasWidget::MASK_COLOR,false);
    one->paramCallback=one;
    two = new ButtonTextWidget(XOffset+ButtonsWidthSize,ButtonsOffset+(ButtonsHeightSize*2),ButtonsHeightSize,ButtonsWidthSize,numberButtonCallback,"2",TFT_WHITE,CanvasWidget::MASK_COLOR,false);
    two->paramCallback=two;
    three = new ButtonTextWidget(XOffset+(ButtonsWidthSize*2),ButtonsOffset+(ButtonsHeightSize*2),ButtonsHeightSize,ButtonsWidthSize,numberButtonCallback,"3",TFT_WHITE,CanvasWidget::MASK_COLOR,false);
    three->paramCallback=three;


    dotBtn = new ButtonTextWidget(XOffset,ButtonsOffset+(ButtonsHeightSize*3),ButtonsHeightSize,ButtonsWidthSize,numberButtonCallback,".",TFT_WHITE,CanvasWidget::MASK_COLOR,false);
    dotBtn->paramCallback=dotBtn;

    zero = new ButtonTextWidget(XOffset+ButtonsWidthSize,ButtonsOffset+(ButtonsHeightSize*3),ButtonsHeightSize,ButtonsWidthSize,numberButtonCallback,"0",TFT_WHITE,CanvasWidget::MASK_COLOR,false);
    zero->paramCallback=zero;

    btnCancel=new ButtonImageXBMWidget(canvas->width()-40,0, 40, 40, [&,this](void * useless){
        destroyKeyboard.once_ms(KEYBOARD_KILL_DELAY,[](){
            EntryTextWidget * ctrl = (EntryTextWidget*)keyboardInstance->destinationWidget;
            ctrl->HideKeyboard(); // must be async to not call my own destructor
        });
    },(const uint8_t *)img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,TFT_BLACK,true);
    btnDiscard=new ButtonImageXBMWidget(0,0, 40, 40, [&,this](void * useless){
        textEntry[0]=0; // reset the string
        dotAdded=false;
    },(const uint8_t *)img_trash_32_bits,img_trash_32_height,img_trash_32_width,TFT_WHITE,TFT_BLACK,true);
    btnSend=new ButtonImageXBMWidget(canvas->width()-50,canvas->height()-50, 50, 50, [&,this](void * useless){
        destroyKeyboard.once_ms(KEYBOARD_KILL_DELAY,[](){
            EntryTextWidget * ctrl = (EntryTextWidget*)keyboardInstance->destinationWidget;
            ctrl->HideKeyboard(keyboardInstance->textEntry); // must be async to not call my own destructor
        });
    }, (const uint8_t *)img_check_32_bits,img_check_32_height,img_check_32_width,TFT_GREEN,ThCol(background),false);

    Tick();
    //@TODO do anim slide here
}

SoftwareNumericKeyboard::~SoftwareNumericKeyboard() {
    delete seven;
    delete eight;
    delete nine;
    delete four;
    delete five;
    delete six;
    delete one;
    delete two;
    delete three;
    delete zero;
    
    delete dotBtn;

    delete btnCancel;
    delete btnDiscard;
    delete btnSend;
    /*
    if ( nullptr != appCopy ) {
        appCopy->deleteSprite();
        delete appCopy;
    }*/
}
#include "../UI/widgets/EntryTextWidget.hpp"
bool SoftwareNumericKeyboard::Tick() {
    EntryTextWidget * sendTo = (EntryTextWidget *)destinationWidget;
    /*
    if (nullptr != appCopy ) {
        appCopy->setPivot(0,0);
        canvas->setPivot(0,0);
        appCopy->pushRotated(canvas,0);
    } else { */
        canvas->fillSprite(ThCol(background));
    //}

    seven->Interact(touched,touchX,touchY);
    eight->Interact(touched,touchX,touchY);
    nine->Interact(touched,touchX,touchY);
    four->Interact(touched,touchX,touchY);
    five->Interact(touched,touchX,touchY);
    six->Interact(touched,touchX,touchY);
    one->Interact(touched,touchX,touchY);
    two->Interact(touched,touchX,touchY);
    three->Interact(touched,touchX,touchY);
    zero->Interact(touched,touchX,touchY);

    dotBtn->Interact(touched,touchX,touchY);

    btnCancel->Interact(touched,touchX,touchY);
    btnDiscard->Interact(touched,touchX,touchY);
    btnSend->Interact(touched,touchX,touchY);
   
    seven->DrawTo(canvas);
    eight->DrawTo(canvas);
    nine->DrawTo(canvas);
    four->DrawTo(canvas);
    five->DrawTo(canvas);
    six->DrawTo(canvas);
    one->DrawTo(canvas);
    two->DrawTo(canvas);
    three->DrawTo(canvas);
    zero->DrawTo(canvas);

    dotBtn->DrawTo(canvas);



    // background of typed number
    canvas->fillRect(40,0,canvas->width()-80,40,TFT_BLACK);
    // cursor blink
    cursorBlinkStep+=10;
    uint32_t color=ThCol(background);
    if ( cursorBlinkStep < 127) { color=TFT_WHITE; }
    canvas->fillRect(canvas->width()-65,5,20,30,color); // bold cursor
    //canvas->drawFastVLine(canvas->width()-20,5,30,color); // bar cursor

    if ( strlen(textEntry) > 0 ) {
        //lLog("Draw textEntry: '%s'\n",textEntry);
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextFont(0);
        canvas->setTextSize(3);
        canvas->setTextDatum(TR_DATUM);
        canvas->setTextWrap(true,true);
        canvas->drawString(textEntry,canvas->width()-70,8);
    }

    btnCancel->DrawTo(canvas);
    btnDiscard->DrawTo(canvas);
    btnSend->DrawTo(canvas);


    return true;
}

SoftwareFreehandKeyboard::SoftwareFreehandKeyboard(void * destinationWidget)  {
    this->destinationWidget=destinationWidget; // overwrite
    btnCancel=new ButtonImageXBMWidget(10,canvas->width()-50, 40, 40, [&,this](void * useless){
        destroyKeyboard.once_ms(KEYBOARD_KILL_DELAY,[](){
            EntryTextWidget * ctrl = (EntryTextWidget*)keyboardInstance->destinationWidget;
            ctrl->HideKeyboard(); // must be async to not call my own destructor
        });
    },(const uint8_t *)img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,TFT_BLACK,false);
    btnDiscard=new ButtonImageXBMWidget((canvas->height()/2)-20,canvas->width()-50, 40, 40, [&,this](void * useless){
        textEntry[0]=0; // reset the string
    },(const uint8_t *)img_trash_32_bits,img_trash_32_height,img_trash_32_width,TFT_WHITE,TFT_BLACK,false);
    btnSend=new ButtonImageXBMWidget(canvas->width()-50,canvas->height()-50, 50, 50, [&,this](void * useless){
        destroyKeyboard.once_ms(KEYBOARD_KILL_DELAY,[](){
            EntryTextWidget * ctrl = (EntryTextWidget*)keyboardInstance->destinationWidget;
            ctrl->HideKeyboard(keyboardInstance->textEntry); // must be async to not call my own destructor
        });
    }, (const uint8_t *)img_check_32_bits,img_check_32_height,img_check_32_width,TFT_GREEN,ThCol(background),false);

    freeHandCanvas = new TFT_eSprite(tft);
    freeHandCanvas->setColorDepth(16);
    freeHandCanvas->createSprite(canvas->width(),canvas->height());
    freeHandCanvas->fillSprite(Drawable::MASK_COLOR);

    perceptronCanvas = new TFT_eSprite(tft);
    perceptronCanvas->setColorDepth(1);
    perceptronCanvas->createSprite(PerceptronMatrixSize,PerceptronMatrixSize);
    perceptronCanvas->fillSprite(0);

    //symbolsTrained=(Perceptron*)ps_calloc(sizeof(TrainableSymbols),sizeof(Perceptron));
    lAppLog("Symbols trainables: %zu\n",FreehandKeyboardLetterTrainableSymbolsCount);
    perceptrons=(Perceptron **)ps_calloc(FreehandKeyboardLetterTrainableSymbolsCount,sizeof(Perceptron *));
    lAppLog("Loading all perceptrons...\n");
    for(size_t current=0;current<FreehandKeyboardLetterTrainableSymbolsCount;current++) {
        lAppLog("Loading symbol '%c'...\n",FreehandKeyboardLetterTrainableSymbols[current]);
        Perceptron * thaPerceptron = LoadPerceptronFromSymbol(FreehandKeyboardLetterTrainableSymbols[current]);
        perceptrons[current] = thaPerceptron;
    }
    /*
    bufferText=(char*)malloc(BufferTextMax);
    bufferText[0]=0; //EOL (empty string)
    */
    RedrawMe();
    UILongTapOverride=true;
}

SoftwareFreehandKeyboard::~SoftwareFreehandKeyboard() {
    delete btnCancel;
    delete btnDiscard;
    delete btnSend;

    for(size_t current=0;current<FreehandKeyboardLetterTrainableSymbolsCount;current++) {
        if ( nullptr == perceptrons[current]) { continue; }
        RemovePerceptron(perceptrons[current]);
    }
    free(perceptrons);
    freeHandCanvas->deleteSprite();
    delete freeHandCanvas;
    perceptronCanvas->deleteSprite();
    delete perceptronCanvas;
    //free(bufferText);
    UILongTapOverride=false;
}
Perceptron * SoftwareFreehandKeyboard::BuildBlankPerceptron() {
    lAppLog("Blank perceptron\n");
    // 0.2 of training rate
    void * ptrData = Perceptron_new(PerceptronMatrixSize*PerceptronMatrixSize, 0.2);
    Perceptron * current = (Perceptron *)ptrData;
    return current;
}
void SoftwareFreehandKeyboard::RemovePerceptron(Perceptron * item) {
    if ( nullptr != item ) {
        lAppLog("Free perceptron memory...\n");
        free(item);
        item=nullptr;
    }
}
Perceptron * SoftwareFreehandKeyboard::LoadPerceptronFromSymbol(char symbol) {
    lAppLog("NVS: Load perceptron...\n");
    // get the key name
    char keyName[15];
    if ( ' ' == symbol ) { symbol = '_'; } // changed to
    sprintf(keyName,"freeHandSym_%c", symbol);

    // Load from NVS
    bool loaded = false; 
    size_t totalSize = NVS.getBlobSize(keyName);
    char *bufferdata = nullptr;
    if ( 0 == totalSize ) {
        lAppLog("NVS: Unable to get perceptron from '%c'\n",symbol);
        return nullptr;
    }
    bufferdata = (char *)ps_malloc(totalSize); // create buffer "packed file"
    loaded = NVS.getBlob(keyName, (uint8_t*)bufferdata, totalSize);
    if ( false == loaded ) {
        lAppLog("NVS: Unable to get '%c'\n",symbol);
        return nullptr;
    }
    Perceptron * currentSymbolPerceptron=(Perceptron *)bufferdata;
    currentSymbolPerceptron->weights_=(double*)(bufferdata+sizeof(Perceptron)); // correct the memory pointer
    lAppLog("Perceptron loaded from NVS (trained: %u times)\n",currentSymbolPerceptron->trainedTimes);
    return currentSymbolPerceptron;
}

void SoftwareFreehandKeyboard::Cleanup() {
    // cleanup
    freeHandCanvas->fillSprite(Drawable::MASK_COLOR);
    perceptronCanvas->fillSprite(0);
    triggered=false;
    oneSecond=0;

    boxX=TFT_WIDTH; // restore default values
    boxY=TFT_HEIGHT;
    boxH=0;
    boxW=0;
}

bool SoftwareFreehandKeyboard::Tick() {
    bool interacted = btnCancel->Interact(touched,touchX,touchY);
    if ( interacted ) { RedrawMe(); return true; }
    interacted = btnDiscard->Interact(touched,touchX,touchY);
    if ( interacted ) { RedrawMe(); return true; }
    interacted = btnSend->Interact(touched,touchX,touchY);
    if ( interacted ) { RedrawMe(); return true; }

    const int32_t RADIUS=14;
    if ( touched ) {
        // draw on freehand canvas
        freeHandCanvas->fillCircle(touchX,touchY,RADIUS, ThCol(text)); // to the buffer (ram operation is fast)
        tft->fillCircle(touchX,touchY,RADIUS, ThCol(text)); // hardware direct
        const float ratioFromScreen = canvas->width()/PerceptronMatrixSize; 
        perceptronCanvas->drawPixel(touchX/ratioFromScreen,touchY/ratioFromScreen,1);
        if ( touchX < boxX ) { boxX = touchX; }
        if ( touchY < boxY ) { boxY = touchY; }
        if ( touchX > boxW ) { boxW = touchX; }
        if ( touchY > boxH ) { boxH = touchY; }
        triggered=true;
        oneSecond=0;
        return false; // fake no changes (already draw on tft)
    } else {
        if ( triggered ) {
            if ( 0 == oneSecond ) {
                boxX-=RADIUS; // zoom out to get whole draw
                boxY-=RADIUS;
                boxH+=RADIUS*2;
                boxW+=RADIUS*2;
                // don't get out of margins
                if ( boxX < 0 ) { boxX=0; }
                if ( boxY < 0 ) { boxY=0; }
                if ( boxH > canvas->height() ) { boxH=canvas->height(); }
                if ( boxW > canvas->width() ) { boxW=canvas->width(); }

                lAppLog("RECT: %d %d %d %d\n",boxX,boxY,boxH-boxY,boxW-boxX);
                canvas->drawRect(boxX-1,boxY-1,(boxW-boxX)+2,(boxH-boxY)+2,TFT_RED); // mark the area to recognize
                canvas->drawRect(boxX,boxY,boxW-boxX,boxH-boxY,TFT_RED);             // mark the area to recognize

                oneSecond=millis()+800; // grace time, maybe the user need to add something before send to the perceptron
                return true;
            } else if ( millis() > oneSecond ) {
                bool valid=false;
                if ( ( boxW-boxX > MINIMALDRAWSIZE ) || ( boxH-boxY > MINIMALDRAWSIZE)) { valid=true; }
                if ( false == valid ) {
                    lAppLog("Discarded (too small draw<%dpx)\n",MINIMALDRAWSIZE);
                    Cleanup();
                    return false;
                }


                const size_t MemNeeds =sizeof(double)*(PerceptronMatrixSize*PerceptronMatrixSize);
                lAppLog("Recognizer matrix (%zu byte):\n",MemNeeds);
                double *perceptronData=(double*)ps_malloc(MemNeeds); // alloc size for train the perceptrons
                memset(perceptronData,0,MemNeeds); // all 0

                double *pDataPtr;
                pDataPtr=perceptronData;
                for (int16_t y=0;y<PerceptronMatrixSize;y++) {
                    pDataPtr=perceptronData+(PerceptronMatrixSize*y);

                    for (int16_t x=0;x<perceptronCanvas->width();x++) {
                        uint32_t color = perceptronCanvas->readPixel(x,y);

                        // 0 nothing 1 draw
                        bool isData= false;
                        if ( TFT_WHITE == color ){ // write only 1 (memset already put 0 on all)
                            *pDataPtr=1;
                            isData=true;
                        }
                        lLog("%c",(isData?'O':' '));
                        pDataPtr++;
                    }
                    lLog("\n");
                }
                char seeChar = 0;
                for(size_t current=0;current<FreehandKeyboardLetterTrainableSymbolsCount;current++) {
                    Perceptron * thaPerceptron = perceptrons[current];
                    if (  nullptr == thaPerceptron ) { continue; }
                    int pResponse0 = Perceptron_getResult(thaPerceptron,perceptronData);
                    if ( 1 == pResponse0 ) {
                        lAppLog("Symbol: '%c' MATCH\n",FreehandKeyboardLetterTrainableSymbols[current]);
                        if ( 0 == seeChar ) {
                            seeChar=FreehandKeyboardLetterTrainableSymbols[current];
                            break; // stop search here
                        }
                    }
                }
                if ( 0 != seeChar ) {
                    char * textEntryBack = (char*)malloc(256); // buffer
                    strcpy(textEntryBack,textEntry);
                    //lLog("VALUE ADDED: '%s'\n",pushedBtn->label);
                    sprintf(textEntry,"%s%c",textEntryBack,seeChar);
                    free(textEntryBack);
                }
                seeChar=0;

                Cleanup();
            }
        }
    }

    if ( millis() > nextRefresh ) { // redraw full canvas
        RedrawMe();
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}

void SoftwareFreehandKeyboard::RedrawMe() {
    canvas->fillSprite(ThCol(background)); // use theme colors
    const int32_t BORDER=20;
    canvas->fillRoundRect(BORDER,30,
    canvas->width()-(BORDER*2),
    canvas->height()-90,
    BORDER/2,ThCol(background_alt));

    canvas->drawFastHLine(30,50,canvas->width()-60,ThCol(shadow));
    canvas->drawFastHLine(30,140,canvas->width()-60,ThCol(light));
    canvas->drawFastHLine(30,155,canvas->width()-60,ThCol(light));

    if ( (false == touched) || ( 0 == touchDragDistance )) {
        btnCancel->DrawTo(canvas);
        btnDiscard->DrawTo(canvas);
        btnSend->DrawTo(canvas);
    }
    canvas->setFreeFont(&FreeMonoBold9pt7b);
    canvas->setTextDatum(TR_DATUM);
    canvas->setTextColor(ThCol(text));
    canvas->drawString(textEntry,TFT_WIDTH-10,5);


    freeHandCanvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
}

