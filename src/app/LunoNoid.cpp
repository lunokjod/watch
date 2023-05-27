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
#include "LunoNoid.hpp"
#include <LilyGoWatch.h>
//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include "LogView.hpp"   // for lLog functions
#include <esp_task_wdt.h>
extern SemaphoreHandle_t I2cMutex;
extern TTGOClass *ttgo;
#include "../static/img_nanonoid_level69.c"
#include "../static/img_nanonoid_level4.c"
#include "../static/img_nanonoid_level3.c"
#include "../static/img_nanonoid_level2.c"
#include "../static/img_nanonoid_level1.c"
#include "../static/img_nanonoid_level0.c"

const uint16_t * LunoNoidLevelData[] = {
    (uint16_t*)img_nanonoid_level0.pixel_data,
    (uint16_t*)img_nanonoid_level1.pixel_data,
    (uint16_t*)img_nanonoid_level2.pixel_data,
    (uint16_t*)img_nanonoid_level3.pixel_data,
    (uint16_t*)img_nanonoid_level4.pixel_data,
    (uint16_t*)img_nanonoid_level69.pixel_data
};

LunoNoidGameApplication::~LunoNoidGameApplication() {
    if ( nullptr != enemyMap ) {
        if ( enemyMap->created() ) { enemyMap->deleteSprite(); }
        delete enemyMap;
        enemyMap=nullptr;
    }
}

LunoNoidGameApplication::LunoNoidGameApplication() {
    directDraw=true; //don't want automatic refresh
    enemyMap = new TFT_eSprite(tft);
    enemyMap->setColorDepth(8);
    enemyMap->createSprite(6,10);


    /* RANDOM MAP
    enemyMap->fillSprite(TFT_BLACK);
    for(int16_t y=0;y<enemyMap->height();y++) {
        for(int16_t x=0;x<enemyMap->width();x++) {
            enemyMap->drawPixel(x,y,random(0,65535));
        }
    }*/
   // load bitmap as level
   enemyMap->pushImage(0,0,6,6,LunoNoidLevelData[level]);
   GatherButtons(); // calibration
}

void LunoNoidGameApplication::GatherButtons() {
    // launch ball on touch
    if ( touched ) { stickyBall=false; }
    // get accel dta
    Accel acc;
    BaseType_t done = xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_FAST_TIME_TICKS);
    if (pdTRUE != done) { return; }
    bool res = ttgo->bma->getAccel(acc);
    xSemaphoreGive(I2cMutex);
    rollval = atan2(acc.x,acc.z) * RAD_TO_DEG;
    pitchval = atan2(acc.y, acc.z) * RAD_TO_DEG;
    if ( 0 > rollval ) { rollval+=360; } // dirty module
    else if ( rollval > 360 ) { rollval-=360; }
    if ( 0 > pitchval ) { pitchval+=360; } // dirty module
    else if ( pitchval > 360 ) { pitchval-=360; }
    // determine 0 point (used in subsecuent calls to get the diff)
    if ( false == groundSet ) {
        refRollval=rollval;
        refPitchval=pitchval;
        groundSet=true;
    }
    // get the diff from "zero"
    if ( groundSet ) {
        rollval-=refRollval;
        pitchval-=refPitchval;
    }
    if ( 0 > rollval ) { rollval+=360; } // dirty module
    else if ( rollval > 360 ) { rollval-=360; }
    if ( 0 > pitchval ) { pitchval+=360; } // dirty module
    else if ( pitchval > 360 ) { pitchval-=360; }
    //lLog("PITCH: %f ROLL: %f\n",pitchval,rollval);
}

