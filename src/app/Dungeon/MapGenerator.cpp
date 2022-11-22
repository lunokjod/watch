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
    if ( nullptr != thisLevel->darknessMap) {
        thisLevel->darknessMap->deleteSprite();
        delete thisLevel->darknessMap;
        thisLevel->darknessMap=nullptr;
    }
    if ( nullptr == thisLevel->darknessMap) {
        thisLevel->darknessMap = new TFT_eSprite(tft);
        thisLevel->darknessMap->setColorDepth(16); // :( only 255 values needed but the sprites 8 bit are a crap! (drawpixel don't work)
        thisLevel->darknessMap->createSprite(thisLevel->width,thisLevel->height);
        thisLevel->darknessMap->fillSprite(0); // clean whole map with dark!
    }


    if ( nullptr != thisLevel->objectsMap) {
        thisLevel->objectsMap->deleteSprite();
        delete thisLevel->objectsMap;
        thisLevel->objectsMap=nullptr;
    }
    if ( nullptr == thisLevel->objectsMap) {
        thisLevel->objectsMap = new TFT_eSprite(tft);
        thisLevel->objectsMap->setColorDepth(16); // :( only 255 values needed but the sprites 8 bit are a crap! (drawpixel don't work)
        thisLevel->objectsMap->createSprite(thisLevel->width,thisLevel->height);
        thisLevel->objectsMap->fillSprite(255); // clean whole map
    }


    // build here! (every value is the offset of DungeonTileSets)

    // easy/short accessors
    TFT_eSprite * floorMap = thisLevel->floorMap;
    TFT_eSprite * wallMap = thisLevel->wallMap;
    TFT_eSprite * topMap = thisLevel->topMap;
    TFT_eSprite * objectsMap = thisLevel->objectsMap;
    

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
                    lLog("Left wall X: %d Y: %d ",x,y);
                    wallMap->drawPixel(x-1,y,17); // put wall here!
                }
                uint16_t colorRight = floorMap->readPixel(x+1,y);
                if ( 255 == colorRight ) { // empty space
                    lLog("Right wall X: %d Y: %d ",x,y);
                    wallMap->drawPixel(x+1,y,16); // put wall here!
                }
            }
        }
    }
    lLog("\n");
    lAppLog("Generating walls (up/down)....\n");
    // put up/down walls 
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
            uint16_t color = floorMap->readPixel(x,y);
            if ( 0 == color ) {
                uint16_t colorUp = floorMap->readPixel(x,y-1);
                if ( 255 == colorUp ) { // empty space?
                    lLog("Up wall X: %d Y: %d ",x,y);
                    wallMap->drawPixel(x,y-1,12); // put wall here!
                }
                uint16_t colorDown = floorMap->readPixel(x,y+1);
                if ( 255 == colorDown ) { // empty space
                    lLog("Down wall X: %d Y: %d ",x,y);
                    wallMap->drawPixel(x,y+1,12); // put wall here!
                }
            }
        }
    }
    lLog("\n");
    lAppLog("Generating wall details...\n");
    for(int32_t y=0;y<wallMap->height();y++) {
        for(int32_t x=0;x<wallMap->width();x++) {
            uint16_t color = wallMap->readPixel(x,y);
            if ( 12 == color ) { // is a front wall
                uint16_t colorUp = wallMap->readPixel(x,y-1); // up
                uint16_t colorDown = wallMap->readPixel(x,y+1); // down
                uint16_t colorRight = wallMap->readPixel(x+1,y); // right
                uint16_t colorLeft = wallMap->readPixel(x-1,y); // left

                uint16_t colorLeftDown = wallMap->readPixel(x-1,y+1); // leftDown
                uint16_t colorRightDown = wallMap->readPixel(x+1,y+1); // rightDown
                uint16_t colorLeftUp = wallMap->readPixel(x-1,y-1); // leftUp
                uint16_t colorRightUp = wallMap->readPixel(x+1,y-1); // rightUp
                if ( 255 == colorLeft ) {
                    if ( 16 == colorLeftDown ) { wallMap->drawPixel(x-1,y,16); }
                    else if ( 17 == colorLeftDown ) { wallMap->drawPixel(x-1,y,17); }
                } else if ( 255 == colorRight ) {
                    if ( 16 == colorRightDown ) { wallMap->drawPixel(x+1,y,16); }
                    else if ( 17 == colorRightDown ) { wallMap->drawPixel(x+1,y,17); }
                
                } // re get the value (mayb is modified)
                colorRight = wallMap->readPixel(x+1,y); // right
                colorLeft = wallMap->readPixel(x-1,y); // left

                if ( 255 == colorLeft ) { // ---------------------------------------
                    if ( 16 == colorLeftUp ) { wallMap->drawPixel(x-1,y,16); }
                    else if ( 17 == colorLeftUp ) { wallMap->drawPixel(x-1,y,17); }
                } else if ( 255 == colorRight ) {
                    if ( 16 == colorRightUp ) { wallMap->drawPixel(x+1,y,16); }
                    else if ( 17 == colorRightUp ) { wallMap->drawPixel(x+1,y,17); }
                }
            }
        }
    }

    lAppLog("Generating wall decorations....\n");
    for(int32_t y=0;y<topMap->height();y++) {
        for(int32_t x=0;x<topMap->width();x++) {
            uint16_t wallColor = wallMap->readPixel(x,y);
            if ( 12 == wallColor ) {
                topMap->drawPixel(x,y-1,18);
                uint16_t bottomColor = wallMap->readPixel(x,y+1);
                if ( 16 == bottomColor ) { topMap->drawPixel(x,y+1,24); }
                else if ( 17 == bottomColor ) { topMap->drawPixel(x,y+1,23); }
            }
            else if ( 16 == wallColor ) { topMap->drawPixel(x,y,24); }
            else if ( 17 == wallColor ) { topMap->drawPixel(x,y,23); }
        }
    }
    // second iteration for corners
    for(int32_t y=0;y<topMap->height();y++) {
        for(int32_t x=0;x<topMap->width();x++) {
            uint16_t wallColor = topMap->readPixel(x,y);
             if ( 18 == wallColor ) {
                uint16_t upColor = topMap->readPixel(x,y-1);
                if ( 24 == upColor) { topMap->drawPixel(x,y,22); }
                else if ( 23 == upColor) { topMap->drawPixel(x,y,21); }
             }
        }
    }
    lAppLog("Details...\n");
    // change some tiles to add more visual richness
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
            uint16_t floorColor = floorMap->readPixel(x,y);
            uint16_t wallColor = wallMap->readPixel(x,y);
            uint16_t topColor = topMap->readPixel(x,y);
            uint16_t objectColor = objectsMap->readPixel(x,y);
            /*
            if ( 18 == topColor ) {
            } else if ( 19 == topColor ) {
            } else if ( 20 == topColor ) {
            }*/

            if ( 12 == wallColor ) {
                // add holes to the walls
                if ( 27 > random(1,100)) {          // probability of hole
                    uint32_t holeversion = 14 + random(0,1);
                    wallMap->drawPixel(x,y,holeversion);
                    delay(1);
                }
                // only on internal faces (if no floor, no decoration)
                uint16_t floorDownColor = floorMap->readPixel(x,y+1);
                if ( 0 == floorDownColor ) {
                    if ( 18  > random(1,100)) {          // probability of wall column
                        floorMap->drawPixel(x,y+1,49);
                        wallMap->drawPixel(x,y,50);
                        topMap->drawPixel(x,y-1,51);
                        delay(1);
                    } else if ( 8 > random(1,100)) {
                        wallMap->drawPixel(x,y,43);             // flag
                        delay(1);
                    } else if ( 6  > random(1,100)) {          // probability of goo
                        wallMap->drawPixel(x,y,42);
                        delay(1);
                    } else if ( 3  > random(1,100)) {          // probability of blue fountain
                        floorMap->drawPixel(x,y+1,52);
                        wallMap->drawPixel(x,y,55);
                        topMap->drawPixel(x,y-1,64);
                        delay(1);
                    } else if ( 3  > random(1,100)) {          // probability of red fountain
                        floorMap->drawPixel(x,y+1,58);
                        wallMap->drawPixel(x,y,61);
                        topMap->drawPixel(x,y-1,64);
                        delay(1);
                    }
                }
            }
            if ( ( 0 == floorColor ) && ( 255 == objectColor )) {
                if ( 2  > random(1,100)) { 
                    objectsMap->drawPixel(x,y,47); // crates
                    delay(1);
                } else if ( 2  > random(1,100)) { 
                    objectsMap->drawPixel(x,y,48); // skulls
                    delay(1);
                } else if ( 3  > random(1,100)) { 
                    floorMap->drawPixel(x,y,10); // hole
                    delay(1);
                }
            }


            if ( 0 == floorColor ) {                // is a basic tile floor? then random floor tile
                if ( 8  > random(1,100)) { 
                    floorMap->drawPixel(x,y,29); // spikes
                    delay(1);
                }
                if ( ( 0 == ( x % 4) ) && ( 0 == ( y % 4) ) ) {
                    if ( 30  > random(1,100)) {
                        objectsMap->drawPixel(x,y,65); // COLUMNS
                        wallMap->drawPixel(x,y,66); // COLUMNS
                        topMap->drawPixel(x,y-1,51); // COLUMNS
                        delay(1);
                    }
                }

                if ( 30  > random(1,100)) { 
                    uint32_t nuColor = random(0,8);
                    floorMap->drawPixel(x,y,nuColor);
                    delay(1);
                }
           }
        }
    }
    lAppLog("Dungeon generated!\n");
    thisLevel->valid=true;
    thisLevel->inProgress=false;
    vTaskDelete(NULL);
};
