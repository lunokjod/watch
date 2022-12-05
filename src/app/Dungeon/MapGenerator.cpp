#include <Arduino.h>
#include <esp_task_wdt.h>
#include "Dungeon.hpp"
#include "../LogView.hpp"

// task generator
void DungeonLevelGenerator(void *data) {
    levelDescriptor * thisLevel=(levelDescriptor *)data;
    thisLevel->inProgress=true;
    thisLevel->valid=false;

    // destroy old level
    if ( nullptr != thisLevel->floorMap) {
        thisLevel->floorMap->deleteSprite();
        delete thisLevel->floorMap;
        thisLevel->floorMap=nullptr;
    }
    // rebuild floor map
    if ( nullptr == thisLevel->floorMap) {
        thisLevel->floorMap = new TFT_eSprite(tft);
        thisLevel->floorMap->setColorDepth(16); // :( only 255 values needed but the sprites 8 bit are a crap! (drawpixel don't work as expected)
        thisLevel->floorMap->createSprite(thisLevel->width,thisLevel->height);
        thisLevel->floorMap->fillSprite(255); // clean whole map
    }
    // regenerate the other layers...
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
        thisLevel->darknessMap->fillSprite(255); // clean whole map with dark!
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
        int32_t x=2;
        int32_t y=random(2,floorMap->height()-4);
        if ( ( 255 == floorMap->readPixel(x,y))
                &&( 255 == floorMap->readPixel(x,y-1) )
                &&( 255 == floorMap->readPixel(x,y-2) )
                &&( 255 == floorMap->readPixel(x,y-3) )
                &&( 255 == floorMap->readPixel(x,y-4) )
                &&( 255 == floorMap->readPixel(x,y+1) )
                &&( 255 == floorMap->readPixel(x,y+2) )
                &&( 255 == floorMap->readPixel(x,y+3) )
                &&( 255 == floorMap->readPixel(x,y+4) )
                ) { // put track on empty place
        //if ( 255 == floorMap->readPixel(x,y) ) { // put track on empty place
            floorMap->drawFastHLine(x,y-1,(floorMap->width()-x)-4,0); // floor
            floorMap->drawFastHLine(x,y,(floorMap->width()-x)-4,0); // floor
            floorMap->drawFastHLine(x,y+1,(floorMap->width()-x)-4,0); // floor
            lAppLog("H Path %d generated X: %d Y: %d\n", horizontalPaths,x,y);
            horizontalPaths--;
        }
        //yield();
        delay(2);
    }
    lAppLog("Generating paths (vertical)....\n");
    int verticalPaths=thisLevel->verticalPaths;
    while(verticalPaths>0) {
        int32_t x=random(2,floorMap->height()-4);
        int32_t y=2;
        if ( ( 255 == floorMap->readPixel(x,y))
                &&( 255 == floorMap->readPixel(x-1,y) )
                &&( 255 == floorMap->readPixel(x-2,y) )
                &&( 255 == floorMap->readPixel(x-3,y) )
                &&( 255 == floorMap->readPixel(x-4,y) )
                &&( 255 == floorMap->readPixel(x+1,y) )
                &&( 255 == floorMap->readPixel(x+2,y) )
                &&( 255 == floorMap->readPixel(x+3,y) )
                &&( 255 == floorMap->readPixel(x+4,y) )
                ) { // put track on empty place
            floorMap->drawFastVLine(x-1,y,(floorMap->width()-4)-y,0); // floor
            floorMap->drawFastVLine(x,y,(floorMap->width()-4)-y,0); // floor
            floorMap->drawFastVLine(x+1,y,(floorMap->width()-4)-y,0); // floor
            lAppLog("V Path %d generated X: %d Y: %d\n", verticalPaths,x,y);
            verticalPaths--;
        }
        delay(2);
    }
    lAppLog("Building rooms....\n");
    // build rooms
    int rooms=thisLevel->rooms;    
    while(rooms>0) {
        int32_t w=random(thisLevel->minRoomSize,thisLevel->maxRoomSize);
        int32_t h=random(thisLevel->minRoomSize,thisLevel->maxRoomSize);

        // from 0 to end of world but with minimal and maximal size
        int32_t x=random(3,thisLevel->width-6)+random(thisLevel->minRoomSize,(thisLevel->maxRoomSize-w)-thisLevel->minRoomSize);
        int32_t y=random(3,((thisLevel->height-6)-h)-thisLevel->minRoomSize);

        bool isEmptyPlace=true;
        /*
        for(int16_t pey=y;pey<y+h;pey++) {
            if ( false == isEmptyPlace ) { break; }
            for(int16_t pex=x;pex<x+w;pex++) {
                if ( 255 != floorMap->readPixel(pex,pey) ) {
                    isEmptyPlace=false;
                    break;
                }
            }
        }*/
        if ( isEmptyPlace ) { // room have space
            floorMap->fillRoundRect(x,y,w,h,thisLevel->roomRadius,0); // mark as floor
            lAppLog("Room %d generated X: %d Y: %d H: %d W: %d\n", rooms,x,y,h,w);
            rooms--;
        }
        //yield();
        delay(5);
    }
    
    lAppLog("Removing small holes...\n");
    // remove stupid holes using colindant values
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

    lAppLog("Generating walls (up/down)....\n");
    // put up/down walls 
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
            uint16_t color = floorMap->readPixel(x,y);
            if ( 0 == color ) { // floor detected
                uint16_t colorUp = floorMap->readPixel(x,y-1);
                if ( 255 == colorUp ) { // empty space?
                    //lLog("Up wall X: %d Y: %d ",x,y);
                    wallMap->drawPixel(x,y-1,12); // put wall here!
                }
                uint16_t colorDown = floorMap->readPixel(x,y+1);
                if ( 255 == colorDown ) { // empty space
                    //lLog("Down wall X: %d Y: %d ",x,y);
                    wallMap->drawPixel(x,y+1,12); // put wall here!
                }
            }
        }
    }

    lAppLog("Generating walls (left/right)....\n");
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
            uint16_t color = floorMap->readPixel(x,y);
            if ( 0 == color ) { // find piece of floor
                uint16_t colorLeft = floorMap->readPixel(x-1,y); // on FLOOR

                if ( 255 == colorLeft ) { // empty space
                    uint16_t colorWallLeft = wallMap->readPixel(x-1,y); // on WALL
                    if ( 255 == colorWallLeft ) { // no wall?
                        //lLog("Left wall X: %d Y: %d ",x,y);
                        wallMap->drawPixel(x-1,y,17); // put wall here!
                    }
                }

                uint16_t colorRight = floorMap->readPixel(x+1,y);
                if ( 255 == colorRight ) { // empty space
                    uint16_t colorWallRight = wallMap->readPixel(x+1,y); // on WALL
                    if ( 255 == colorWallRight ) { // no wall?
                        //lLog("Left wall X: %d Y: %d ",x,y);
                        wallMap->drawPixel(x+1,y,16); // put wall here!
                    }
                }
            }
            // end the walls beltween horizontal walls
            uint16_t colorWall = wallMap->readPixel(x,y); // on WALL
            if ( 12 == colorWall ) {
                uint16_t colorRightWall = wallMap->readPixel(x+1,y); // on WALL
                uint16_t colorRight = floorMap->readPixel(x+1,y); // on floor
                if ( ( 255 == colorRightWall )&&( 255 == colorRight )) { // empty space
                    wallMap->drawPixel(x+1,y,16); // put wall here!
                }

                uint16_t colorLeftWall = wallMap->readPixel(x-1,y); // on WALL
                uint16_t colorLeft = floorMap->readPixel(x-1,y); // on floor
                if ( ( 255 == colorLeftWall )&&( 255 == colorLeft )) { // empty space
                    wallMap->drawPixel(x-1,y,17); // put wall here!
                }

            }

        }
    }

    lAppLog("Generating wall decorations....\n");
    for(int32_t y=0;y<topMap->height();y++) {
        for(int32_t x=0;x<topMap->width();x++) {
            uint16_t wallColor = wallMap->readPixel(x,y);
            if ( 12 == wallColor ) {

                uint16_t wallDownColor = wallMap->readPixel(x,y+1);
                if ( 16 == wallDownColor ) { // is wall left?
                    topMap->drawPixel(x,y,26);
                } else if ( 17 == wallDownColor ) { // is right
                    topMap->drawPixel(x,y,25);
                }

                uint16_t wallUpColor = wallMap->readPixel(x,y-1);
                if ( 16 == wallUpColor ) { // is wall left?
                    topMap->drawPixel(x,y-1,22);
                } else if ( 17 == wallUpColor ) { // is right
                    topMap->drawPixel(x,y-1,21);
                } else {
                    uint16_t wallRightColor = wallMap->readPixel(x+1,y);
                    uint16_t wallLeftColor = wallMap->readPixel(x-1,y);
                    if ( ( 12 == wallLeftColor ) && ( 12 == wallRightColor ) ) {
                        topMap->drawPixel(x,y-1,18); // by default full
                    } else if ( 12 == wallLeftColor ) {
                        topMap->drawPixel(x,y-1,69); // bite left
                    } else if ( 12 == wallRightColor ) {
                        topMap->drawPixel(x,y-1,70); // bite right
                    }
                }
            }            
            else if ( 16 == wallColor ) {
                uint16_t wallDownColor = wallMap->readPixel(x,y+1);
                if ( 255 != wallDownColor ) {
                    topMap->drawPixel(x,y,24);
                }

                uint16_t wallUpColor = wallMap->readPixel(x,y-1);
                if ( 255 == wallUpColor ) {
                    topMap->drawPixel(x,y-1,27);
                }
            }
            else if ( 17 == wallColor ) {
                uint16_t wallDownColor = wallMap->readPixel(x,y+1);
                if ( 255 != wallDownColor ) {
                    topMap->drawPixel(x,y,23);
                }

                uint16_t wallUpColor = wallMap->readPixel(x,y-1);
                if ( 255 == wallUpColor ) {
                    topMap->drawPixel(x,y-1,28);
                }
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

            if ( 12 == wallColor ) {
                // add holes to the walls
                if ( 27 > random(1,100)) {          // probability of hole
                    uint32_t holeversion = 14 + random(0,1);
                    wallMap->drawPixel(x,y,holeversion);
                    //esp_task_wdt_reset();
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
                        floorMap->drawPixel(x,y+1,67);  // goo floor
                        delay(1);
                    } else if ( 3  > random(1,100)) {          // probability of blue fountain
                        // CHECK THE WALLS OF THE SIDE
                        uint16_t wallRightMap = wallMap->readPixel(x+1,y);
                        uint16_t wallLeftMap = wallMap->readPixel(x-1,y);
                        if ( (12 == wallRightMap ) && (12 == wallLeftMap ) ) {
                            floorMap->drawPixel(x,y+1,52);
                            wallMap->drawPixel(x,y,55);
                            topMap->drawPixel(x,y-1,64);
                            delay(1);
                        }
                    } else if ( 3  > random(1,100)) {          // probability of red fountain
                        // CHECK THE WALLS OF THE SIDE
                        uint16_t wallRightMap = wallMap->readPixel(x+1,y);
                        uint16_t wallLeftMap = wallMap->readPixel(x-1,y);
                        if ( (12 == wallRightMap ) && (12 == wallLeftMap ) ) {
                            floorMap->drawPixel(x,y+1,58);
                            wallMap->drawPixel(x,y,61);
                            topMap->drawPixel(x,y-1,64);
                            delay(1);
                        }
                    }
                }
            }
            if ( ( 0 == floorColor ) && ( 255 == objectColor )) {

                if ( 2  > random(1,100)) { 
                    objectsMap->drawPixel(x,y,47); // crates
                } else if ( 2  > random(1,100)) { 
                    objectsMap->drawPixel(x,y,48); // skulls
                }
                
                if ( 30  > random(1,100)) {  // randomize floor
                    uint32_t nuFloor = random(0,8);
                    floorMap->drawPixel(x,y,nuFloor);
                } else if ( 3  > random(1,100)) { 
                    floorMap->drawPixel(x,y,10); // hole
                } else if ( 3  > random(1,100)) { 
                    uint16_t topColor = topMap->readPixel(x,y);
                    if ( 255 == topColor ) { // nothing on top?
                        floorMap->drawPixel(x,y,29); // spikes
                    }
                } else if ( 10  > random(1,100)) {
                    if ( ( 0 == ( x % 4) ) && ( 0 == ( y % 4) ) ) {
                        objectsMap->drawPixel(x,y,65); // don't change ground (transp column foot)
                        wallMap->drawPixel(x,y-1,66); // COLUMNS
                        topMap->drawPixel(x,y-2,68); // COLUMNS
                    }
                }


           }
        }
    }

    lAppLog("Determine the player spawnpoint\n");
    bool keepSearching=true;
    while(keepSearching) {
        delay(15);
        int16_t candidateX=random(0,thisLevel->width-1);
        int16_t candidateY=random(0,thisLevel->height-1);
        uint16_t floorColor = floorMap->readPixel(candidateX,candidateY);
        if ( 0 == floorColor) {
            thisLevel->PlayerBeginX = candidateX;
            thisLevel->PlayerBeginY = candidateY;
            keepSearching=false;
            lAppLog("Player Spawnpoint X: %d Y: %d\n",thisLevel->PlayerBeginX,thisLevel->PlayerBeginY);

            break; // bah
        }
    }
    lAppLog("Dungeon generated!\n");
    //delay(20000);
    thisLevel->valid=true;
    thisLevel->inProgress=false;
    thisLevel->running = true;
    vTaskDelete(NULL);
};
