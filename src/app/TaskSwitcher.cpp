#include <Arduino.h>
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "TaskSwitcher.hpp"
#include <LilyGoWatch.h>

extern TTGOClass *ttgo; // access to ttgo specific libs
extern size_t lastAppsOffset;
extern const char  * lastAppsName[LUNOKIOT_MAX_LAST_APPS]; 
extern TFT_eSprite * lastApps[LUNOKIOT_MAX_LAST_APPS];
extern unsigned long lastAppsTimestamp[LUNOKIOT_MAX_LAST_APPS];
extern SemaphoreHandle_t lAppStack;
extern unsigned long touchDownTimeMS;

TaskSwitcher::TaskSwitcher() {
    // Init here any you need
    //lAppLog("Hello from LastApps!\n");

    canvas->fillSprite(TFT_BLACK); // use theme colors

    // Remember to dump anything to canvas to get awesome splash screen
    // draw of TFT_eSPI canvas for splash here
    dirty=true;
    Tick(); // OR call this if no splash 
    //directDraw=true;
}

TaskSwitcher::~TaskSwitcher() {

    int searchOffset = LUNOKIOT_MAX_LAST_APPS; // must be signed
    while ( searchOffset > 0 ) {
        searchOffset--;
        if ( nullptr != captures[searchOffset] ) {
            //lAppLog("Freeing low resolution of %u thumbnail at: %p\n",searchOffset,captures[searchOffset]);
            captures[searchOffset]->deleteSprite();
            delete captures[searchOffset];  
            captures[searchOffset]=nullptr;              
        }
    }
    //directDraw=false;
}

bool TaskSwitcher::Tick() {
    /*
    if ( 0 == touchDownTimeMS ) { // other way to detect touch :P
        longTap=false;
    } else { 
        // touch
        unsigned long pushTime = millis() - touchDownTimeMS; // how much time?
        if ( pushTime > LUNOKIOT_TOUCH_LONG_MS ) {
            if ( false == longTap ) {
                longTap=true;
                //lAppLog("@TODO long push back?\n");
                
                //#ifdef LILYGO_WATCH_2020_V3
                //    ttgo->shake();
                //    delay(200);
                //#endif

                LaunchWatchface();
                return false;
            }
        }
    }*/

    if ( millis() > nextRefresh ) {
        dirty=true;
        nextRefresh=millis()+(1000/2);
    }
    if ( dirty ) {
        //ttgo->tft->fillScreen(TFT_BLACK);
        canvas->fillSprite(TFT_BLACK); // use theme colors
        int searchOffset = LUNOKIOT_MAX_LAST_APPS; // must be signed value
        bool alreadyExists=false;
        int16_t x=0;
        int16_t y=0;
        //lLog("@TODO ugly hardcoded list of hidden apps (use const string and share with the apps AppName())\n");
        if( xSemaphoreTake( lAppStack, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) == pdTRUE )  {
            while ( searchOffset > 0 ) {
                searchOffset--;

                if ( 0 == lastAppsTimestamp[searchOffset] )   { continue; }
                if ( nullptr == lastAppsName[searchOffset] )  { continue; }
                if ( nullptr == lastApps[searchOffset] )      { continue; }
                if ( 0 == strcmp(lastAppsName[searchOffset],"Task switcher") ) {
                    continue; // hide myself
                }
                if ( 0 == strcmp(lastAppsName[searchOffset],"Main menu") ) {
                    continue; // the menu is accesable more easily
                }
                if ( 0 == strcmp(lastAppsName[searchOffset],"Settings menu") ) {
                    continue; // secondary menu
                }
                // probably timeouted by the system... ¿@think is necessary dissapear in the fingers of the user? maybe TaskSwitch don't must follow the timeouts
                if ( millis() > (lastAppsTimestamp[searchOffset]+(LUNOKIOT_RECENT_APPS_TIMEOUT_S*1000))) {
                    //lAppLog("Task %d is timed-out\n",searchOffset);
                    continue;
                }
                // clean up!!! clean up!!!!!!
                if ( nullptr != captures[searchOffset] ) {
                    //lAppLog("Removing thumbnail %u at: %p\n",searchOffset,captures[searchOffset]);
                    captures[searchOffset]->deleteSprite();
                    delete captures[searchOffset];
                    captures[searchOffset]=nullptr;
                }
                // update the app capture
                // little black margin
                captures[searchOffset] = ScaleSprite(lastApps[searchOffset], 0.32);//0.3333);
                /*
                if ( nullptr == captures[searchOffset] ) {
                    lAppLog("Unable to create thumbnail from last app: %u canvas at: %p, discarded\n",searchOffset,lastApps[searchOffset]);
                    continue;
                } // unable to get 
                //lAppLog("Created low resolution of %d canvas at: %p from: %p\n",searchOffset,captures[searchOffset],lastApps[searchOffset]);
                */
               if ( nullptr != captures[searchOffset] ) { // this code allow "holes" on task mosaic
                    canvas->setPivot(x,y);
                    captures[searchOffset]->setPivot(0,0);
                    captures[searchOffset]->pushRotated(canvas,0,CanvasWidget::MASK_COLOR);
                    //lAppLog("RENDERING: %p '%s' %d at X: %d Y: %d\n",lastApps[searchOffset],lastAppsName[searchOffset],searchOffset,x,y);
                }
                // build 3x3 grid
                x+=canvas->width()/3; //captures[searchOffset]->width();
                if ( x >= canvas->width() ) { x=0; y+=(canvas->height()/3); } //captures[searchOffset]->height(); }
                if ( y >= canvas->height() ) {
                    break;
                }

            }
            //lAppLog("----------------------------\n");
            xSemaphoreGive( lAppStack );
            delay(80);
        }
        dirty=false;
        return true;
    }
    return false;
}
