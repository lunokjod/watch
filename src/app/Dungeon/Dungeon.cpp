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
#include "Dungeon.hpp"
#include <libraries/TFT_eSPI/TFT_eSPI.h>

//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include "../LogView.hpp"   // for lLog functions
#include <WiFi.h>

#include "../../../static/Dungeon/img_lava.c"

#include "../../../static/Dungeon/img_splash_dungeon.c" // splash
#include "../../../static/Dungeon/img_error_map.c"
#include "../../../static/Dungeon/img_palette_test.c"

//extern const uint32_t DungeonTileSetsSize;
extern const unsigned char * DungeonTileSets[];

//extern const unsigned char * DungeonTileSets[];
//extern const uint32_t DungeonTileSetsSize;

extern void DungeonLevelGenerator(void *data);

DungeonGameApplication::~DungeonGameApplication() {
    if ( currentLevel.valid ) {
        currentLevel.floorMap->deleteSprite();
        delete currentLevel.floorMap;
        currentLevel.wallMap->deleteSprite();
        delete currentLevel.wallMap;
        currentLevel.topMap->deleteSprite();
        delete currentLevel.topMap;
        currentLevel.objectsMap->deleteSprite();
        delete currentLevel.objectsMap;
        currentLevel.darknessMap->deleteSprite();
        delete currentLevel.darknessMap;
        delete currentLevel.player;
    }
    /*
    if ((NULL != MapGeneratorHandle) && ( currentLevel.running )) {
        lAppLog("Killing map generator....\n");
        vTaskDelete(MapGeneratorHandle);
        MapGeneratorHandle=NULL;
    }
    */
    delete gameScreen;
    delete floorLayer;
    delete objectLayer;
    delete wallLayer;
    delete topLayer;

    if ( nullptr != dragScreenCopy ) {
        dragScreenCopy->deleteSprite();
        delete dragScreenCopy;
        dragScreenCopy=nullptr;
    }
    WiFi.mode(WIFI_MODE_NULL);

    //delete lavaBackground;
    //delete shadowLayer;
    directDraw=false;
}

