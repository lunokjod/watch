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

/*    if ((NULL != MapGeneratorHandle) && ( currentLevel.running )) {
        lAppLog("Killing map generator....\n");
        vTaskDelete(MapGeneratorHandle);
    }*/
    delete gameScreen;
    delete composerLayer;
    if ( nullptr != dragScreenCopy ) {
        dragScreenCopy->deleteSprite();
        delete dragScreenCopy;
        dragScreenCopy=nullptr;
    }
    //WiFi.mode(WIFI_MODE_NULL);

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
    
    gameScreen=new CanvasZWidget(TFT_HEIGHT,TFT_WIDTH);
    composerLayer=new CanvasZWidget(TFT_HEIGHT,TFT_WIDTH);
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
        animationTimeout=millis()+renderTime+(1000/3); // don't redraw meanwhile drag
        UINextTimeout = millis()+UITimeout; // no sleep if user is actively use
        
        if ( false == lastThumb) {
            lastThumb=true;
            lEvLog("Thumb IN X: %d Y: %d\n",touchX,touchY);
            // do something at touch BEGIN
            int32_t calculatedX = touchDragVectorX+((TFT_WIDTH/2)-((gameScreen->canvas->width()/2)));
            int32_t calculatedY = touchDragVectorY+((TFT_HEIGHT/2)-((gameScreen->canvas->height()/2)));
            tft->setPivot(0,0);
            gameScreen->canvas->setPivot(0,0);
            gameScreen->DrawTo(0,0,0.7,true);
        } else {
            // draw MEANWHILE touch
            int32_t calculatedX = touchDragVectorX+((TFT_WIDTH/2)-((gameScreen->canvas->width()/2)));
            int32_t calculatedY = touchDragVectorY+((TFT_HEIGHT/2)-((gameScreen->canvas->height()/2)));
            int32_t calculatedX2 = calculatedX+gameScreen->canvas->width();
            int32_t calculatedY2 = calculatedY+gameScreen->canvas->height();
            lEvLog("UI: Continuous touch X: %d Y: %d CX: %d CY: %d \n",touchX,touchY,touchDragVectorX,touchDragVectorY);
            tft->setPivot(0,0);
            gameScreen->canvas->setPivot(0,0);
            gameScreen->canvas->pushSprite(calculatedX,calculatedY);

            // ERASERS
            // spiral design to divide the effort of refresh
            tft->fillRect(calculatedX,0,TFT_WIDTH-calculatedX,calculatedY,TFT_RED); // up eraser
            tft->fillRect(0,0,calculatedX,calculatedY2,TFT_GREEN); // left window
            tft->fillRect(calculatedX2,calculatedY,TFT_WIDTH-calculatedX2,TFT_HEIGHT-calculatedY,TFT_YELLOW); // right window
            tft->fillRect(0,calculatedY2,calculatedX2,TFT_HEIGHT-calculatedY2,TFT_BLUE); // down window
        }
    } else {
        if (lastThumb) {
            lastThumb=false;
            // do something at touch END
            offsetX+=touchDragVectorX;
            offsetY+=touchDragVectorY;
            int32_t calculatedX = ((TFT_WIDTH/2)-((gameScreen->canvas->width()/2)));
            int32_t calculatedY = ((TFT_HEIGHT/2)-((gameScreen->canvas->height()/2)));
            tft->setPivot(0,0);
            gameScreen->canvas->setPivot(0,0);
            gameScreen->DirectDraw(calculatedX,calculatedY);
            lEvLog("Thumb LEAVE X: %d Y: %d OffsetX: %d OffsetY: %d VecX: %d VecY: %d\n",touchX,touchY,offsetX,offsetY,touchDragVectorX,touchDragVectorY);
            dirty=true;
        }
    }
    if ( millis() > animationTimeout ) {
        Redraw();
        animationTimeout=millis()+renderTime+(1000/3);
    } else {
        if ( dirty ) { Redraw(); }
    }
    return false;
}


