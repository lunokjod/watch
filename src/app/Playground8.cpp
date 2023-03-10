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
//
#include "Playground8.hpp"
#include "../UI/UI.hpp"

#include <TFT_eSPI.h>

// -s 240x240

// ffmpeg -i rick.gif -filter:v fps=10 -vcodec rawvideo -f rawvideo -ss 2 -pix_fmt rgb565be test.raw
// ffmpeg -i rick.gif -filter:v fps=10 -vcodec rawvideo -f rawvideo -pix_fmt rgb565be test.raw
/*
IO... rgb565be               3            16 <== good one
IO... rgb565le               3            16
IO... bgr565be               3            16
IO... bgr565le               3            16
*/
extern const PROGMEM uint8_t rick_start[] asm("_binary_asset_test_raw_start");
extern const PROGMEM uint8_t rick_end[] asm("_binary_asset_test_raw_end");

PlaygroundApplication8::~PlaygroundApplication8() {
    directDraw=false;
    //if (nullptr != colorBuffer ) { free(colorBuffer); }
}
PlaygroundApplication8::PlaygroundApplication8() {
    directDraw=true;
    //colorBuffer = (uint16_t *)ps_calloc(TOTALPIXELS, sizeof(uint16_t));
    videoPtr = (uint16_t*)rick_start;
    tft->fillScreen(TFT_BLACK);
}
bool PlaygroundApplication8::Tick() {
    static uint32_t frames=0;
    delay(1000/10);
    colorBuffer=videoPtr;
    videoPtr+=TOTALPIXELS;
    frames++;
    if ( videoPtr >= ((uint16_t*)rick_end-TOTALPIXELS) ) {
        //Serial.printf("Video end (frames: %d)\n",frames);
        frames=0;
        videoPtr = (uint16_t*)rick_start;
    }
    unsigned long b4 = millis();
    tft->setSwapBytes(false);
    tft->pushRect(33,53,175,135,colorBuffer);
    unsigned long used=millis()-b4;
    //Serial.printf("FPS: %d\n", 1000/used);
    return false;
}