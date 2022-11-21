#include "Dungeon.hpp"
#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include "../LogView.hpp"   // for lLog functions

#include "../../../static/Dungeon/img_splash_dungeon.c" // splash
//extern const unsigned char * DungeonTileSets;
extern const unsigned char * DungeonTileSets[];

extern void DungeonGameLevelGenerator(void *data);

extern void DungeonLevelGenerator(void *data);

DungeonGameApplication::~DungeonGameApplication() {
    if ( currentLevel.valid ) {
        currentLevel.floorMap->deleteSprite();
        delete currentLevel.floorMap;
        currentLevel.wallMap->deleteSprite();
        delete currentLevel.wallMap;
        currentLevel.topMap->deleteSprite();
        delete currentLevel.topMap;
    }

    if ( currentLevel.inProgress ) {
        lAppLog("Killing map generator....\n");
        vTaskDelete(MapGeneratorHandle);
        currentLevel.inProgress=false;
    }
    delete gameScreen;
    delete layer0;
    delete layer1;
    delete layer2;
    directDraw=false;
}

DungeonGameApplication::DungeonGameApplication() {
    directDraw=true;

    gameScreen=new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    layer0=new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    layer1=new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    layer2=new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    // launch map generator meanwhile the app is loading
    currentLevel.valid=false;

    lAppLog("Starting map generator...\n");
    xTaskCreate(DungeonLevelGenerator, "", LUNOKIOT_APP_STACK_SIZE, &currentLevel, uxTaskPriorityGet(NULL), &MapGeneratorHandle);

    // use canvas as splash screen
    canvas->setSwapBytes(true);
    canvas->pushImage(0,0,img_splash_dungeon.width,img_splash_dungeon.height, (uint16_t *)img_splash_dungeon.pixel_data);
    canvas->setSwapBytes(false);

}
bool DungeonGameApplication::Tick() {
    if (false == currentLevel.valid ) {
        if ( 0 == waitGeneratorTimeout ) {
            waitGeneratorTimestamp=0;
            waitGeneratorTimeout=millis()+20000;
        } else {
            if ( millis() > waitGeneratorTimeout ) {
                lAppLog("Map generator timeout, retrying...\n");
                vTaskDelete(MapGeneratorHandle);
                currentLevel.inProgress=false;
                lAppLog("Starting map generator (rertry %d)...\n",generationRetries);
                generationRetries++;
                xTaskCreate(DungeonLevelGenerator, "", LUNOKIOT_APP_STACK_SIZE, &currentLevel, uxTaskPriorityGet(NULL), &MapGeneratorHandle);
                waitGeneratorTimeout=0;
            } else if ( millis() > waitGeneratorTimestamp ) {
                if ( currentSentence > (sizeof(LoadSentences)/sizeof(LoadSentences[0]) ) ) {
                    currentSentence=0;
                }
                tft->fillRect(0,TFT_HEIGHT-30,TFT_WIDTH,30,tft->color24to16(0x191919));
                tft->setTextColor(TFT_WHITE,tft->color24to16(0x191919));
                tft->setTextDatum(BC_DATUM);
                tft->setTextFont(0);
                tft->setTextSize(1);
                tft->drawString(LoadSentences[currentSentence], TFT_WIDTH/2,TFT_HEIGHT-10);
                waitGeneratorTimestamp=millis()+2000;
                //Serial.printf("CURRENT %d\n", currentSentence);
                currentSentence++;
            }
        }
        return false;
    }
    if ( touched ) {
        UINextTimeout = millis()+UITimeout; // do not sleep if user is playing
        if ( false == lastThumb) {
            lastThumb=true;
            // do something at touch BEGIN
            return false;
        } else {
            // draw WHILE touch
            gameScreen->canvas->pushSprite(
                (touchDragVectorX),
                (touchDragVectorY));
            tft->fillRect(0,0,TFT_WIDTH,touchDragVectorY,TFT_BLACK); // up window
            tft->fillRect(0,gameScreen->canvas->height()+touchDragVectorY,TFT_WIDTH,abs(touchDragVectorY),TFT_BLACK); // down window
            tft->fillRect(0,touchDragVectorY,touchDragVectorX,gameScreen->canvas->width(),TFT_BLACK); // left window
            tft->fillRect(touchDragVectorX+gameScreen->canvas->width(),touchDragVectorY,abs(touchDragVectorX),gameScreen->canvas->height(),TFT_BLACK); // right window
            return false;
        }
    } else {
        if (lastThumb) {
            lastThumb=false;
            // do something at touch END

            // lets go redraw the view
            offsetX+=touchDragVectorX;
            offsetY+=touchDragVectorY;
            int16_t disX=(offsetX%tileW);
            int16_t disY=(offsetY%tileH);
            // must be inverted
            int32_t tileOffsetX=((offsetX/tileW))*-1;
            int32_t tileOffsetY=((offsetY/tileH))*-1;
            Serial.printf("Offset X: %d Y: %d\n",tileOffsetX,tileOffsetY);

            //lAppLog("[Touch] END offX: %d offY: %d DX: %d DY: %d TX: %d TY: %d\n",offsetX,offsetY,disX,disY,tileOffsetX,tileOffsetY);

            // draw background
            gameScreen->canvas->fillSprite(TFT_BLACK);
            for(int16_t y=disY;y<(gameScreen->canvas->height()+disY);y+=tileH) {
                for(int16_t x=disX;x<(gameScreen->canvas->width()+disX);x+=tileW) {
                    gameScreen->canvas->fillCircle(x+(tileW/2),y+(tileH/2),2,canvas->color24to16(0x000722));
                }
            }
            
            // first layer render (floor)
            layer0->canvas->fillSprite(TFT_BLACK);
            for(int16_t y=disY;y<(layer0->canvas->height()+disY);y+=tileH) {
                for(int16_t x=disX;x<(layer0->canvas->width()+disX);x+=tileW) {
                    int16_t tileX = (tileOffsetX+((x-disX)/tileW));
                    int16_t tileY = (tileOffsetY+((y-disY)/tileH));
                    // check bounds
                    if ( ( tileX >= 0 ) && ( tileY >= 0 ) && ( tileX < currentLevel.width ) && ( tileY < currentLevel.height ) ) {
                        uint16_t tileColor = currentLevel.floorMap->readPixel(tileX,tileY);
                        if ( 255 != tileColor ) {
                            //if ( tileColor < (sizeof(DungeonTileSets)/sizeof(DungeonTileSets[0])) ) { // check tileset bounds
                            //Serial.printf("C: %u ",tileColor);
                            const unsigned char *ptr=DungeonTileSets[tileColor];
                            layer0->canvas->pushImage(x,y,tileW,tileH,(uint16_t *)ptr);
                        }

                    }
                }
            }

            // second layer (walls)
            layer1->canvas->fillSprite(TFT_BLACK);
            for(int16_t y=disY;y<(layer1->canvas->height()+disY);y+=tileH) {
                for(int16_t x=disX;x<(layer1->canvas->width()+disX);x+=tileW) {
                    int16_t tileX = (tileOffsetX+((x-disX)/tileW));
                    int16_t tileY = (tileOffsetY+((y-disY)/tileH));
                    // check bounds
                    if ( ( tileX >= 0 ) && ( tileY >= 0 ) && ( tileX < currentLevel.width ) && ( tileY < currentLevel.height ) ) {
                        uint16_t tileColor = currentLevel.wallMap->readPixel(tileX,tileY);
                        if ( 255 != tileColor ) {
                            //if ( tileColor < (sizeof(DungeonTileSets)/sizeof(DungeonTileSets[0])) ) { // check tileset bounds
                            //Serial.printf("C: %u ",tileColor);
                            const unsigned char *ptr=DungeonTileSets[tileColor];
                            layer1->canvas->pushImage(x,y,tileW,tileH,(uint16_t *)ptr);
                        }

                    }
                }
            }

            // third layer (upper walls)
            layer2->canvas->fillSprite(TFT_BLACK);
            for(int16_t y=disY;y<(layer2->canvas->height()+disY);y+=tileH) {
                for(int16_t x=disX;x<(layer2->canvas->width()+disX);x+=tileW) {
                    int16_t tileX = (tileOffsetX+((x-disX)/tileW));
                    int16_t tileY = (tileOffsetY+((y-disY)/tileH));
                    // check bounds
                    if ( ( tileX >= 0 ) && ( tileY >= 0 ) && ( tileX < currentLevel.width ) && ( tileY < currentLevel.height ) ) {
                        uint16_t tileColor = currentLevel.topMap->readPixel(tileX,tileY);
                        if ( 255 != tileColor ) {
                            const unsigned char *ptr=DungeonTileSets[tileColor];
                            layer2->canvas->pushImage(x,y,tileW,tileH,(uint16_t *)ptr);
                        }

                    }
                }
            }


            
            // push layers to gameScreen
            gameScreen->canvas->setPivot(0,0);
            
            // layer0
            layer0->canvas->setPivot(0,0);
            layer0->canvas->pushRotated(gameScreen->canvas,0,TFT_BLACK);
            // layer1
            layer1->canvas->setPivot(0,0);
            layer1->canvas->pushRotated(gameScreen->canvas,0,TFT_BLACK);
            // layer2
            layer2->canvas->setPivot(0,0);
            layer2->canvas->pushRotated(gameScreen->canvas,0,TFT_BLACK);
            // push directly to TFT (directDraw=true)
            gameScreen->canvas->pushSprite(0,0); // don't use mask color
            return false;
        }
    }
    return false;
}