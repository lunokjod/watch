#include <Arduino.h>
#include "Playground.hpp"
#include "../static/img_sample_120.c"
#include "../static/img_sample2_120.c"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/UI.hpp"
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo library shit ;)

PlaygroundApplication::~PlaygroundApplication() {
    directDraw = false;

    if ( nullptr != imageTest ) {
        delete imageTest;
        imageTest = nullptr;
    }
    if ( nullptr != imageTest2 ) {
        delete imageTest2;
        imageTest2 = nullptr;
    }
    if ( nullptr != imageTest3 ) {
        delete imageTest3;
        imageTest3 = nullptr;
    }
    if ( nullptr != buffer ) {
        delete buffer;
        buffer = nullptr;
    }
}
PlaygroundApplication::PlaygroundApplication() {
    directDraw=true;
    buffer = new CanvasWidget(TFT_HEIGHT,TFT_WIDTH);
    buffer->canvas->fillSprite(TFT_BLACK);
    buffer->canvas->pushSprite(0,0); // directDraw allow to get control about SPI TFT dumps

    imageTest = new CanvasWidget(img_sample_120.height, img_sample_120.width);
    imageTest->canvas->setSwapBytes(true);
    imageTest->canvas->pushImage(0,0,img_sample_120.width,img_sample_120.height,(uint16_t *)img_sample_120.pixel_data);
    imageTest->canvas->setSwapBytes(false);
    
    imageTest2 = new CanvasWidget(img_sample_120.height, img_sample_120.width);

    imageTest3 = new CanvasWidget(img_sample2_120.height, img_sample2_120.width);
    imageTest3->canvas->setSwapBytes(true);
    imageTest3->canvas->pushImage(0,0,img_sample2_120.width,img_sample2_120.height,(uint16_t *)img_sample2_120.pixel_data);
    imageTest3->canvas->setSwapBytes(false);



}

int16_t carrousel = 0;
bool GyroTest(int x,int y, int cx, int cy, int angle, int step, void* payload) {
    PlaygroundApplication * obj = (PlaygroundApplication * )payload;
    uint8_t border = 5;

    if ( int(angle) == ((120+carrousel)%360) ) {
        int32_t canx = x-(obj->imageTest->canvas->width()/2);
        int32_t cany = y-(obj->imageTest->canvas->height()/2);
        int32_t canh = obj->imageTest->canvas->height();
        int32_t canw = obj->imageTest->canvas->width();
        if ( directDraw ) {
            obj->imageTest->canvas->pushSprite(canx, cany);
            ttgo->tft->fillRect(canx-border,cany,border,canh,TFT_BLACK);
            ttgo->tft->fillRect(canx-border,cany-border,canw+(border*2),border,TFT_BLACK);
            ttgo->tft->fillRect(canx+canw,cany,border,canh,TFT_BLACK);
            ttgo->tft->fillRect(canx-border,cany+canh,canw+(border*2),border,TFT_BLACK);
        } else {
            obj->imageTest->DrawTo(obj->buffer->canvas,canx,cany);
        }
    }
    if ( int(angle) == ((240+carrousel)%360) ) {
        int32_t canx = x-(obj->imageTest2->canvas->width()/2);
        int32_t cany = y-(obj->imageTest2->canvas->height()/2);
        int32_t canh = obj->imageTest2->canvas->height();
        int32_t canw = obj->imageTest2->canvas->width();
        if ( directDraw ) {
            obj->imageTest2->canvas->pushSprite(canx, cany);
            ttgo->tft->fillRect(canx-border,cany,border,canh,TFT_BLACK);
            ttgo->tft->fillRect(canx-border,cany-border,canw+(border*2),border,TFT_BLACK);
            ttgo->tft->fillRect(canx+canw,cany,border,canh,TFT_BLACK);
            ttgo->tft->fillRect(canx-border,cany+canh,canw+(border*2),border,TFT_BLACK);

        } else {
            obj->imageTest2->DrawTo(obj->buffer->canvas,canx,cany);
        }
    }
    if ( int(angle) == ((carrousel)%360)) {
        int32_t canx = x-(obj->imageTest3->canvas->width()/2);
        int32_t cany = y-(obj->imageTest3->canvas->height()/2);
        int32_t canh = obj->imageTest3->canvas->height();
        int32_t canw = obj->imageTest3->canvas->width();
        if ( directDraw ) {
            obj->imageTest3->canvas->pushSprite(canx, cany);

            ttgo->tft->fillRect(canx-border,cany,border,canh,TFT_BLACK);
            ttgo->tft->fillRect(canx-border,cany-border,canw+(border*2),border,TFT_BLACK);
            ttgo->tft->fillRect(canx+canw,cany,border,canh,TFT_BLACK);
            ttgo->tft->fillRect(canx-border,cany+canh,canw+(border*2),border,TFT_BLACK);
        } else {
            obj->imageTest3->DrawTo(obj->buffer->canvas,canx,cany);
        }
    }
    return true;
}
bool PlaygroundApplication::Tick() {
    unsigned long b4 = millis();
    if ( false == directDraw ) {
        buffer->canvas->fillSprite(TFT_BLACK);
    }
    for (int x=0;x<img_sample_120.width;x++){
        for (int y=0;y<img_sample_120.height;y++){
                uint16_t originalPixel16 = imageTest->canvas->readPixel(x,y);
                uint16_t originalPixel16_2 = imageTest3->canvas->readPixel(x,y);
                uint16_t color16 = canvas->alphaBlend(currentAlpha,originalPixel16,originalPixel16_2);
                imageTest2->canvas->drawPixel(x,y,color16);
        }
    }
    
    if ( alphaDirection ) { currentAlpha+=16; }
    else { currentAlpha-=16; }
    if ( currentAlpha > 255 ) {
        currentAlpha = 255;
        alphaDirection=(!alphaDirection);
    } else if ( currentAlpha < 0 ) {
        currentAlpha = 0;
        alphaDirection=(!alphaDirection);
    }        
    
    DescribeCircle(120,120,80,GyroTest,this);
    carrousel=(carrousel+3)%360;

    if ( directDraw ) {
        unsigned long delta=millis()-b4;
        unsigned long fps = 1000/delta;
        //Serial.printf("FPS: %u\n",fps);
    } else {
        buffer->DrawTo(canvas,0,0,-1);
    }
    return true;
}
