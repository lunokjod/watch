#include "Dungeon.hpp"
#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include "LogView.hpp"   // for lLog functions


#include "../static/Dungeon/img_splash_dungeon.c" // splash

// all tiles
#include "../static/Dungeon/floor_1.c"
#include "../static/Dungeon/floor_2.c"
#include "../static/Dungeon/floor_3.c"
#include "../static/Dungeon/floor_4.c"
#include "../static/Dungeon/floor_5.c"
#include "../static/Dungeon/floor_6.c"
#include "../static/Dungeon/floor_7.c"
#include "../static/Dungeon/floor_8.c"
#include "../static/Dungeon/floor_ladder.c"
#include "../static/Dungeon/floor_ladder2.c"
#include "../static/Dungeon/hole.c"

#include "../static/Dungeon/wall_right.c"
#include "../static/Dungeon/wall_mid.c"
#include "../static/Dungeon/wall_left.c"
#include "../static/Dungeon/wall_hole_1.c"
#include "../static/Dungeon/wall_hole_2.c"

#include "../static/Dungeon/floor_spikes_anim_f0.c"
#include "../static/Dungeon/floor_spikes_anim_f1.c"
#include "../static/Dungeon/floor_spikes_anim_f2.c"
#include "../static/Dungeon/floor_spikes_anim_f3.c"

#include "../static/Dungeon/chest_empty_open_anim_f0.c"
#include "../static/Dungeon/chest_empty_open_anim_f1.c"
#include "../static/Dungeon/chest_empty_open_anim_f2.c"
#include "../static/Dungeon/chest_full_open_anim_f0.c"
#include "../static/Dungeon/chest_full_open_anim_f1.c"
#include "../static/Dungeon/chest_full_open_anim_f2.c"
#include "../static/Dungeon/chest_mimic_open_anim_f0.c"
#include "../static/Dungeon/chest_mimic_open_anim_f1.c"
#include "../static/Dungeon/chest_mimic_open_anim_f2.c"

#include "../static/Dungeon/wall_side_front_right.c"
#include "../static/Dungeon/wall_side_front_left.c"

#include "../static/Dungeon/wall_top_mid.c"
#include "../static/Dungeon/wall_inner_corner_l_top_rigth.c"
#include "../static/Dungeon/wall_inner_corner_l_top_left.c"
#include "../static/Dungeon/wall_corner_bottom_right.c"
#include "../static/Dungeon/wall_corner_bottom_left.c"
#include "../static/Dungeon/wall_side_mid_left.c"
#include "../static/Dungeon/wall_side_mid_right.c"


const unsigned char * tileSets[] = {
    floor_1.pixel_data,
    floor_2.pixel_data,
    floor_3.pixel_data,
    floor_4.pixel_data,
    floor_5.pixel_data,
    floor_6.pixel_data,
    floor_7.pixel_data,
    floor_8.pixel_data,
    floor_ladder.pixel_data, // 8
    floor_ladder2.pixel_data,
    hole.pixel_data, // 10

    wall_right.pixel_data,
    wall_mid.pixel_data, // 12
    wall_left.pixel_data,
    wall_hole_1.pixel_data,
    wall_hole_2.pixel_data,
    wall_side_front_right.pixel_data,
    wall_side_front_left.pixel_data, // 17
    wall_top_mid.pixel_data,
    wall_inner_corner_l_top_rigth.pixel_data,
    wall_inner_corner_l_top_left.pixel_data,
    wall_corner_bottom_right.pixel_data, // 21
    wall_corner_bottom_left.pixel_data,
    wall_side_mid_left.pixel_data,
    wall_side_mid_right.pixel_data,

    floor_spikes_anim_f0.pixel_data, // 25
    floor_spikes_anim_f1.pixel_data,
    floor_spikes_anim_f2.pixel_data,
    floor_spikes_anim_f3.pixel_data,

    chest_empty_open_anim_f0.pixel_data,
    chest_empty_open_anim_f1.pixel_data,
    chest_empty_open_anim_f2.pixel_data,
    chest_full_open_anim_f0.pixel_data,
    chest_full_open_anim_f1.pixel_data,
    chest_full_open_anim_f2.pixel_data,
    chest_mimic_open_anim_f0.pixel_data,
    chest_mimic_open_anim_f1.pixel_data,
    chest_mimic_open_anim_f2.pixel_data,
};

