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
#include <Arduino.h>
#include <esp_task_wdt.h>
#include "Dungeon.hpp"
#include "../LogView.hpp"

extern TFT_eSPI * tft;
/*
void DrawLevelGeneratorMap(levelDescriptor * currentLevel) {
    delay(10);
    return;
    // show dungeon generation progress
    tft->fillRect(0,0,currentLevel->width,currentLevel->height,TFT_WHITE);
    currentLevel->floorMap->pushSprite(0,0,255);
    currentLevel->objectsMap->pushSprite(0,0,255);
    currentLevel->wallMap->pushSprite(0,0,255);
    currentLevel->topMap->pushSprite(0,0,255);
}*/
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
    
    lLog("Generating paths (horizontal)....\n");
    int horizontalPaths=thisLevel->horizontalPaths;
    while(horizontalPaths>0) {
        int32_t x=2;
        int32_t y=random(2,floorMap->width()-4);
        if ( ( 255 == floorMap->readPixel(x,y))
                &&( 255 == floorMap->readPixel(x,y-1) )
                &&( 255 == floorMap->readPixel(x,y-2) )
                &&( 255 == floorMap->readPixel(x,y-3) )
                &&( 255 == floorMap->readPixel(x,y-4) )
                &&( 255 == floorMap->readPixel(x,y-5) )
                &&( 255 == floorMap->readPixel(x,y-6) )
                
                &&( 255 == floorMap->readPixel(x,y+1) )
                &&( 255 == floorMap->readPixel(x,y+2) )
                &&( 255 == floorMap->readPixel(x,y+3) )
                &&( 255 == floorMap->readPixel(x,y+4) )
                &&( 255 == floorMap->readPixel(x,y+5) )
                &&( 255 == floorMap->readPixel(x,y+6) )
                ) { // put track on empty place
        //if ( 255 == floorMap->readPixel(x,y) ) { // put track on empty place
            floorMap->drawFastHLine(x,y-1,(floorMap->width()-x)-4,0); // floor
            floorMap->drawFastHLine(x,y,(floorMap->width()-x)-4,0); // floor
            floorMap->drawFastHLine(x,y+1,(floorMap->width()-x)-4,0); // floor
            lLog("H Path %d generated X: %d Y: %d\n", horizontalPaths,x,y);
            horizontalPaths--;
            //delay(100);
        }
    }
    

    lLog("Generating paths (vertical)....\n");
    int verticalPaths=thisLevel->verticalPaths;
    while(verticalPaths>0) {
        int32_t x=random(2,floorMap->height()-4);
        int32_t y=2;
        if ( ( 255 == floorMap->readPixel(x,y))
                &&( 255 == floorMap->readPixel(x-1,y) )
                &&( 255 == floorMap->readPixel(x-2,y) )
                &&( 255 == floorMap->readPixel(x-3,y) )
                &&( 255 == floorMap->readPixel(x-4,y) )
                &&( 255 == floorMap->readPixel(x-5,y) )
                &&( 255 == floorMap->readPixel(x-6,y) )
                
                &&( 255 == floorMap->readPixel(x+1,y) )
                &&( 255 == floorMap->readPixel(x+2,y) )
                &&( 255 == floorMap->readPixel(x+3,y) )
                &&( 255 == floorMap->readPixel(x+4,y) )
                &&( 255 == floorMap->readPixel(x+5,y) )
                &&( 255 == floorMap->readPixel(x+6,y) )
                ) { // put track on empty place
            floorMap->drawFastVLine(x-1,y,(floorMap->height()-4)-y,0); // floor
            floorMap->drawFastVLine(x,y,(floorMap->height()-4)-y,0); // floor
            floorMap->drawFastVLine(x+1,y,(floorMap->height()-4)-y,0); // floor
            lLog("V Path %d generated X: %d Y: %d\n", verticalPaths,x,y);
            verticalPaths--;
            //delay(100);
        }
    }
    lLog("Generating rooms....\n");
    // build rooms
    int rooms=thisLevel->rooms;

    while(rooms>0) {
        int32_t w=random(thisLevel->minRoomSize,thisLevel->maxRoomSize+1);
        int32_t h=random(thisLevel->minRoomSize,thisLevel->maxRoomSize+1);
        int32_t x=random(4,((thisLevel->width-4)-w));
        int32_t y=random(4,((thisLevel->height-4)-h));
        uint16_t leftFloorColor = floorMap->readPixel(x-3,y+(h/2)); // floor on left?
        uint16_t rightFloorColor = floorMap->readPixel((x+w)+2,y+(h/2)); // floor on right?
        uint16_t topFloorColor = floorMap->readPixel(x+(w/2),y-3); // floor up?
        uint16_t downFloorColor = floorMap->readPixel(x+(w/2),(y+h)+2); // floor down?
        bool isGoodPlace=false;
        if ( 0 == leftFloorColor ) { isGoodPlace=true; } // closer to other room or path?
        if ( 0 == rightFloorColor ) { isGoodPlace=true; }
        if ( 0 == topFloorColor ) { isGoodPlace=true; }
        if ( 0 == downFloorColor ) { isGoodPlace=true; }
        // try to get empty floor space with border
        for(int16_t pey=y-2;pey<(y+h)+2;pey++) {
            if ( false == isGoodPlace ) { break; }
            for(int16_t pex=x-2;pex<(x+w)+2;pex++) {
                if ( 255 != floorMap->readPixel(pex,pey) ) {
                    isGoodPlace=false;
                    break;
                }
            }
        }
        if ( isGoodPlace ) { // room have enough space and have near path or room
            floorMap->fillRoundRect(x,y,w,h,thisLevel->roomRadius,0); // mark as floor

            // goth style (centered entry x3 floor width)
            if ( 0 == leftFloorColor ) { 
                floorMap->fillRect(x-2,y+(h/2)-1,2,3,0);
                //floorMap->drawPixel(x-1,y+(h/2),0);
            } // draw the left entry
            if ( 0 == rightFloorColor ) {
                floorMap->fillRect((x+w),y+(h/2)-1,2,3,0);
                //floorMap->drawPixel((x+w),y+(h/2),0);
            } // draw the right entry
            if ( 0 == topFloorColor ) {
                floorMap->fillRect((x+(w/2))-1,y-2,3,2,0);
                //floorMap->drawPixel(x+(w/2),y-1,0); 
            } // draw the up entry
            if ( 0 == downFloorColor ) {
                floorMap->fillRect(x+(w/2)-1,(y+h),3,2,0);
                //floorMap->drawPixel(x+(w/2),(y+h),0);
            } // draw the down entry


            lLog("Room %d generated X: %d Y: %d H: %d W: %d\n", rooms,x,y,h,w);
            rooms--;
            //delay(10);
        }
    }
    
    lLog("Removing small holes...\n");
    // remove stupid holes using colindant values
    for(int32_t y=0;y<floorMap->height();y+=4) {
        for(int32_t x=0;x<floorMap->width();x+=4) {
            uint16_t color = floorMap->readPixel(x,y);
            if ( 255 == color ) { // find empty
                uint16_t colorUp = floorMap->readPixel(x,y-1);
                uint16_t colorDown = floorMap->readPixel(x,y+1);
                if ( ( 0 == colorUp ) && ( 0 == colorDown ) ) {
                    floorMap->drawPixel(x,y,0); // define floor
                    //delay(100);
                    continue;
                }
                uint16_t colorLeft = floorMap->readPixel(x-1,y);
                uint16_t colorRight = floorMap->readPixel(x+1,y);
                if ( ( 0 == colorLeft ) && ( 0 == colorRight ) ) {
                    floorMap->drawPixel(x,y,0); // new updown path
                    //delay(100);
                    continue;
                }
            }
        }
    }
    lLog("Generating walls (up/down)....\n");
    // put up/down walls 
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
            uint16_t floorColor = floorMap->readPixel(x,y);
            if ( 0 == floorColor ) { // floor detected
                uint16_t floorUpColor = floorMap->readPixel(x,y-1);
                uint16_t colorWallUp = wallMap->readPixel(x,y-1);
                if ( ( 255 == floorUpColor ) && (255 == colorWallUp)) { // empty space?
                    //lLog("Up wall X: %d Y: %d ",x,y);
                    
                    wallMap->drawPixel(x,y,12); // put wall here!
                }
                uint16_t colorDown = floorMap->readPixel(x,y+1);
                if ( 255 == colorDown ) { // empty space
                    //lLog("Down wall X: %d Y: %d ",x,y);
                    wallMap->drawPixel(x,y+1,12); // put wall here!
                }
            }
            // remove double walls
            uint16_t wallColor = wallMap->readPixel(x,y);
            uint16_t wallUpColor = wallMap->readPixel(x,y-1);
            //uint16_t wallDownColor = wallMap->readPixel(x,y+1);
            if ( ( 12 == wallColor ) && (12 == wallUpColor)) {
            //if ( ( 12 == wallColor ) && ( (12 == wallUpColor) || (12 == wallDownColor) ) ) {
                floorMap->drawPixel(x,y,0); // redefine as floor
                wallMap->drawPixel(x,y,255); // remove wall (use only the Up wall)
            }
        }
    }
    //delay(1000);

    lLog("Generating walls (left/right)....\n");
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

            // end the walls between horizontal/vertical walls
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

                /*
                //UP AND DOWN
                uint16_t colorUpWall = wallMap->readPixel(x,y-1); // on WALL
                uint16_t colorUp = floorMap->readPixel(x,y-1); // on floor
                if ( ( 12 == colorUpWall )&&( 255 == colorUp )) { // empty space
                    //wallMap->drawPixel(x-1,y,17); // put wall here!
                }*/
            }
        }
    }
    //delay(1000);

    lLog("Inner columns...\n");
    int current=thisLevel->columns;
    // change some tiles to add more visual richness
    while(current > 0) {
        int16_t x=random(thisLevel->width/3)*3;
        int16_t y=random(thisLevel->height/3)*3;
        if ( ( 0 == ( x % 3) ) && ( 0 == ( y % 3) ) ) {

            uint16_t floorColor = floorMap->readPixel(x,y);
            if ( 0 != floorColor ) { continue; }
            
            //uint16_t wallColor = wallMap->readPixel(x,y);
            //uint16_t topColor = topMap->readPixel(x,y);

            //uint16_t objectColor = objectsMap->readPixel(x,y);
            uint16_t wallUpColor = wallMap->readPixel(x,y-1);
            uint16_t topColor = topMap->readPixel(x,y-2);
            uint16_t floorUpColor = floorMap->readPixel(x,y-1);
            if ( ( 255 == wallUpColor ) && ( 255 == topColor ) && (0 == floorUpColor) ) {
                objectsMap->drawPixel(x,y,65); // don't change ground (transp column foot)
                wallMap->drawPixel(x,y-1,66); // column wall
                topMap->drawPixel(x,y-2,68); // column top
                current--;
                //delay(10);
            }
        }
    }
    //delay(1000);

   
    lLog("Generating wall decorations....\n");
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
                    } else {
                        topMap->drawPixel(x,y-1,18); // fuck! why?
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
    //delay(1000);

    
    lLog("Details...\n");
    // change some tiles to add more visual richness
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
            //random(255); // get entropy
            uint16_t floorColor = floorMap->readPixel(x,y);
            uint16_t wallColor = wallMap->readPixel(x,y);
            uint16_t topColor = topMap->readPixel(x,y);
            uint16_t objectColor = objectsMap->readPixel(x,y);

            if ( 12 == wallColor ) {
                // only on internal faces (if no floor, no decoration)
                uint16_t floorDownColor = floorMap->readPixel(x,y+1);
                if ( 0 == floorDownColor ) { // the down is floor? (inner)
                    if ( 32  > random(1,100)) {          // probability of wall column
                        if ( ( 0 == ( x % 4) ) && ( 0 == ( y % 4) ) ) { // aligned wit inner columns
                            // only if have walls to the sides
                            uint16_t wallRightColor = wallMap->readPixel(x+1,y);
                            uint16_t wallLeftColor = wallMap->readPixel(x-1,y);
                            if ( ( 12 == wallRightColor )&&( 12 == wallLeftColor )) {
                                floorMap->drawPixel(x,y+1,49);
                                wallMap->drawPixel(x,y,50);
                                topMap->drawPixel(x,y-1,51);
                                //delay(1);
                            }
                        }
                    } else if ( 8 > random(1,100)) {
                        wallMap->drawPixel(x,y,43);             // green flag
                        //delay(1);
                    } else if ( 8 > random(1,100)) {
                        wallMap->drawPixel(x,y,44);             // blue flag
                        //delay(1);
                    } else if ( 8 > random(1,100)) {
                        wallMap->drawPixel(x,y,45);             // red flag
                        //delay(1);
                    } else if ( 8 > random(1,100)) {
                        wallMap->drawPixel(x,y,46);             // yellow flag
                        //delay(1);
                    } else if ( 6  > random(1,100)) {          // probability of goo
                        wallMap->drawPixel(x,y,42);
                        floorMap->drawPixel(x,y+1,67);  // goo floor
                        //delay(1);
                    } else if ( 9  > random(1,100)) {          // probability of blue fountain
                        // CHECK THE WALLS OF THE SIDE
                        uint16_t wallRightMap = wallMap->readPixel(x+1,y);
                        uint16_t wallLeftMap = wallMap->readPixel(x-1,y);
                        if ( (12 == wallRightMap ) && (12 == wallLeftMap ) ) {

                            // CHECK THE columns
                            uint16_t topMapColor = topMap->readPixel(x,y);
                            uint16_t topDownMap = topMap->readPixel(x,y+1);
                            if ( ( 255 == topMapColor ) && ( 255 == topDownMap ) ) {
                                floorMap->drawPixel(x,y+1,52);
                                wallMap->drawPixel(x,y,55);
                                topMap->drawPixel(x,y-1,64);
                            }
                        }
                    } else if ( 9  > random(1,100)) {          // probability of red fountain
                        // CHECK THE WALLS OF THE SIDE
                        uint16_t wallRightMap = wallMap->readPixel(x+1,y);
                        uint16_t wallLeftMap = wallMap->readPixel(x-1,y);
                        //@TODO check fountains to avoid glitch
                        if ( (12 == wallRightMap ) && (12 == wallLeftMap ) ) {
                            // CHECK THE columns
                            uint16_t topDownMap = topMap->readPixel(x,y+1);
                            if ( ( 255 == topColor ) && ( 255 == topDownMap ) ) {
                                floorMap->drawPixel(x,y+1,58);
                                wallMap->drawPixel(x,y,61);
                                topMap->drawPixel(x,y-1,64);
                            }
                        }
                    }
                } 
                uint16_t wallColor = wallMap->readPixel(x,y);
                if ( 12 == wallColor ) {
                    if ( 28 > random(1,100)) {          // probability of hole
                        uint32_t holeversion = 14 + random(0,1);
                        wallMap->drawPixel(x,y,holeversion);
                    }
                }
            }

            floorColor = floorMap->readPixel(x,y);
            objectColor = objectsMap->readPixel(x,y);
            if ( ( 0 == floorColor ) && ( 255 == objectColor )) {
                random(255);
                if ( 2  > random(1,100)) { 
                    objectsMap->drawPixel(x,y,48); // skulls
                } else if ( 3  > random(1,100)) { 
                    floorMap->drawPixel(x,y,10); // hole
                } else if ( 6 > random(1,100)) { 
                    uint16_t topColor = topMap->readPixel(x,y);
                    uint16_t wallColor = wallMap->readPixel(x,y);
                    uint16_t objectColor = objectsMap->readPixel(x,y);
                    if ( ( 255 == topColor )&&( 255 == wallColor )&&( 255 == objectColor )) { // nothing on top?
                        floorMap->drawPixel(x,y,29); // spikes
                        //delay(5);
                        //random(255);
                    }
                }

           }
        }
    }
    //delay(1000);
    lLog("Crates...\n");
    //uint8_t 
    current=thisLevel->cratesNumber;
    // change some tiles to add more visual richness
    while(current > 0) {
        //delay(5);
        //random(255);
        int16_t x=random(0,thisLevel->width-1);
        int16_t y=random(0,thisLevel->height-1);

        uint16_t floorColor = floorMap->readPixel(x,y);
        uint16_t wallColor = wallMap->readPixel(x,y);
        uint16_t topColor = topMap->readPixel(x,y);
        uint16_t objectColor = objectsMap->readPixel(x,y);
        if ( ( 0 == floorColor ) && ( 255 == objectColor )
                && ( 255 == topColor ) && ( 255 == wallColor ) ) {
            objectsMap->drawPixel(x,y,47);
            current--;
        }
    }
    //delay(1000);
    lLog("Chests...\n");
    current=thisLevel->chestNumber;
    // change some tiles to add more visual richness
    while(current > 0) {
        int16_t x=random(0,thisLevel->width-1);
        int16_t y=random(0,thisLevel->height-1);

        uint16_t floorColor = floorMap->readPixel(x,y);
        uint16_t wallColor = wallMap->readPixel(x,y);
        uint16_t topColor = topMap->readPixel(x,y);
        uint16_t objectColor = objectsMap->readPixel(x,y);
        if ( ( 0 == floorColor ) && ( 255 == objectColor )
                && ( 255 == topColor ) && ( 255 == wallColor ) ) {
                bool valid=true;
                for(int16_t ay=x-1;ay<y+1;ay++) {
                    if ( false == valid ) { break; }
                    for(int16_t ax=x-1;ax<x+1;ax++) {
                        if ( 255 != objectsMap->readPixel(ax,ay) ) {
                            valid=false;
                            break;
                        }
                    }
                }
            if ( valid ) {
                objectsMap->drawPixel(x,y,33);
                current--;
                //delay(100);
            }

        }
    }
    //delay(1000);
    lLog("Coins...\n");
    current=thisLevel->coinNumber;
    // change some tiles to add more visual richness
    while(current > 0) {
        int16_t x=random(0,thisLevel->width-1);
        int16_t y=random(0,thisLevel->height-1);

        uint16_t floorColor = floorMap->readPixel(x,y);
        uint16_t wallColor = wallMap->readPixel(x,y);
        uint16_t topColor = topMap->readPixel(x,y);
        uint16_t objectColor = objectsMap->readPixel(x,y);
        if ( ( 0 == floorColor ) && ( 255 == objectColor )
                && ( 255 == topColor ) && ( 255 == wallColor ) ) {
            objectsMap->drawPixel(x,y,71);
            current--;
            //delay(10);
        }
    }
    
    if ( ( 0 == thisLevel->PlayerBeginX ) && ( 0 == thisLevel->PlayerBeginY ) ) {
        lLog("Determine the player spawnpoint\n");
        bool keepSearching=true;
        while(keepSearching) {
            int16_t candidateX=random(0,thisLevel->width-1);
            int16_t candidateY=random(0,thisLevel->height-1);
            uint16_t floorColor = floorMap->readPixel(candidateX,candidateY);
            if ( 0 == floorColor) { // is normal floor
                thisLevel->PlayerBeginX = candidateX;
                thisLevel->PlayerBeginY = candidateY;
                keepSearching=false;
                lLog("Player Spawnpoint X: %d Y: %d\n",thisLevel->PlayerBeginX,thisLevel->PlayerBeginY);

                break; // gotcha
            }
            //delay(1);
        }
    }
    //delay(1000);
    lLog("Randomize floor tiles...\n");
    for(int32_t y=0;y<floorMap->height();y++) {
        for(int32_t x=0;x<floorMap->width();x++) {
                uint16_t floorColor = floorMap->readPixel(x,y);
                if ( 0 == floorColor ) {
                    //if ( 50  > random(1,100)) {  // randomize floor
                        uint32_t nuFloor = random(0,8);
                        floorMap->drawPixel(x,y,nuFloor);
                    //}
                }
        }
    }

    thisLevel->player = new ElfMalePlayer();        // male elf by default
    thisLevel->player->SetPos(thisLevel->PlayerBeginX,thisLevel->PlayerBeginY);
    lLog("Dungeon generated!\n");
    //delay(1000);
    thisLevel->valid=true;
    thisLevel->inProgress=false;
    thisLevel->running = true;
    vTaskDelete(NULL);
};
