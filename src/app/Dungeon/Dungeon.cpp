#include "Dungeon.hpp"
#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include "../LogView.hpp"   // for lLog functions
#include "../Watchface.hpp"
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
    lAppLog("Starting map generator...\n");
    xTaskCreate(DungeonLevelGenerator, "", LUNOKIOT_TASK_STACK_SIZE, &currentLevel, uxTaskPriorityGet(NULL), &MapGeneratorHandle);

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
            waitGeneratorTimeout=millis()+(30*1000);
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
                lAppLog("Starting map generator (retry %d)...\n",generationRetries);
                generationRetries++;
                if ( generationRetries > 5) {
                    currentLevel.running=false;
                    lAppLog("Unable to generate coherent level with this paramethers aborting launch!\n");
                    LaunchApplication(new WatchfaceApplication());
                }
                this->canvas->pushSprite(0,0); 
                xTaskCreate(DungeonLevelGenerator, "", LUNOKIOT_APP_STACK_SIZE, &currentLevel, uxTaskPriorityGet(NULL), &MapGeneratorHandle);
                waitGeneratorTimeout=0;
                return;
            } else if ( millis() > waitGeneratorTimestamp ) {
                // loop all loading messages
                if ( currentSentence >= (sizeof(LoadSentences)/sizeof(LoadSentences[0]) ) ) { currentSentence=0; }
                NoticesBanner(LoadSentences[currentSentence]);
                waitGeneratorTimestamp=millis()+1800;
                currentSentence++;
            }
        }
    } else {

        if ( false == loadedSpawnPoint ) {
            offsetX=(currentLevel.PlayerBeginX*tileW)*-1;
            offsetY=(currentLevel.PlayerBeginY*tileH)*-1;
            tft->fillScreen(TFT_BLACK);
            loadedSpawnPoint=true;
            lAppLog("User location X: %d Y: %d px: %d py: %d\n",currentLevel.PlayerBeginX,currentLevel.PlayerBeginY,offsetX,offsetY);
        }

    }
}

bool DungeonGameApplication::Tick() {
    ManageSplashScreen();
    // waiting for the scenery building
    if ( false == currentLevel.running ) { return false; }
    if ( touched ) {
        animationTimeout=millis()+renderTime; // don't redraw meanwhile drag
        UINextTimeout = millis()+UITimeout; // no sleep if user is actively use
        
        if ( false == lastThumb) {
            lastThumb=true;
            //lEvLog("Thumb IN X: %d Y: %d\n",touchX,touchY);
            // do something at touch BEGIN
            //int32_t calculatedX = touchDragVectorX+((TFT_WIDTH/2)-((gameScreen->canvas->width()/2)));
            //int32_t calculatedY = touchDragVectorY+((TFT_HEIGHT/2)-((gameScreen->canvas->height()/2)));
            //tft->setPivot(0,0);
            //gameScreen->canvas->setPivot(0,0);
            //gameScreen->DrawTo(0,0,0.7,true);
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
            //gameScreen->canvas->pushSprite(touchDragVectorX,touchDragVectorY);
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
            Redraw();
            return false;
        }
    }
    if ( millis() > animationTimeout ) {
        Redraw();
        animationTimeout=millis()+(renderTime*2);
    } else {
        if ( dirty ) {
            Redraw();
        }
    }
    return false;
}