DungeonGameApplication::DungeonGameApplication() {
    directDraw=true; //don't want automatic refresh

    PoToCXMap=0; // points to last processed coordinate X of map 
    PoToCYMap=0; // points to last processed coordinate Y of map 

    randomSeed(analogRead(0)); // need random values
    WiFi.mode(WIFI_MODE_STA); // random numbers uses wifi to get seeds
    
    gameScreen=new CanvasZWidget(TFT_HEIGHT/NormalScale,TFT_WIDTH/NormalScale);
    objectLayer=new CanvasZWidget(TFT_HEIGHT/NormalScale,TFT_WIDTH/NormalScale);
    floorLayer=new CanvasZWidget(TFT_HEIGHT/NormalScale,TFT_WIDTH/NormalScale);
    wallLayer=new CanvasZWidget(TFT_HEIGHT/NormalScale,TFT_WIDTH/NormalScale);
    topLayer=new CanvasZWidget(TFT_HEIGHT/NormalScale,TFT_WIDTH/NormalScale);

    gameScreen->canvas->fillSprite(TFT_BLACK);
    
    // launch map generator meanwhile the app is loading
    currentLevel.valid=false;
    loadedSpawnPoint=false;
    lAppLog("Starting map generator...\n");
    xTaskCreatePinnedToCore(DungeonLevelGenerator, "", LUNOKIOT_TASK_STACK_SIZE, &currentLevel, uxTaskPriorityGet(NULL), &MapGeneratorHandle,1);

    // use canvas as splash screen
    // https://www.fontspace.com/category/medieval
    canvas->setSwapBytes(true);
    canvas->pushImage(0,0,img_splash_dungeon.width,img_splash_dungeon.height, (uint16_t *)img_splash_dungeon.pixel_data);
//    canvas->pushImage(0,0,img_palette_test.width,img_palette_test.height, (uint16_t *)img_palette_test.pixel_data);
    canvas->setSwapBytes(false);



    tft->setTextColor(TFT_WHITE,tft->color24to16(0x191919));
    tft->setTextDatum(BC_DATUM);
    tft->setFreeFont(&FreeSerif12pt7b);
    tft->setTextSize(1);
    //lavaBackground->canvas->pushImage(0,0,16,16,(uint16_t *)img_lava.pixel_data);
}
void DungeonGameApplication::ManageSplashScreen() {

    if (false == currentLevel.valid ) {
        if ( 0 == waitGeneratorTimeout ) {
            waitGeneratorTimestamp=0;
            waitGeneratorTimeout=millis()+(25*1000);
        } else {
            if ( millis() > waitGeneratorTimeout ) {
                lAppLog("Map generator timeout, retrying...\n");
                /* @TODO convert this in less invasive
                tft->fillScreen(TFT_WHITE);
                tft->setSwapBytes(true);
                tft->pushImage(19,42,img_error_map.width,img_error_map.height, (uint16_t *)img_error_map.pixel_data);
                tft->setSwapBytes(false);
                */
                vTaskDelete(MapGeneratorHandle);
                MapGeneratorHandle=NULL;
                currentLevel.running=false;
                loadedSpawnPoint=false;

                lAppLog("Starting map generator (retry %d)...\n",generationRetries);
                generationRetries++;
                if ( generationRetries > 5) {
                    currentLevel.running=false;
                    lAppLog("Unable to generate coherent level with this paramethers aborting launch!\n");
                    LaunchWatchface();
                }
                this->canvas->pushSprite(0,0); 
                xTaskCreatePinnedToCore(DungeonLevelGenerator, "", LUNOKIOT_APP_STACK_SIZE, &currentLevel, uxTaskPriorityGet(NULL), &MapGeneratorHandle,1);
                waitGeneratorTimeout=0;
                return;
            } else if ( millis() > waitGeneratorTimestamp ) {
                // loop all loading messages
                if ( currentSentence >= (sizeof(LoadSentences)/sizeof(LoadSentences[0]) ) ) { currentSentence=0; }
                NoticesBanner(LoadSentences[currentSentence]);
                waitGeneratorTimestamp=millis()+1800;
                currentSentence++;
            }
            /*
            // show dungeon generation layers
            canvas->fillRect(0,0,currentLevel.width,currentLevel.height,TFT_WHITE); // map generator background
            tft->fillRect(0,0,currentLevel.width,currentLevel.height,TFT_WHITE); // map generator background
            currentLevel.floorMap->pushSprite(0,0,255);
            currentLevel.objectsMap->pushSprite(0,0,255);
            currentLevel.wallMap->pushSprite(0,0,255);
            currentLevel.topMap->pushSprite(0,0,255);
            */
        }
    }
}