bool LunoNoidGameApplication::Tick() {
    if ( millis() < nextRefresh ) { return false; }
    nextRefresh=millis()+(1000/24);
    char buffer[255];
    GatherButtons();
    int16_t ballXCell=ballX/40;
    int16_t ballYCell=(ballY-10)/15; // centered, is a circle
    // draw bricks
    bool almostOne=false;
    BaseType_t done = xSemaphoreTake(UISemaphore, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
    if ( pdTRUE != done ) { return false; }
    
    if ( 1 > credits ) {
        //lLog("GAME OVER\n");
        tft->setTextFont(0);
        tft->setTextColor(TFT_WHITE);
        tft->setTextSize(4);
        tft->setTextDatum(CC_DATUM);
        sprintf(buffer,"GAME OVER");
        tft->drawString(buffer, tft->width()/2, tft->height()/2);
        delay(1000);
        tft->fillScreen(TFT_BLACK);
        enemyMap->fillSprite(TFT_BLACK);
        credits=3;
        level=0;
        enemyMap->pushImage(0,0,6,6,LunoNoidLevelData[level]);
        stickyBall=true;
        groundSet=false; //recalibrate
        GatherButtons();
    }
    for(int16_t y=0;y<enemyMap->height();y++) {
        for(int16_t x=0;x<enemyMap->width();x++) {
            uint32_t color = enemyMap->readPixel(x,y);
            if ( TFT_BLACK != color ) {
                almostOne=true;
                if ( ( ballXCell == x ) && ( ballYCell == y ) ) {
                    // collision!!!
                    color=TFT_BLACK;
                    enemyMap->drawPixel(x,y,color);
                    BallSpeedY*=-1;
                }
            }
            //tft->fillRect(x*40,(y*15)+20,40,15,color);
            tft->fillRoundRect(x*40,(y*15)+20,40,15,5,color);
        }
    }
    if ( false == almostOne ) {
        //lLog("YOU WIN!\n");
        tft->setTextFont(0);
        tft->setTextColor(TFT_WHITE);
        tft->setTextSize(4);
        tft->setTextDatum(CC_DATUM);
        sprintf(buffer,"YOU WIN!");
        tft->drawString(buffer, tft->width()/2, tft->height()/2);
        delay(1000);
        tft->fillScreen(TFT_BLACK);
        credits+=1;
        level++;
        if ( level > 5 ) { level = 5; }
        enemyMap->fillSprite(TFT_BLACK);
        enemyMap->pushImage(0,0,6,6,LunoNoidLevelData[level]);
        stickyBall=true;
        groundSet=false; //recalibrate
        GatherButtons();
    }
    // clean last frame
    tft->fillRect(playerPosition-(BarLenght/2),canvas->height()-30,BarLenght,5,TFT_BLACK);
    tft->fillCircle(ballX,ballY+20,5,TFT_BLACK);
    // player movement
    if ( pitchval > 180 ) { playerPosition-=7; }
    else if ( pitchval < 180 ) { playerPosition+=7; }
    // player limits
    if ( playerPosition < 0 ) { playerPosition=0; }
    else if ( playerPosition > canvas->width() ) { playerPosition=canvas->width(); }
    if ( stickyBall ) {
        ballX=playerPosition;
        ballY=canvas->height()-32-20;
    } else { // ball step
        ballX+=BallSpeedX;
        ballY+=BallSpeedY;
    }
    // rebound with walls
    if (( ballX < 0 )||( ballX > canvas->width() )) { BallSpeedX*=-1; }    
    if ( ballY+20 < 0 ) { BallSpeedY*=-1; }
    if ( ballY+20 > canvas->height() ) {
        // DEBUG: BallSpeedY*=-1;
        ballX = playerPosition;
        ballY = canvas->height()-32-20;
        stickyBall=true;
        credits--;
        if ( credits < 0 ) { credits=0; }
    }
    // player rebound
    if (( ballY+20 >= (canvas->height()-30) )&&( ballY+20 <= (canvas->height()-25) )) {
        if ( ( ballX > playerPosition-(BarLenght/2) ) && ( ballX < playerPosition+(BarLenght/2) )) {
            BallSpeedY*=-1;
        }
    }
    // draw player and ball
    tft->fillRect(playerPosition-(BarLenght/2),canvas->height()-30,BarLenght,5,TFT_GREEN);
    tft->fillCircle(ballX,ballY+20,5,TFT_WHITE);

    tft->setTextFont(0);
    tft->setTextColor(TFT_WHITE,TFT_BLACK);
    tft->setTextSize(2);
    tft->setTextDatum(TL_DATUM);
    sprintf(buffer,"Level: %u  ", level );
    tft->drawString(buffer, 0, 0);
    tft->setTextDatum(TR_DATUM);
    sprintf(buffer,"  Remain: %d", credits );
    tft->drawString(buffer, tft->width(), 0);
    xSemaphoreGive(UISemaphore);
    return false;
}