void DungeonGameApplication::Redraw() {
    unsigned long beginMS=millis();
    if (true) {//( dirty ) {
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
        gameScreen->canvas->fillSprite(TFT_BLACK);
        floorLayer->canvas->fillSprite(TFT_BLACK);
        objectLayer->canvas->fillSprite(TFT_BLACK);
        wallLayer->canvas->fillSprite(TFT_BLACK);
        topLayer->canvas->fillSprite(TFT_BLACK);

        //Serial.printf("Current tile offset X: %d (%d MAX: %d) Y: %d (%d MAX: %d)\n",tileOffsetX,offsetX,maxOffsetX,tileOffsetY,offsetY,maxOffsetY);
        //Serial.printf("Current tile offset X: %d Y: %d DisX: %d DisY: %d\n",tileOffsetX,tileOffsetY,disX,disY);
        //for(int16_t y=disY;y<=floorLayer->canvas->width()+(tileW*NormalScale);y+=(tileW*NormalScale)) {
        //    for(int16_t x=disX;x<=floorLayer->canvas->height()+(tileH*NormalScale);x+=(tileH*NormalScale)) {
        for(int16_t y=0;y<=floorLayer->canvas->width()+(tileH*2);y+=(tileH)) {
            for(int16_t x=0;x<=floorLayer->canvas->height()+(tileW*2);x+=(tileW)) {
                int16_t tileX = (x/(tileW))+tileOffsetX;
                int16_t tileY = (y/(tileH))+tileOffsetY;
                if ( tileX < 0 ) { break; }
                if ( tileY < 0 ) { break; }
                if ( tileX > currentLevel.width ) { break; }
                if ( tileY > currentLevel.height ) { break; }

                // floor
                uint16_t floorColor = currentLevel.floorMap->readPixel(tileX,tileY);
                if ( ( 255 != floorColor) && ( 0xFFFF != floorColor) ) {
                    //Serial.printf("tX: %d tY: %d\n",tileX,tileY);
                    const unsigned char *ptr=DungeonTileSets[floorColor]; // current tile ptr            
                    CanvasZWidget *tempBuffer = new CanvasZWidget(tileH,tileW);
                    tempBuffer->canvas->setSwapBytes(true);
                    tempBuffer->canvas->pushImage(0,0,tileW,tileH,(uint16_t *)ptr);
                    floorLayer->canvas->setPivot(0,0); // to set the incoming title
                    tempBuffer->canvas->setPivot(0,0);
                    // dump current
                    directDraw=false;
                    tempBuffer->DrawTo(floorLayer->canvas,x,y);
                    directDraw=true;
                    delete tempBuffer;
                //} else {
                //    floorLayer->canvas->fillRect(x,y,tileW*NormalScale,tileH*NormalScale,TFT_BLACK);
                }

                // floor sprite rotation
                if ( 52 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,53); }
                else if ( 53 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,54); }
                else if ( 54 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,52); }
                
                if ( 58 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,59); }
                else if ( 59 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,60); }
                else if ( 60 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,58); }
                
                if ( 29 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,30); }
                else if ( 30 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,31); }
                else if ( 31 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,32); }
                else if ( 32 ==  floorColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,29); }
                


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


                // wall
                uint16_t wallColor = currentLevel.wallMap->readPixel(tileX,tileY);
                if ( ( 255 != wallColor) && ( 0xFFFF != wallColor) ) {
                    //Serial.printf("tX: %d tY: %d\n",tileX,tileY);
                    const unsigned char *ptr=DungeonTileSets[wallColor]; // current tile ptr            
                    CanvasZWidget *tempBuffer = new CanvasZWidget(tileH,tileW);
                    tempBuffer->canvas->setSwapBytes(true);
                    tempBuffer->canvas->pushImage(0,0,tileW,tileH,(uint16_t *)ptr);
                    wallLayer->canvas->setPivot(0,0); // to set the incoming title
                    tempBuffer->canvas->setPivot(0,0);
                    // dump current
                    directDraw=false;
                    tempBuffer->DrawTo(wallLayer->canvas,x,y,1.0,false,TFT_BLACK);
                    directDraw=true;
                    delete tempBuffer;
                }

                // walls anim
                if ( 52 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,53); }
                else if ( 53 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,54); }
                else if ( 54 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,52); }
                if ( 55 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,56); }
                else if ( 56 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,57); }
                else if ( 57 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,55); }
                if ( 58 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,59); }
                else if ( 59 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,60); }
                else if ( 60 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,58); }
                if ( 29 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,30); }
                else if ( 30 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,31); }
                else if ( 31 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,32); }
                else if ( 32 ==  wallColor ) { currentLevel.wallMap->drawPixel(tileX,tileY,29); }


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
        directDraw=false;
        floorLayer->DrawTo(gameScreen->canvas,0,0,1.0,false,TFT_BLACK);
        objectLayer->DrawTo(gameScreen->canvas,0,0,1.0,false,TFT_BLACK);
        wallLayer->DrawTo(gameScreen->canvas,0,0,1.0,false,TFT_BLACK);
        topLayer->DrawTo(gameScreen->canvas,0,0,1.0,false,TFT_BLACK);
        directDraw=true;
        dirty=false;
    }
    //int32_t calculatedX = touchDragVectorX+((TFT_WIDTH/2)-((gameScreen->canvas->width()/2)));
    //int32_t calculatedY = touchDragVectorY+((TFT_HEIGHT/2)-((gameScreen->canvas->height()/2)));
    //gameScreen->DrawTo(calculatedX,calculatedY,NormalScale,false);
    //gameScreen->DrawTo(TFT_WIDTH/2,TFT_HEIGHT/2,NormalScale,true);
    gameScreen->DrawTo(0,0,NormalScale,false);
    
    unsigned long endMS=millis();
    renderTime=(endMS-beginMS);

    //Serial.printf("Time: %u\n", renderTime);
    return;


    /*
    //const int16_t mapMaxX = composerLayer->canvas->width()*tileW;
    //const int16_t mapMaxY = composerLayer->canvas->height()*tileH;
    // must be inverted
    // max tile numbers

    Serial.printf("Current tile offset X: %d (%d MAX: %d) Y: %d (%d MAX: %d)\n",tileOffsetX,offsetX,maxOffsetX,tileOffsetY,offsetY,maxOffsetY);

    size_t tileCount=0;
    size_t tileMissCount=0;
    size_t tileEmptyCount=0;
    size_t PoToCXMapCurrentCounter=0;   // controls how much interaction is enough per Tick
    for(int16_t y=0;y<=composerLayer->canvas->width();y+=(tileW*NormalScale)) {
        for(int16_t x=0;x<=composerLayer->canvas->height();x+=(tileH*NormalScale)) {
            if ( false == ActiveRect::InRect(tileX,tileY,0,0,currentLevel.width,currentLevel.height)) {
            //if ( ( tileX < 0 )||( tileY < 0 )||( tileX > currentLevel.width )||( tileY > currentLevel.height )) {
                //composerLayer->canvas->fillRect(x,y,tileW,tileH,TFT_YELLOW);
                //continue;
            }
            


            PoToCXMapCurrentCounter++;  // increment until death
            if ( PoToCXMapCurrentCounter >=CNumTileProcess ) {
                //tft->fillCircle(TFT_WIDTH/2,10,5,tft->color24to16(0x56818a)); 
                //lAppLog("Tile processor: Reached limit of tiles processed by cycle (max: %d)\n",CNumTileProcess);
                //goto staphProcessTiles; // :P one of the few situations on the goto is recommended 
            } else {
                //tft->fillCircle(TFT_WIDTH/2,10,5,tft->color24to16(0x353e45));
            }

            PoToCXMap=x; // save the current offset
            PoToCYMap=y;
            //Serial.printf("Tile processor: X: %d Y: %d V: %d Task: %d\n",x,y,tileColor,PoToCXMapCurrentCounter);
            tileCount++;
        }
    }
    PoToCYMap = 0; // reset the work pointers (and ready for next call)
    PoToCXMap = 0;
    //lAppLog("Tile processor: All tiles are looped :)\n");
    staphProcessTiles:  // for exit the double for loop (C don't have multi break like php or python

    // get current position
    int16_t disX=(offsetX%int(tileW*NormalScale));
    int16_t disY=(offsetY%int(tileH*NormalScale));

    // temporal disable directDraw (only want push to canvas)
    gameScreen->canvas->fillSprite(TFT_BLACK);
    composerLayer->canvas->setPivot(0,0);
    gameScreen->canvas->setPivot(0,0);
    directDraw=false;
    composerLayer->DrawTo(gameScreen->canvas,
                    gameScreen->canvas->width()/2,
                    gameScreen->canvas->height()/2,
                    1.0,true,TFT_TRANSPARENT);
    directDraw=true;

    // flush to TFT dev
    //if (dirty){

    tft->setPivot(0,0);
    gameScreen->DirectDraw(0,0);
    dirty=false;
*/
 //   }

    //lAppLog("Tiles count // Visible: %u miss: %u empty: %u pixels: %d\n",tileCount,tileMissCount,tileEmptyCount, currentLevel.width*currentLevel.height);

    /*
    tileCount=0;
    for(int16_t y=disY;y<(composerLayer->canvas->height()+disY);y+=tileH) {
        for(int16_t x=disX;x<(composerLayer->canvas->width()+disX);x+=tileW) {
            int16_t tileX = (tileOffsetX+((x-disX)/tileW));
            int16_t tileY = (tileOffsetY+((y-disY)/tileH));

            // check bounds

            if ( false == ActiveRect::InRect(tileX,tileY,0,0,currentLevel.height-1,currentLevel.width-1)) { continue; }
            uint16_t tileColor = currentLevel.floorMap->readPixel(tileX,tileY);
            if ( 255 == tileColor) { continue; }
            //lAppLog("Tile X: %d Y: %d\n",tileX,tileY);

            // floor sprite rotation
            if ( 52 ==  tileColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,53); }
            else if ( 53 ==  tileColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,54); }
            else if ( 54 ==  tileColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,52); }
            
            if ( 58 ==  tileColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,59); }
            else if ( 59 ==  tileColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,60); }
            else if ( 60 ==  tileColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,58); }
            
            if ( 29 ==  tileColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,30); }
            else if ( 30 ==  tileColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,31); }
            else if ( 31 ==  tileColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,32); }
            else if ( 32 ==  tileColor ) { currentLevel.floorMap->drawPixel(tileX,tileY,29); }
            

            uint16_t tileColor2 = currentLevel.wallMap->readPixel(x,y);
            // walls anim
            if ( 52 ==  tileColor2 ) { currentLevel.wallMap->drawPixel(tileX,tileY,53); }
            else if ( 53 ==  tileColor2 ) { currentLevel.wallMap->drawPixel(tileX,tileY,54); }
            else if ( 54 ==  tileColor2 ) { currentLevel.wallMap->drawPixel(tileX,tileY,52); }
            if ( 58 ==  tileColor2 ) { currentLevel.wallMap->drawPixel(tileX,tileY,59); }
            else if ( 59 ==  tileColor2 ) { currentLevel.wallMap->drawPixel(tileX,tileY,60); }
            else if ( 60 ==  tileColor2 ) { currentLevel.wallMap->drawPixel(tileX,tileY,58); }
            if ( 29 ==  tileColor2 ) { currentLevel.wallMap->drawPixel(tileX,tileY,30); }
            else if ( 30 ==  tileColor2 ) { currentLevel.wallMap->drawPixel(tileX,tileY,31); }
            else if ( 31 ==  tileColor2 ) { currentLevel.wallMap->drawPixel(tileX,tileY,32); }
            else if ( 32 ==  tileColor2 ) { currentLevel.wallMap->drawPixel(tileX,tileY,29); }
            // get again the new possible value
            tileColor = currentLevel.floorMap->readPixel(tileX,tileY);
            if ( 255 == tileColor) { continue; } // used as none (same as readPixel returns 0xffff out of map)
            // push the bitmap on the buffer
            const unsigned char *ptr=DungeonTileSets[tileColor];
            composerLayer->canvas->pushImage(x,y,tileW,tileH,(uint16_t *)ptr);
            tileCount++;
        }   
    }
    lAppLog("Tiles processed: %u\n",tileCount);
            */

           /*
    composerLayer->canvas->setPivot(0,0);

    directDraw=false;
    composerLayer->DrawTo(gameScreen->canvas,60,60,1.0,true);
    directDraw=true;
*/
    //composerLayer->canvas->pushRotated(gameScreen->canvas,0,TFT_BLACK);
    //lAppLog("Visible Tiles: %u\n",tileCount);
    //lAppLog("Displacement: X: %d Y: %d\n",disX,disY);
    //int32_t calculatedX = touchDragVectorX+((TFT_WIDTH-composerLayer->canvas->width())/2);
    //int32_t calculatedY = touchDragVectorY+((TFT_HEIGHT-composerLayer->canvas->height())/2);
/*
    if ( dirty ) {
        // dump the local terrain
        directDraw=false;
        composerLayer->DrawTo(gameScreen->canvas);
        directDraw=true;
        //composerLayer->canvas->pushSprite(0,0);
        gameScreen->canvas->pushSprite(0,0);
        dirty=false;
    }*/
}


void DungeonGameApplication::NoticesBanner(const char *what) {
    if ( false == currentLevel.running ) {
        tft->fillRect(0,TFT_HEIGHT-40,TFT_WIDTH,40,tft->color24to16(0x191919));
        tft->drawString(what, TFT_WIDTH/2,TFT_HEIGHT-10);
    }
}