void DungeonGameApplication::Redraw() {
    unsigned long beginMS=millis();
    // get current position
    int16_t disX=(offsetX%tileW);
    int16_t disY=(offsetY%tileH);
    // must be inverted
    int32_t tileOffsetX=((offsetX/tileW))*-1;
    int32_t tileOffsetY=((offsetY/tileH))*-1;
    // draw background
    composerLayer->canvas->fillSprite(TFT_BLACK);

    size_t tileCount=0;
    size_t tileMissCount=0;
    size_t tileEmptyCount=0;
    size_t PoToCXMapCurrentCounter=0;   // controls how much interaction is enough per Tick

    for(int16_t y=PoToCYMap;y<=tileOffsetY+currentLevel.height;y++) {
        for(int16_t x=PoToCXMap;x<tileOffsetX+currentLevel.width;x++) {


            //gameScreen->canvas->fillCircle(x+(tileW/2),y+(tileH/2),2,canvas->color24to16(0x000722));
            int16_t tileX = (tileOffsetX+((x-disX)/tileW));
            int16_t tileY = (tileOffsetY+((y-disY)/tileH));
            int32_t calculatedX = touchDragVectorX+((TFT_WIDTH/2)-((gameScreen->canvas->width()/2)));
            int32_t calculatedY = touchDragVectorY+((TFT_HEIGHT/2)-((gameScreen->canvas->height()/2)));
            tft->setPivot(0,0);
            gameScreen->canvas->setPivot(0,0);

                //      if ( ( tileX >= 0 ) && ( tileY >= 0 ) && ( tileX < currentLevel.width ) && ( tileY < currentLevel.height ) ) {
                /// MARRONA AQUI @TODO AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
                //Serial.printf("FUCKING TILES TX: %d TY: %d  TOX: %d TOY: %d\n",tileX,tileY,tileOffsetX,tileOffsetY);
                //            Serial.printf("OFFF X: %d Y: %d DX: %d DY: %d\n",tileOffsetX,tileOffsetY,disX,disY);

            /*
            if ( false == ActiveRect::InRect(tileX,tileY,0,0,currentLevel.height,currentLevel.width)) {
                    tileMissCount++;
                    continue;
                }*/
            /*
            if ( tileX < 0 ) {
                AAAAAAAAAAAAAAAAAa
            }*/

            //if ( ( tileX >= 0 ) && ( tileY >= 0 ) && ( tileX < currentLevel.width ) && ( tileY < currentLevel.height ) ) {
            //if ( false == ActiveRect::InRect(x,y,0,0,currentLevel.height-1,currentLevel.width-1)) { tileMissCount++; break; }
            //if ( (composerLayer->canvas->height()/tileH) <= y ) { tileMissCount++; break; }
            //if ( (composerLayer->canvas->width()/tileW) <= x ) { tileMissCount++; break; }

            uint16_t tileColor = currentLevel.floorMap->readPixel(x,y);
            if ( ( 255 == tileColor) || ( 0xFFFF == tileColor) ) {
                //lAppLog("Color 0xffff is the same as 'out of bounds' Value: %u on X: %d Y: %d\n",tileColor,x,y);
                tileEmptyCount++;
            }
            
            const unsigned char *ptr=DungeonTileSets[tileColor];
            
            PoToCXMapCurrentCounter++;  // increment until death
            if ( PoToCXMapCurrentCounter >=CNumTileProcess ) {
                tft->fillCircle(TFT_WIDTH/2,10,5,tft->color24to16(0x56818a)); 
                //lAppLog("Tile processor: Reached limit of tiles processed by cycle (max: %d)\n",CNumTileProcess);
                goto staphProcessTiles; // :P one of the few situations on the goto is recommended 
            } else {
                tft->fillCircle(TFT_WIDTH/2,10,5,tft->color24to16(0x353e45));
            }
            CanvasZWidget *tempBuffer = new CanvasZWidget(tileH,tileW,1.0,8);

            //tempBuffer->canvas->pushImage(0,0,tileW,tileH,(uint16_t *)ptr); // composerLayer
            PoToCXMap=x; // save the current offset
            PoToCYMap=y;
            //Serial.printf("Tile processor: X: %d Y: %d V: %d Task: %d\n",x,y,tileColor,PoToCXMapCurrentCounter);
            tileCount++;
            //tempBuffer->DrawTo(composerLayer->canvas,10,10,1.0,true,CanvasWidget::MASK_COLOR);
            //->canvas->pushSprite(composerLayer->canvas,0,0);
            //composerLayer->DrawTo(TFT_WIDTH/2,TFT_HEIGHT/2,(float)1.0,true);
            //delete tempBuffer;
        }
    }
    PoToCYMap = 0; // reset the work pointers (and loop infinitely)
    PoToCXMap = 0;
    //lAppLog("Tile processor: All tiles are looped :)\n");
    staphProcessTiles:  // for exit the double for loop (C don't have multi break like php or python




    //gameScreen->canvas->pushSprite(0,0);
    //composerLayer->canvas->setSwapBytes(false);
    //composerLayer->canvas->pushSprite(0,0);

    gameScreen->canvas->fillSprite(TFT_BLACK);
    directDraw=false;
    composerLayer->canvas->setPivot(0,0);
    composerLayer->DrawTo(gameScreen->canvas,0,0);
    directDraw=true;
//    if (dirty) {
    gameScreen->canvas->setPivot(0,0);
        gameScreen->DirectDraw(0,0);
        dirty=false;
//    } else {

 //   }

    lAppLog("Tiles count // Visible: %u miss: %u empty: %u pixels: %d\n",tileCount,tileMissCount,tileEmptyCount, currentLevel.width*currentLevel.height);

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
