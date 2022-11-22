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
        currentLevel.objectsMap->deleteSprite();
        delete currentLevel.objectsMap;
        currentLevel.darknessMap->deleteSprite();
        delete currentLevel.darknessMap;
    }

    if ((NULL != MapGeneratorHandle) && ( currentLevel.inProgress )) {
        lAppLog("Killing map generator....\n");
        vTaskDelete(MapGeneratorHandle);
        currentLevel.inProgress=false;
    }
    delete gameScreen;
    delete layer0;
    delete layer1;
    delete layer2;
    delete layer3;
    directDraw=false;
}

DungeonGameApplication::DungeonGameApplication() {
    directDraw=true;

    gameScreen=new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    layer0=new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    layer1=new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    layer2=new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    layer3=new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    
    // launch map generator meanwhile the app is loading
    currentLevel.valid=false;

    lAppLog("Starting map generator...\n");
    xTaskCreate(DungeonLevelGenerator, "", LUNOKIOT_APP_STACK_SIZE, &currentLevel, uxTaskPriorityGet(NULL), &MapGeneratorHandle);

    // use canvas as splash screen
    // https://www.fontspace.com/category/medieval
    canvas->setSwapBytes(true);
    canvas->pushImage(0,0,img_splash_dungeon.width,img_splash_dungeon.height, (uint16_t *)img_splash_dungeon.pixel_data);
    canvas->setSwapBytes(false);

    tft->setTextColor(TFT_WHITE,tft->color24to16(0x191919));
    tft->setTextDatum(BC_DATUM);
    tft->setFreeFont(&FreeSerif12pt7b);
    tft->setTextSize(1);

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
                if ( currentSentence >= (sizeof(LoadSentences)/sizeof(LoadSentences[0]) ) ) {
                    currentSentence=0;
                }
                tft->fillRect(0,TFT_HEIGHT-40,TFT_WIDTH,40,tft->color24to16(0x191919));
                tft->drawString(LoadSentences[currentSentence], TFT_WIDTH/2,TFT_HEIGHT-10);
                waitGeneratorTimestamp=millis()+1500;
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
            offsetX+=touchDragVectorX;
            offsetY+=touchDragVectorY;
            dirty=true;

        }
    }
    if ( millis() > forceRedrawTimeout ) {
        forceRedrawTimeout=millis()+(1000/2);
        dirty=true;
    }

    if ( dirty ) { // lets go redraw the view
        unsigned long beginMS=millis();
        int16_t disX=(offsetX%tileW);
        int16_t disY=(offsetY%tileH);
        // must be inverted
        int32_t tileOffsetX=((offsetX/tileW))*-1;
        int32_t tileOffsetY=((offsetY/tileH))*-1;
        //Serial.printf("Offset X: %d Y: %d\n",tileOffsetX,tileOffsetY);

        // draw background
        gameScreen->canvas->fillSprite(TFT_BLACK);
        layer0->canvas->fillSprite(TFT_BLACK);
        layer1->canvas->fillSprite(TFT_BLACK);
        layer2->canvas->fillSprite(TFT_BLACK);
        layer3->canvas->fillSprite(TFT_BLACK);


        for(int16_t y=disY;y<(gameScreen->canvas->height()+disY);y+=tileH) {
            for(int16_t x=disX;x<(gameScreen->canvas->width()+disX);x+=tileW) {
                gameScreen->canvas->fillCircle(x+(tileW/2),y+(tileH/2),2,canvas->color24to16(0x000722));

                int16_t tileX = (tileOffsetX+((x-disX)/tileW));
                int16_t tileY = (tileOffsetY+((y-disY)/tileH));

                // check bounds
                if ( ( tileX >= 0 ) && ( tileY >= 0 ) && ( tileX < currentLevel.width ) && ( tileY < currentLevel.height ) ) {
                    // layer0 render
                    uint16_t tileColor = currentLevel.floorMap->readPixel(tileX,tileY);

                    // animation (sprite rotation)
                    // floor anim
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


                    if ( 255 != tileColor ) {
                        const unsigned char *ptr=DungeonTileSets[tileColor];
                        layer0->canvas->pushImage(x,y,tileW,tileH,(uint16_t *)ptr);
                    }
                    // layer 3 render
                    tileColor = currentLevel.objectsMap->readPixel(tileX,tileY);
                    if ( 255 != tileColor ) {
                        const unsigned char *ptr=DungeonTileSets[tileColor];
                        layer3->canvas->pushImage(x,y,tileW,tileH,(uint16_t *)ptr);
                    }
                    // layer 1 render
                    tileColor = currentLevel.wallMap->readPixel(tileX,tileY);
                    if ( 255 != tileColor ) {
                        //if ( tileColor < (sizeof(DungeonTileSets)/sizeof(DungeonTileSets[0])) ) { // check tileset bounds
                        //Serial.printf("C: %u ",tileColor);
                        const unsigned char *ptr=DungeonTileSets[tileColor];
                        layer1->canvas->pushImage(x,y,tileW,tileH,(uint16_t *)ptr);
                    }
                    // layer 2 render
                    tileColor = currentLevel.topMap->readPixel(tileX,tileY);
                    if ( 255 != tileColor ) {
                        //if ( tileColor < (sizeof(DungeonTileSets)/sizeof(DungeonTileSets[0])) ) { // check tileset bounds
                        //Serial.printf("C: %u ",tileColor);
                        const unsigned char *ptr=DungeonTileSets[tileColor];
                        layer2->canvas->pushImage(x,y,tileW,tileH,(uint16_t *)ptr);
                    }

                }
            }
        }

        // push layers to gameScreen
        gameScreen->canvas->setPivot(0,0);
        
        // layer0 (floor)
        layer0->canvas->setPivot(0,0);
        layer0->canvas->pushRotated(gameScreen->canvas,0,TFT_BLACK);

        // Layer 3 (objetcts/items)
        layer3->canvas->setPivot(0,0);
        layer3->canvas->pushRotated(gameScreen->canvas,0,TFT_BLACK);

        //@TODO HERE THE PLAYERS LAYER

        // layer1 (walls)
        layer1->canvas->setPivot(0,0);
        layer1->canvas->pushRotated(gameScreen->canvas,0,TFT_BLACK);
        // layer2 (wall decoration)
        layer2->canvas->setPivot(0,0);
        layer2->canvas->pushRotated(gameScreen->canvas,0,TFT_BLACK);

        // darkness 
        for(int16_t y=disY;y<(gameScreen->canvas->height()+disY);y+=tileH) {
            for(int16_t x=disX;x<(gameScreen->canvas->width()+disX);x+=tileW) {

                int16_t tileX = (tileOffsetX+((x-disX)/tileW));
                int16_t tileY = (tileOffsetY+((y-disY)/tileH));

                // check bounds
                if ( ( tileX >= 0 ) && ( tileY >= 0 ) && ( tileX < currentLevel.width ) && ( tileY < currentLevel.height ) ) {
                    uint16_t darkColor = currentLevel.darknessMap->readPixel(tileX,tileY);
                    uint16_t originalColor = gameScreen->canvas->readPixel(x,y);
                    uint16_t mixColor = tft->alphaBlend(255,darkColor,originalColor);
                    gameScreen->canvas->drawPixel(x,y,mixColor);
                }
            }
        }

        // push directly to TFT (directDraw=true)
        gameScreen->canvas->pushSprite(0,0); // don't use mask color
        unsigned long endMS=millis();
        Serial.printf("Time: %u\n", endMS-beginMS);
        dirty=false;
    }
    return false;
}
