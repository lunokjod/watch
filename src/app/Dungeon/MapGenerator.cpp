#include <Arduino.h>
#include "Dungeon.hpp"
#include "../LogView.hpp"

//extern const unsigned char * DungeonTileSets[];
/*
extern uint8_t *dungeonGameGrid;
extern uint8_t *dungeonGameGridHigh;
*/
void DungeonLevelGenerator(void *data) {
    levelDescriptor * thisLevel=(levelDescriptor *)data;
    thisLevel->inProgress=true;
    thisLevel->valid=false;

    if ( nullptr != thisLevel->floorMap) {
        thisLevel->floorMap->deleteSprite();
        delete thisLevel->floorMap;
        thisLevel->floorMap=nullptr;
    }
    if ( nullptr == thisLevel->floorMap) {
        thisLevel->floorMap = new TFT_eSprite(tft);
        thisLevel->floorMap->setColorDepth(16); // :( only 255 values needed but the sprites 8 bit are a crap! (drawpixel don't work)
        thisLevel->floorMap->createSprite(thisLevel->width,thisLevel->height);
        thisLevel->floorMap->fillSprite(255); // clean whole map
    }

    if ( nullptr != thisLevel->wallMap) {
        thisLevel->wallMap->deleteSprite();
        delete thisLevel->wallMap;
        thisLevel->wallMap=nullptr;
    }
    if ( nullptr == thisLevel->wallMap) {
        thisLevel->wallMap = new TFT_eSprite(tft);
        thisLevel->wallMap->setColorDepth(16); // :( only 255 values needed but the sprites 8 bit are a crap! (drawpixel don't work)
        thisLevel->wallMap->createSprite(thisLevel->width,thisLevel->height);
        thisLevel->wallMap->fillSprite(255); // clean whole map
    }
    if ( nullptr != thisLevel->topMap) {
        thisLevel->topMap->deleteSprite();
        delete thisLevel->topMap;
        thisLevel->topMap=nullptr;
    }
    if ( nullptr == thisLevel->topMap) {
        thisLevel->topMap = new TFT_eSprite(tft);
        thisLevel->topMap->setColorDepth(16); // :( only 255 values needed but the sprites 8 bit are a crap! (drawpixel don't work)
        thisLevel->topMap->createSprite(thisLevel->width,thisLevel->height);
        thisLevel->topMap->fillSprite(255); // clean whole map
    }



    // build here! (every value is the offset of DungeonTileSets)

    // easy/short accessors
    TFT_eSprite * floorMap = thisLevel->floorMap;
    TFT_eSprite * wallMap = thisLevel->wallMap;
    TFT_eSprite * topMap = thisLevel->topMap;

    lAppLog("Generating paths (horizontal)....\n");
    int horizontalPaths=thisLevel->horizontalPaths;
    while(horizontalPaths>0) {
        int32_t x=0;
        int32_t y=random(0,floorMap->width());
        if ( 255 == floorMap->readPixel(x,y) ) { // put track on empty place
            floorMap->drawFastHLine(x,y-1,floorMap->height()-x,0); // floor
            floorMap->drawFastHLine(x,y,floorMap->height()-x,0); // floor
            floorMap->drawFastHLine(x,y+1,floorMap->height()-x,0); // floor
            lAppLog("H Path %d generated X: %d Y: %d\n", horizontalPaths,x,y);
            horizontalPaths--;
        }
        delay(1);
    }
    lAppLog("Generating paths (vertical)....\n");
    int verticalPaths=thisLevel->verticalPaths;
    while(verticalPaths>0) {
        int32_t x=random(0,floorMap->height());
        int32_t y=0;
        if ( 255 == floorMap->readPixel(x,y) ) { // put track on empty place
            floorMap->drawFastVLine(x-1,y,floorMap->width()-y,0); // floor
            floorMap->drawFastVLine(x,y,floorMap->width()-y,0); // floor
            floorMap->drawFastVLine(x+1,y,floorMap->width()-y,0); // floor
            lAppLog("V Path %d generated X: %d Y: %d\n", verticalPaths,x,y);
            verticalPaths--;
        }
        delay(1);
    }
    lAppLog("Generating rooms....\n");
    // build rooms
    int rooms=thisLevel->rooms;    
    while(rooms>0) {
        int32_t w=random(thisLevel->minRoomSize,thisLevel->maxRoomSize);
        int32_t h=random(thisLevel->minRoomSize,thisLevel->maxRoomSize);
        int32_t x=random(0,(floorMap->width()-w)-thisLevel->minRoomSize);
        int32_t y=random(0,(floorMap->height()-h)-thisLevel->minRoomSize);
        if ( 0 == floorMap->readPixel(x,y) ) { // room must have a path
            floorMap->fillRoundRect(x,y,w,h,thisLevel->roomRadius,0); // mark as floor
            lAppLog("Room %d generated H: %d W: %d\n", rooms,h,w);
            rooms--;
        }
        delay(1);
    }
    lAppLog("Removing small holes...\n");
    // remove stupid 1x1 holes
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
            uint16_t color = floorMap->readPixel(x,y);
            if ( 255 == color ) { // find empty
                uint16_t colorUp = floorMap->readPixel(x,y-1);
                if ( 0 == colorUp ) {
                    uint16_t colorDown = floorMap->readPixel(x,y+1);
                    if ( 0 == colorDown ) {
                        floorMap->drawPixel(x,y,0); // add floor on hole
                    }
                }
                uint16_t colorLeft = floorMap->readPixel(x-1,y);
                if ( 0 == colorLeft ) {
                    uint16_t colorRight = floorMap->readPixel(x+1,y);
                    if ( 0 == colorRight ) {
                        floorMap->drawPixel(x,y,0); // add floor on hole
                    }
                }
            }
        }
    }



    lAppLog("Generating walls (left/right)....\n");
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
            uint16_t color = floorMap->readPixel(x,y);
            if ( 0 == color ) {
                uint16_t colorLeft = floorMap->readPixel(x-1,y);
                if ( 255 == colorLeft ) { // empty space
                    Serial.printf("Left wall X: %d Y: %d ",x,y);
                    wallMap->drawPixel(x-1,y,23); // put wall here!
                }
                uint16_t colorRight = floorMap->readPixel(x+1,y);
                if ( 255 == colorRight ) { // empty space
                    Serial.printf("Right wall X: %d Y: %d ",x,y);
                    wallMap->drawPixel(x+1,y,24); // put wall here!
                }
            }
        }
    }
    lAppLog("Generating walls (up/down)....\n");
    // put up/down walls 
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
            uint16_t color = floorMap->readPixel(x,y);
            if ( 0 == color ) {
                uint16_t colorUp = floorMap->readPixel(x,y-1);
                if ( 255 == colorUp ) { // empty space
                    Serial.printf("Up wall X: %d Y: %d ",x,y);
                    wallMap->drawPixel(x,y-1,12); // put wall here!
                }
                uint16_t colorDown = floorMap->readPixel(x,y+1);
                if ( 255 == colorDown ) { // empty space
                    Serial.printf("Down wall X: %d Y: %d ",x,y);
                    wallMap->drawPixel(x,y+1,12); // put wall here!
                }
            }
        }
    }
    lAppLog("Generating wall decorations....\n");
    // put up/down walls 
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
            uint16_t wallColor = wallMap->readPixel(x,y);
            if ( 12 == wallColor ) {                // front wall put up decoration
                topMap->drawPixel(x,y-1,18);       // top mid
            }
        }
    }

    lAppLog("Details...\n");
    // change some tiles to add more visual richness
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
            uint16_t floorColor = floorMap->readPixel(x,y);
            uint16_t wallColor = wallMap->readPixel(x,y);

            if ( 0 == floorColor ) {                // is a basic tile floor? then random floor tile
                uint32_t nuColor = random(0,8);
                floorMap->drawPixel(x,y,nuColor);
            }

            // wall_hole_1
            //wall_hole_2
            if ( 12 == wallColor ) {                // add holes to the walls
                if ( 20 > random(1,100)) {
                    uint32_t holeversion = 14 + random(0,1);
                    wallMap->drawPixel(x,y,holeversion);
                }
            }

        }
        delay(1);
    }
    lAppLog("Dungeon generated!\n");
    thisLevel->valid=true;
    thisLevel->inProgress=false;
    vTaskDelete(NULL);
};