bool dungeonGameGridGenerated=false;

uint8_t *dungeonGameGrid = nullptr;
uint8_t *dungeonGameGridHigh = nullptr;

void DungeonGameLevelGenerator(void *data) {
    dungeonGameGridGenerated=false;
    if ( nullptr != dungeonGameGrid ) { free(dungeonGameGrid); }
    if ( nullptr != dungeonGameGridHigh ) { free(dungeonGameGrid); }

    dungeonGameGrid = (uint8_t*)ps_malloc(DungeonGameApplication::GridSizeH*DungeonGameApplication::GridSizeW);
    dungeonGameGridHigh = (uint8_t*)ps_malloc(DungeonGameApplication::GridSizeH*DungeonGameApplication::GridSizeW);

    // don't use first line (used for props in high grid)
    for ( size_t off=0;off<DungeonGameApplication::GridSizeW;off++) {
        dungeonGameGrid[off] = 255;
    }
    // generate random floor
    for ( size_t off=DungeonGameApplication::GridSizeW;off<(DungeonGameApplication::GridSizeH*DungeonGameApplication::GridSizeW);off++) {
        dungeonGameGrid[off] = random(0,7);
    }

    // clean high map
    for ( size_t off=0;off<(DungeonGameApplication::GridSizeH*DungeonGameApplication::GridSizeW);off++) {
        dungeonGameGridHigh[off] = 255;
    }

    for ( size_t x=0;x<DungeonGameApplication::GridSizeW;x++) {
        uint8_t *ptr=dungeonGameGridHigh+(DungeonGameApplication::GridSizeW+x);
        *ptr=12; // high wall
        ptr=dungeonGameGridHigh+(x+(DungeonGameApplication::GridSizeW*(DungeonGameApplication::GridSizeH-1)));
        *ptr=12; // lower wall
    }
    for ( size_t y=0;y<DungeonGameApplication::GridSizeH-1;y++) {
        uint8_t *ptr=dungeonGameGridHigh+((y*DungeonGameApplication::GridSizeW));
        *ptr=24; // left wall 
        ptr=dungeonGameGridHigh+((y*DungeonGameApplication::GridSizeW)+(DungeonGameApplication::GridSizeW-1));
        *ptr=23; // right wall
    }

    // top wall props
    for ( size_t x=0;x<DungeonGameApplication::GridSizeW;x++) {
        dungeonGameGridHigh[x] = 18;
        dungeonGameGridHigh[((DungeonGameApplication::GridSizeH-2)*DungeonGameApplication::GridSizeW)+x] = 18;
    }
    dungeonGameGridHigh[((DungeonGameApplication::GridSizeH-2)*DungeonGameApplication::GridSizeW)] = 22;
    dungeonGameGridHigh[((DungeonGameApplication::GridSizeH-2)*DungeonGameApplication::GridSizeW)+(DungeonGameApplication::GridSizeW-1)] = 21;

    dungeonGameGridGenerated=true;
    lAppLog("Dungeon generated!\n");
    vTaskDelete(NULL);
};

DungeonGameApplication::~DungeonGameApplication() {
    delete gameScreen;
    delete gameScreen2;
    free(dungeonGameGrid);
    free(dungeonGameGridHigh);
    dungeonGameGrid=nullptr;
    dungeonGameGridHigh=nullptr;
    directDraw=false;
}

DungeonGameApplication::DungeonGameApplication() {
    directDraw=true;
    dungeonGameGridGenerated=false;
    gameScreen=new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    gameScreen2=new CanvasWidget(TFT_WIDTH,TFT_HEIGHT);
    offsetX=0;
    offsetY=0;
    xTaskCreate(DungeonGameLevelGenerator, "", LUNOKIOT_APP_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), NULL);

    // use canvas as splash screen
    canvas->setSwapBytes(true);
    canvas->pushImage(0,0,img_splash_dungeon.width,img_splash_dungeon.height, (uint16_t *)img_splash_dungeon.pixel_data);
    canvas->setSwapBytes(false);
}

