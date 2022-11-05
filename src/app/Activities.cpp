#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../lunokiot_config.hpp"

#include "Activities.hpp"

#include "../static/img_activity_running.xbm"
#include "../static/img_activity_walking.xbm"
#include "../static/img_activity_stay.xbm"

ActivitiesApplication::~ActivitiesApplication() {
    if ( nullptr != giroLog ) {
        giroLog->deleteSprite();
        delete giroLog;
    }
}

ActivitiesApplication::ActivitiesApplication() {
    counter = 0;
    giroLog = new TFT_eSprite(ttgo->tft);
    giroLog->setColorDepth(16);
    giroLog->createSprite(TFT_WIDTH, 100);
    giroLog->fillSprite(TFT_BLACK);
    //giroLog->setSwapBytes(true);
    nextSampleGiro = 0;
    nextRedraw = 0;
    angle=0;
}
int16_t lWpcX = 0;
int16_t lWpcY = 0;
int16_t lWpcZ = 0;

bool ActivitiesApplication::Tick() {

    uint32_t activityColor = giroLog->color565(64,32,32);
    if ( nullptr != currentActivity ) {
        if ( 0 == strcmp("BMA423_USER_STATIONARY",currentActivity)) {
            activityColor = giroLog->color565(255,0,0);
        } else if ( 0 == strcmp("BMA423_USER_WALKING",currentActivity)) {
            activityColor = giroLog->color565(0,255,255);
        } else if ( 0 == strcmp("BMA423_USER_RUNNING",currentActivity)) {
            activityColor = giroLog->color565(0,255,0);
        } else if ( 0 == strcmp("BMA423_STATE_INVALID",currentActivity)) {
            activityColor = giroLog->color565(0,0,0);
        } else if ( 0 == strcmp("None",currentActivity)) {
            activityColor = giroLog->color565(64,32,32);
        } else {
            activityColor = giroLog->color565(255,255,255);
        }
    }
    giroLog->scroll(6,0);
    /*
    /// RGB -> BRG !!!! OMG!!! shit
    giroLog->drawFastVLine(0,50-(pcX/2), pcX, giroLog->color565(0,255,0));
    giroLog->drawFastVLine(2,50-(pcY/2), pcY, giroLog->color565(0,0,255));
    giroLog->drawFastVLine(4,50-(pcZ/2), pcZ, giroLog->color565(255,0,0));
    */
   giroLog->drawFastVLine(0,0, giroLog->height(), giroLog->color565(64,64,64));
    for(int p=0;p<giroLog->height();p+=6) {
       giroLog->drawFastHLine(0,p,6,giroLog->color565(64,64,64));
    }

   giroLog->drawLine(0,pcX, 7, lWpcX, giroLog->color565(0,255,0));
   giroLog->drawLine(0,pcY, 7, lWpcY, giroLog->color565(0,0,255));
   giroLog->drawLine(0,pcZ, 7, lWpcZ, giroLog->color565(255,0,0));

   giroLog->drawPixel(0,pcX,TFT_WHITE);
   giroLog->drawPixel(0,pcY,TFT_WHITE);
   giroLog->drawPixel(0,pcZ,TFT_WHITE);

   lWpcX = pcX;
   lWpcY = pcY;
   lWpcZ = pcZ;
    //giroLog->drawPixel(0, pcX, TFT_WHITE);
    /*
    giroLog->drawFastVLine(0,50-(pcX/2), pcX, TFT_GREEN);
    giroLog->drawFastVLine(2,50-(pcY/2), pcY, TFT_GREEN);
    giroLog->drawFastVLine(4,50-(pcZ/2), pcZ, TFT_GREEN);
    */
    for(int i=0;i<6;i++) {
        giroLog->drawFastVLine(i,0,5, activityColor);
        giroLog->drawFastVLine(i,giroLog->height()-5,5, activityColor);
    }

    if ( nextRedraw < millis() ) {
        char stepsString[128] = { 0 };
        this->GetCanvas()->fillSprite(TFT_BLACK);
        this->GetCanvas()->setPivot(0,TFT_HEIGHT-100);
        this->GetCanvas()->fillRect(0,TFT_WIDTH-100,TFT_HEIGHT,100, TFT_BLACK);
        giroLog->setPivot(0,0);
        giroLog->pushRotated(this->GetCanvas(),0,TFT_BLACK);
        this->GetCanvas()->setPivot(TFT_WIDTH/2,TFT_HEIGHT/2);

        this->GetCanvas()->setTextSize(1);
        this->GetCanvas()->setFreeFont(&FreeMonoBold12pt7b);
        this->GetCanvas()->setTextDatum(TL_DATUM);
        this->GetCanvas()->setTextWrap(false,false);
        int32_t posX = 10;
        int32_t posY = 10;
        sprintf(stepsString,"Steps: %u", stepCount );
        this->GetCanvas()->setTextColor(TFT_WHITE);
        this->GetCanvas()->drawString(stepsString, posX, posY);

        float methers = (stepCount*stepDistanceCm)/100.0;
        if ( methers < 1000 ) {
            sprintf(stepsString,"Dist: %.2f m", methers );
        } else {
            sprintf(stepsString,"Dist: %.2f Km", float(methers/1000.0) );
        }
        this->GetCanvas()->setTextColor(TFT_WHITE);
        this->GetCanvas()->drawString(stepsString, posX, posY+20);


        if ( nullptr != currentActivity ) {
            if ( 0 == strcmp("BMA423_USER_WALKING",currentActivity)) {
                this->GetCanvas()->drawXBitmap(10,60, img_activity_walking_bits, img_activity_walking_width, img_activity_walking_height, TFT_WHITE);
            } else if ( 0 == strcmp("BMA423_USER_RUNNING",currentActivity)) {
                this->GetCanvas()->drawXBitmap(10,60, img_activity_running_bits, img_activity_running_width, img_activity_running_height, TFT_WHITE);
            } else {
                this->GetCanvas()->drawXBitmap(10,60, img_activity_stay_bits, img_activity_stay_width, img_activity_stay_height, TFT_WHITE);
            }
        }


        this->GetCanvas()->setTextSize(1);
        this->GetCanvas()->setFreeFont(&FreeMonoBold9pt7b);
        this->GetCanvas()->setTextDatum(TL_DATUM);
        this->GetCanvas()->setTextWrap(false,false);
        this->GetCanvas()->setTextColor(TFT_WHITE);

        const int32_t Radius = 30;
        const int16_t x = 170;
        const int16_t y = 100;

        this->GetCanvas()->fillCircle(x,y,Radius,TFT_BLACK);
        
        this->GetCanvas()->fillEllipse(x,y,pcY*float(Radius/100.0),Radius, giroLog->color565(0,0,64));
        this->GetCanvas()->fillEllipse(x,y,Radius,pcX*float(Radius/100.0), giroLog->color565(0,64,0));
        this->GetCanvas()->drawEllipse(x,y,Radius,pcX*float(Radius/100.0), giroLog->color565(0,255,0));
        this->GetCanvas()->drawEllipse(x,y,pcY*float(Radius/100.0),Radius, giroLog->color565(0,0,255));
        //this->GetCanvas()->drawEllipse(x,y,pcZ*float(Radius/100.0),pcZ*float(Radius/100.0), giroLog->color565(255,0,0));



        this->GetCanvas()->drawCircle(x,y,Radius,TFT_WHITE);

        sprintf(stepsString,"X: %.2f", degX );
        this->GetCanvas()->drawString(stepsString, 10, (TFT_WIDTH-100)+pcX-20);
        sprintf(stepsString,"Y: %.2f", degY );
        this->GetCanvas()->drawString(stepsString, 95, (TFT_WIDTH-100)+pcY-20);
        sprintf(stepsString,"Z: %.2f", degZ );
        this->GetCanvas()->drawString(stepsString, 170, (TFT_WIDTH-100)+pcZ-20);

        unsigned long timeFromBoot = millis()/1000;
        this->GetCanvas()->setTextDatum(BR_DATUM);
        sprintf(stepsString,"Time: %lu s", timeFromBoot );
        this->GetCanvas()->drawString(stepsString, TFT_WIDTH, TFT_HEIGHT);

        /*
        angle+=3;
        size_t offset = 0;
        for(int y=20;y<TFT_WIDTH;y+=20) {
            for(int x=20;x<TFT_HEIGHT;x+=20) {
                if ( offset == counter ) {
                    this->GetCanvas()->fillCircle(x,y,10,TFT_YELLOW);
                } else {
                    //this->GetCanvas()->fillCircle(x,y,10,TFT_BLACK);
                    this->GetCanvas()->drawCircle(x,y,5,TFT_YELLOW);

                }
                offset++;
            }
        }
        counter++;
        if (counter > offset ) { counter = 0; }

        int16_t x,y;
        bool touch = ttgo->getTouch(x,y);
        if ( touch ) {
            this->GetCanvas()->fillRect(x-40,y-40,80,80, TFT_GREEN);
        }
        */
        nextRedraw = millis()+80;
        return true;
    }
    return false;
}