/*
void DungeonLevelGenerator(void *data) {
    dungeonGameGridGenerated=false;
    levelDescriptor * thisLevel=(levelDescriptor *)data;

    if ( nullptr != dungeonGameGrid ) { free(dungeonGameGrid); }
    if ( nullptr != dungeonGameGridHigh ) { free(dungeonGameGrid); }

    dungeonGameGrid = (uint8_t*)ps_malloc(DungeonGameApplication::GridSizeH*DungeonGameApplication::GridSizeW);
    dungeonGameGridHigh = (uint8_t*)ps_malloc(DungeonGameApplication::GridSizeH*DungeonGameApplication::GridSizeW);


    // clean high map
    for ( size_t off=0;off<(DungeonGameApplication::GridSizeH*DungeonGameApplication::GridSizeW);off++) {
        dungeonGameGridHigh[off] = 255;
    }
    
    // generate random floor
    for ( size_t off=0;off<(DungeonGameApplication::GridSizeH*DungeonGameApplication::GridSizeW);off++) {
        dungeonGameGrid[off] = random(0,7);
    }
    // add props
    for (int i=0;i<4;i++ ) {
        uint8_t *crateLoc = dungeonGameGridHigh+random(DungeonGameApplication::GridSizeW,((DungeonGameApplication::GridSizeH*DungeonGameApplication::GridSizeW)-DungeonGameApplication::GridSizeW));
        *crateLoc = 38; //box
    }
    for (int i=0;i<5;i++ ) {
        uint8_t *crateLoc = dungeonGameGridHigh+random(DungeonGameApplication::GridSizeW,((DungeonGameApplication::GridSizeH*DungeonGameApplication::GridSizeW)-DungeonGameApplication::GridSizeW));
        *crateLoc = 39; // skull
    }

    // don't use first line (used for props in high grid)
    for ( size_t off=0;off<DungeonGameApplication::GridSizeW;off++) { dungeonGameGrid[off] = 255;  }


    for ( size_t y=0;y<DungeonGameApplication::GridSizeH-1;y++) {
        uint8_t *ptr=dungeonGameGridHigh+((y*DungeonGameApplication::GridSizeW));
        *ptr=24; // left wall 
        ptr=dungeonGameGridHigh+((y*DungeonGameApplication::GridSizeW)+(DungeonGameApplication::GridSizeW-1));
        *ptr=23; // right wall
    }

    for ( size_t x=0;x<DungeonGameApplication::GridSizeW;x++) {
        uint8_t *ptr=dungeonGameGrid+(DungeonGameApplication::GridSizeW+x);
        *ptr=12; // high wall
        ptr=dungeonGameGrid+(x+(DungeonGameApplication::GridSizeW*(DungeonGameApplication::GridSizeH-1)));
        *ptr=12; // lower wall
    }

    // top & bottom wall props
    for ( size_t x=0;x<DungeonGameApplication::GridSizeW;x++) {
        dungeonGameGridHigh[x] = 18;
        dungeonGameGridHigh[((DungeonGameApplication::GridSizeH-2)*DungeonGameApplication::GridSizeW)+x] = 18;
    }
    // the square
    dungeonGameGridHigh[((DungeonGameApplication::GridSizeH-2)*DungeonGameApplication::GridSizeW)] = 22;
    dungeonGameGridHigh[((DungeonGameApplication::GridSizeH-2)*DungeonGameApplication::GridSizeW)+(DungeonGameApplication::GridSizeW-1)] = 21;

    dungeonGameGridGenerated=true;
    lAppLog("Dungeon generated!\n");
    vTaskDelete(NULL);
};
*/