bool DungeonGameApplication::Tick() {
    if (false == dungeonGameGridGenerated) {
        lAppLog("Waiting map generator...\n");
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
            int16_t disX=(offsetX%tileW);
            int16_t disY=(offsetY%tileH);

            // must be inverted
            size_t tileOffsetX=((offsetX/tileW))*-1;
            size_t tileOffsetY=((offsetY/tileH))*-1;
            //lAppLog("[Touch] END offX: %d offY: %d DX: %d DY: %d TX: %d TY: %d\n",offsetX,offsetY,disX,disY,tileOffsetX,tileOffsetY);

            // redraw to match with new displacement
            gameScreen->canvas->fillSprite(TFT_BLACK);
            gameScreen->canvas->setSwapBytes(true);

            for(int16_t y=disY;y<(gameScreen->canvas->height()+disY);y+=tileH) {
                for(int16_t x=disX;x<(gameScreen->canvas->width()+disX);x+=tileW) {
                    int16_t tileX = (tileOffsetX+((x-disX)/tileW));
                    int16_t tileY = (tileOffsetY+((y-disY)/tileH));
                    // check bounds
                    if ( ( tileX >= 0 ) && ( tileY >= 0 ) && ( tileX < GridSizeW ) && ( tileY < GridSizeH ) ) {
                        uint8_t *tileValue = dungeonGameGrid+((tileY*GridSizeW)+tileX);
                        if ( *tileValue < (sizeof(tileSets)/sizeof(tileSets[0])) ) {
                            gameScreen->canvas->pushImage(x,y,tileW,tileH, (uint16_t *)tileSets[*tileValue]);
                        } else {
                            //Serial.printf("WTF %d\n",*tileValue);
                            //gameScreen->canvas->fillRect(x,y,tileW,tileH,TFT_RED);
                        }
                    } else {
                        gameScreen->canvas->fillCircle(x+(tileW/2),y+(tileH/2),2,canvas->color24to16(0x000722));
                    }
                }
            }
            gameScreen->canvas->setSwapBytes(false);

            // second layer
            
            // http://www.rinkydinkelectronics.com/calc_rgb565.php
            //const uint32_t MASK_COLOR2=0x1FF8; // GIMP COLOR 0xff00ff
            //const uint32_t MASK_COLOR2_INV=0xF81F;
            //const uint32_t MASK_COLOR_SHIT=0xF01E;
            //#define TFT_PINK        0xFE19      /* 255, 192, 203 */ 
            gameScreen2->canvas->fillSprite(TFT_BLACK);
            //gameScreen2->canvas->setSwapBytes(true);
            for(int16_t y=disY;y<(gameScreen->canvas->height()+disY);y+=tileH) {
                for(int16_t x=disX;x<(gameScreen->canvas->width()+disX);x+=tileW) {
                    int16_t tileX = (tileOffsetX+((x-disX)/tileW));
                    int16_t tileY = (tileOffsetY+((y-disY)/tileH));
                    // check bounds
                    if ( ( tileX >= 0 ) && ( tileY >= 0 ) && ( tileX < GridSizeW ) && ( tileY < GridSizeH ) ) {
                        uint8_t *tileValue = dungeonGameGridHigh+((tileY*GridSizeW)+tileX);
                        if ( *tileValue < (sizeof(tileSets)/sizeof(tileSets[0])) ) {
                            gameScreen2->canvas->pushImage(x,y,tileW,tileH, (uint16_t *)tileSets[*tileValue]);
                        }
                    }
                }
            }
            //gameScreen2->canvas->setSwapBytes(false);
            
            gameScreen->canvas->setPivot(0,0);
            gameScreen2->canvas->setPivot(0,0);
            //gameScreen->canvas->setSwapBytes(true);
            gameScreen2->canvas->pushRotated(gameScreen->canvas,0,TFT_BLACK);
            gameScreen->canvas->pushSprite(0,0); // don't use mask color
            //gameScreen->canvas->setSwapBytes(false);
            return false;
        }
    }

    //if ( millis() < nextRefresh ) { return false; }
    // yeah draw the current
    //nextRefresh = millis()+(1000/4);
    return false;
}