bool DungeonGameApplication::Tick() {
    // waiting for the scenery building
    if ( false == currentLevel.running ) {
        ManageSplashScreen();
        return false;
    }
    if ( false == zeroSet ) { // set the zero
        zeroDegX = degX;
        zeroDegY = degY;
        zeroDegZ = degZ;
        zeroSet = true;
        return false;
    }
    if ( false == loadedSpawnPoint ) {
        //offsetX=((currentLevel.PlayerBeginX*tileW)*NormalScale);
        //offsetY=((currentLevel.PlayerBeginY*tileH)*NormalScale);
        offsetX=((currentLevel.PlayerBeginX*tileW)*NormalScale)*-1;
        offsetY=((currentLevel.PlayerBeginY*tileH)*NormalScale)*-1;
        tft->fillScreen(TFT_BLACK);
        loadedSpawnPoint=true;
        lAppLog("User location X: %d Y: %d px: %d py: %d\n",currentLevel.PlayerBeginX,currentLevel.PlayerBeginY,offsetX,offsetY);
        dirty=true;
        //Redraw();
        //return false;
    }
    if ( touched ) {
        animationTimeout=millis()+renderTime; // don't redraw meanwhile drag
        
        if ( false == lastThumb) {
            lastThumb=true;
            //lEvLog("Thumb IN X: %d Y: %d\n",touchX,touchY);
            // do something at touch BEGIN
            //int32_t calculatedX = touchDragVectorX+((TFT_WIDTH/2)-((gameScreen->canvas->width()/2)));
            //int32_t calculatedY = touchDragVectorY+((TFT_HEIGHT/2)-((gameScreen->canvas->height()/2)));
            //tft->setPivot(0,0);
            //gameScreen->canvas->setPivot(0,0);
            //gameScreen->DrawTo(0,0,0.7,true);

            zeroSet=false; // force to reset the new ground zero
            return false;
        } else {
            // draw MEANWHILE touch
            int32_t calculatedX = touchDragVectorX+((TFT_WIDTH/2)-((gameScreen->canvas->width()*NormalScale)/2));
            int32_t calculatedY = touchDragVectorY+((TFT_HEIGHT/2)-((gameScreen->canvas->height()*NormalScale)/2));
            int32_t calculatedX2 = calculatedX+gameScreen->canvas->width()*NormalScale;
            int32_t calculatedY2 = calculatedY+gameScreen->canvas->height()*NormalScale;
            //lEvLog("UI: Continuous touch X: %d Y: %d CX: %d CY: %d \n",touchX,touchY,touchDragVectorX,touchDragVectorY);
            tft->setPivot(0,0);
            gameScreen->canvas->setPivot(0,0);
            
            gameScreen->DrawTo(calculatedX,calculatedY,NormalScale,false);
            // ERASERS
            // spiral design to divide the effort of refresh
            tft->fillRect(calculatedX,0,TFT_WIDTH-calculatedX,calculatedY,TFT_BLACK); // up eraser
            tft->fillRect(0,0,calculatedX,calculatedY2,TFT_BLACK); // left window
            tft->fillRect(calculatedX2,calculatedY,TFT_WIDTH-calculatedX2,TFT_HEIGHT-calculatedY,TFT_BLACK); // right window
            tft->fillRect(0,calculatedY2,calculatedX2,TFT_HEIGHT-calculatedY2,TFT_BLACK); // down window

            return false;
        }
    } else {
        if (lastThumb) {
            lastThumb=false;
            // do something at touch END
            offsetX+=touchDragVectorX;
            offsetY+=touchDragVectorY;

            //int32_t calculatedX = ((TFT_WIDTH/2)-((gameScreen->canvas->width()/2)));
            //int32_t calculatedY = ((TFT_HEIGHT/2)-((gameScreen->canvas->height()/2)));

            // maybe you want to orient the next render 
            PoToCXMap=0;
            PoToCYMap=0;

            tft->setPivot(0,0);
            gameScreen->canvas->setPivot(0,0);
            //gameScreen->DirectDraw(calculatedX,calculatedY);
            //lEvLog("Thumb LEAVE X: %d Y: %d OffsetX: %d OffsetY: %d VecX: %d VecY: %d\n",touchX,touchY,offsetX,offsetY,touchDragVectorX,touchDragVectorY);
            // clean visible map
            //floorLayer->canvas->fillSprite(TFT_BLACK);
            //gameScreen->DirectDraw(0,0);
            dirty=true;
            //Redraw();
            //return false;
        }
    }

    if ( millis() > animationTimeout ) {
        dirty = true;
        //Redraw();
        animationTimeout=millis()+(1000/8); //(renderTime*1.25);
    }
    if ( dirty ) {
        Redraw();
    }
    return false;
}


