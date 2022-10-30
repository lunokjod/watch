#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Playground8.hpp"
#include "../UI/UI.hpp"
// -s 240x240

// ffmpeg -i rick.gif -filter:v fps=10 -vcodec rawvideo -f rawvideo -ss 2 -pix_fmt rgb565be test.raw
// ffmpeg -i rick.gif -filter:v fps=10 -vcodec rawvideo -f rawvideo -pix_fmt rgb565be test.raw
/*
IO... rgb565be               3            16 <== good one
IO... rgb565le               3            16
IO... bgr565be               3            16
IO... bgr565le               3            16
*/
extern const uint8_t rick_start[] asm("_binary_asset_test_raw_start");
extern const uint8_t rick_end[] asm("_binary_asset_test_raw_end");

PlaygroundApplication8::~PlaygroundApplication8() {
    directDraw=false;
    //if (nullptr != colorBuffer ) { free(colorBuffer); }
}
PlaygroundApplication8::PlaygroundApplication8() {
    directDraw=true;
    //colorBuffer = (uint16_t *)ps_calloc(TOTALPIXELS, sizeof(uint16_t));
    videoPtr = (uint16_t*)rick_start;
    ttgo->tft->fillScreen(TFT_BLACK);
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
    ttgo->tft->setSwapBytes(false);
    ttgo->tft->pushRect(33,53,175,135,colorBuffer);
    unsigned long used=millis()-b4;
    //Serial.printf("FPS: %d\n", 1000/used);
    return false;
}