void DungeonGameApplication::Redraw() {
    unsigned long beginMS=millis();
    int32_t disX=(offsetX%int(tileW*NormalScale));
    int32_t disY=(offsetY%int(tileH*NormalScale));
    int16_t maxOffsetX =abs((currentLevel.width*(tileW*NormalScale))-floorLayer->canvas->width());
    int16_t maxOffsetY =abs((currentLevel.height*(tileH*NormalScale))-floorLayer->canvas->height());

    int32_t tileOffsetX=(offsetX/(tileW*NormalScale))*-1;
    int32_t tileOffsetY=(offsetY/(tileH*NormalScale))*-1;

    if ( tileOffsetX < 0 ) {
        //Serial.println("Out of map -X!!!");
        offsetX=0;
        //gameScreen->DirectDraw(0,0);
        //Redraw();
        return;
    } else if (abs(offsetX) > maxOffsetX ) {
        //Serial.println("Out of map +X!!!");
        offsetX=(maxOffsetX-1)*-1;
        //gameScreen->DirectDraw(0,0);
        //Redraw();
        return;
    }
    if ( tileOffsetY < 0 ) {
        //Serial.println("Out of map -Y!!!");
        offsetY=0;
        //gameScreen->DirectDraw(0,0);
        //Redraw();
        return;
    } else if (abs(offsetY) > maxOffsetY ) {
        //Serial.println("Out of map +Y!!!");
        offsetY=(maxOffsetY-1)*-1;            
        //gameScreen->DirectDraw(0,0);
        //Redraw();
        return;
    }
    if ( dirty ) {
        gameScreen->canvas->fillSprite(TFT_BLACK);
        floorLayer->canvas->fillSprite(TFT_BLACK);
        objectLayer->canvas->fillSprite(TFT_BLACK);
        wallLayer->canvas->fillSprite(TFT_BLACK);
        topLayer->canvas->fillSprite(TFT_BLACK);
    }

    //Serial.printf("Current tile offset X: %d (%d MAX: %d) Y: %d (%d MAX: %d)\n",tileOffsetX,offsetX,maxOffsetX,tileOffsetY,offsetY,maxOffsetY);
    //Serial.printf("Current tile offset X: %d Y: %d DisX: %d DisY: %d\n",tileOffsetX,tileOffsetY,disX,disY);
    //for(int16_t y=disY;y<=floorLayer->canvas->width()+(tileW*NormalScale);y+=(tileW*NormalScale)) {
    //    for(int16_t x=disX;x<=floorLayer->canvas->height()+(tileH*NormalScale);x+=(tileH*NormalScale)) {
    for(int16_t y=0;y<=floorLayer->canvas->height()+(tileH*2);y+=(tileH)) {
        for(int16_t x=0;x<=floorLayer->canvas->width()+(tileW*2);x+=(tileW)) {
            int16_t tileX = (x/(tileW))+tileOffsetX;
            int16_t tileY = (y/(tileH))+tileOffsetY;
            if ( tileX < 0 ) { break; }
            if ( tileY < 0 ) { break; }
            if ( tileX > currentLevel.width ) { break; }
            if ( tileY > currentLevel.height ) { break; }

            uint16_t floorColor = currentLevel.floorMap->readPixel(tileX,tileY);
            if ( ( 255 != floorColor) && ( 0xFFFF != floorColor) ) {
                bool animated=false;
                // floor sprite rotation
                if ( 52 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,53); animated=true; }
                else if ( 53 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,54); animated=true; }
                else if ( 54 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,52); animated=true; }
                
                if ( 58 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,59); animated=true; }
                else if ( 59 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,60); animated=true; }
                else if ( 60 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,58); animated=true; }
                
                if ( 29 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,30); animated=true; }
                else if ( 30 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,31); animated=true; }
                else if ( 31 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,32); animated=true; }
                else if ( 32 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,29); animated=true; }
                floorColor = currentLevel.floorMap->readPixel(tileX,tileY);
                if (( dirty )||( animated )) {
                    //Serial.printf("tX: %d tY: %d\n",tileX,tileY);
                    const unsigned char *ptr=DungeonTileSets[floorColor]; // current tile ptr            
                    CanvasZWidget *tempBuffer = new CanvasZWidget(tileH,tileW);
                    tempBuffer->canvas->setSwapBytes(true);
                    tempBuffer->canvas->pushImage(0,0,tileW,tileH,(uint16_t *)ptr);
                    floorLayer->canvas->setPivot(0,0); // to set the incoming title
                    tempBuffer->canvas->setPivot(0,0);
                    if ( dirty ) { // dump to buffer
                        directDraw=false;
                        tempBuffer->DrawTo(floorLayer->canvas,x,y);
                        directDraw=true;
                    } else if ( animated ) { // direct update
                        tempBuffer->DrawTo(x*NormalScale,y*NormalScale,NormalScale);
                    }
                    delete tempBuffer;
                }
            }

            if ( dirty ) {
                // objects
                uint16_t objectColor = currentLevel.objectsMap->readPixel(tileX,tileY);
                if ( ( 255 != objectColor) && ( 0xFFFF != objectColor) ) {
                    //Serial.printf("tX: %d tY: %d\n",tileX,tileY);
                    const unsigned char *ptr=DungeonTileSets[objectColor]; // current tile ptr            
                    CanvasZWidget *tempBuffer = new CanvasZWidget(tileH,tileW);
                    tempBuffer->canvas->setSwapBytes(true);
                    tempBuffer->canvas->pushImage(0,0,tileW,tileH,(uint16_t *)ptr);
                    objectLayer->canvas->setPivot(0,0); // to set the incoming title
                    tempBuffer->canvas->setPivot(0,0);
                    // dump current
                    directDraw=false;
                    tempBuffer->DrawTo(objectLayer->canvas,x,y,1.0,false,TFT_BLACK);
                    directDraw=true;
                    delete tempBuffer;
                }
            }

            uint16_t wallColor = currentLevel.wallMap->readPixel(tileX,tileY);
            if ( ( 255 != wallColor) && ( 0xFFFF != wallColor) ) {
                bool animated=false;
                // walls anim
                if ( 55 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,56); animated=true; }
                else if ( 56 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,57); animated=true; }
                else if ( 57 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,55);animated=true;  }                    
                if ( 61 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,62); animated=true; }
                else if ( 62 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,63); animated=true; }
                else if ( 63 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,61); animated=true; }
                wallColor = currentLevel.wallMap->readPixel(tileX,tileY);
                if ( ( dirty )||( animated )) {    
                    //Serial.printf("tX: %d tY: %d\n",tileX,tileY);
                    const unsigned char *ptr=DungeonTileSets[wallColor]; // current tile ptr            
                    CanvasZWidget *tempBuffer = new CanvasZWidget(tileH,tileW);
                    tempBuffer->canvas->setSwapBytes(true);
                    tempBuffer->canvas->pushImage(0,0,tileW,tileH,(uint16_t *)ptr);
                    wallLayer->canvas->setPivot(0,0); // to set the incoming title
                    tempBuffer->canvas->setPivot(0,0);

                    // dump current
                    if ( dirty ) { // dump to buffer
                        directDraw=false;
                        tempBuffer->DrawTo(wallLayer->canvas,x,y,1.0,false,TFT_BLACK);
                        directDraw=true;
                    } else if ( animated ) { // direct update
                        tempBuffer->DrawTo(x*NormalScale,y*NormalScale,NormalScale,false,TFT_BLACK);
                    }
                    delete tempBuffer;
                }

            }
            uint16_t objectColor = currentLevel.objectsMap->readPixel(tileX,tileY);
            if ( ( 255 != objectColor) && ( 0xFFFF != objectColor) ) {
                bool animated=false;
                // coin?
                if ( 71 ==  objectColor ) { currentLevel.objectsMap->drawPixel(tileX,tileY,72); animated=true; }
                else if ( 72 ==  objectColor ) { currentLevel.objectsMap->drawPixel(tileX,tileY,73); animated=true; }
                else if ( 73 ==  objectColor ) { currentLevel.objectsMap->drawPixel(tileX,tileY,74); animated=true; }
                else if ( 74 ==  objectColor ) { currentLevel.objectsMap->drawPixel(tileX,tileY,71); animated=true; }
                if ( ( dirty )||( animated )) {    
                    //Serial.printf("tX: %d tY: %d\n",tileX,tileY);
                    const unsigned char *ptr=DungeonTileSets[objectColor]; // current tile ptr            
                    CanvasZWidget *tempBuffer = new CanvasZWidget(tileH,tileW);
                    tempBuffer->canvas->setSwapBytes(true);
                    tempBuffer->canvas->pushImage(0,0,tileH,tileW,(uint16_t *)ptr);
                    objectLayer->canvas->setPivot(0,0); // to set the incoming title
                    tempBuffer->canvas->setPivot(0,0);

                    // dump current
                    if ( dirty ) { // dump to buffer
                        directDraw=false;
                        tempBuffer->DrawTo(objectLayer->canvas,x,y,1.0,false,TFT_BLACK);
                        directDraw=true;
                    } else if ( animated ) { // direct update
                        tempBuffer->DrawTo(x*NormalScale,y*NormalScale,NormalScale,false,TFT_BLACK);
                    }
                    delete tempBuffer;
                }
            }

            if ( dirty ) {
                // top
                uint16_t topColor = currentLevel.topMap->readPixel(tileX,tileY);
                if ( ( 255 != topColor) && ( 0xFFFF != topColor) ) {
                    //Serial.printf("tX: %d tY: %d\n",tileX,tileY);
                    const unsigned char *ptr=DungeonTileSets[topColor]; // current tile ptr            
                    CanvasZWidget *tempBuffer = new CanvasZWidget(tileH,tileW);
                    tempBuffer->canvas->setSwapBytes(true);
                    tempBuffer->canvas->pushImage(0,0,tileW,tileH,(uint16_t *)ptr);
                    topLayer->canvas->setPivot(0,0); // to set the incoming title
                    tempBuffer->canvas->setPivot(0,0);
                    // dump current
                    directDraw=false;
                    tempBuffer->DrawTo(topLayer->canvas,x,y,1.0,false,TFT_BLACK);
                    directDraw=true;
                    delete tempBuffer;
                }
            }
        }
    }
    //int32_t calculatedX = touchDragVectorX+((TFT_WIDTH/2)-((gameScreen->canvas->width()/2)));
    //int32_t calculatedY = touchDragVectorY+((TFT_HEIGHT/2)-((gameScreen->canvas->height()/2)));
    //gameScreen->DrawTo(calculatedX,calculatedY,NormalScale,false);
    //gameScreen->DrawTo(TFT_WIDTH/2,TFT_HEIGHT/2,NormalScale,true);
    if ( dirty ) {
        directDraw=false;
        floorLayer->DrawTo(gameScreen->canvas,0,0,1.0,false,TFT_BLACK);
        objectLayer->DrawTo(gameScreen->canvas,0,0,1.0,false,TFT_BLACK);
        wallLayer->DrawTo(gameScreen->canvas,0,0,1.0,false,TFT_BLACK);
        topLayer->DrawTo(gameScreen->canvas,0,0,1.0,false,TFT_BLACK);
        directDraw=true;
        //if (nullptr == lastGameScreen ) {
            gameScreen->DrawTo(0,0,NormalScale,false);
        /*} else {
            for(int16_t y=0;y<=lastGameScreen->height();y++) {
                for(int16_t x=0;x<=lastGameScreen->width();x++) {
                    uint16_t lastColor = lastGameScreen->readPixel(x,y);
                    uint16_t newColor = gameScreen->canvas->readPixel(x,y);
                    if ( lastColor != newColor ) {
                        //tft->fillRect(x*(tileW*NormalScale),y*(tileH*NormalScale),tileW,tileH,newColor);
                        //tft->fillRect(x*NormalScale,y*NormalScale,tileW/NormalScale,tileH/NormalScale,TFT_GREEN);
                        // usefull to debug draw
                        tft->drawPixel(x*NormalScale,y*NormalScale,TFT_GREEN);
                        //tft->drawRect(x*NormalScale,y*NormalScale,NormalScale,NormalScale,TFT_GREEN);
                    }
                }
            }
        }

        if (nullptr != lastGameScreen ) {
            lastGameScreen->deleteSprite();
            delete lastGameScreen;
        }
        lastGameScreen=ScaleSprite(gameScreen->canvas,1.0);*/
        dirty=false;
    }
    /* GRID
    //Serial.printf("DisX: %d DisY: %d\n",disX,disY);
    for(int16_t y=0;y<=(floorLayer->canvas->width())*NormalScale;y+=(tileH*NormalScale)) {
        for(int16_t x=0;x<=(floorLayer->canvas->height())*NormalScale;x+=(tileW*NormalScale)) {
            tft->drawRect(x,y,tileW*NormalScale,tileH*NormalScale,TFT_YELLOW);
        }
    }*/

    unsigned long endMS=millis();
    renderTime=(endMS-beginMS);
    //Serial.printf("Time: %u\n", renderTime);
}


void DungeonGameApplication::NoticesBanner(const char *what) {
    if ( false == currentLevel.running ) {
        tft->fillRect(0,TFT_HEIGHT-40,TFT_WIDTH,40,tft->color24to16(0x191919));
        tft->drawString(what, TFT_WIDTH/2,TFT_HEIGHT-10);
    }
}
