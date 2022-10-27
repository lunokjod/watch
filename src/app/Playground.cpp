#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Playground.hpp"
#include "../static/img_sample_120.c"
#include "../static/img_sample2_120.c"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/UI.hpp"
#include "../static/img_sample_400.c"


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
    buffer = new CanvasWidget(TFT_HEIGHT,TFT_WIDTH);
    buffer->canvas->fillSprite(TFT_BLACK);

    imageTest = new CanvasWidget(img_sample_120.height, img_sample_120.width);
    imageTest->canvas->setSwapBytes(true);
    imageTest->canvas->pushImage(0,0,img_sample_120.width,img_sample_120.height,(uint16_t *)img_sample_120.pixel_data);
    imageTest->canvas->setSwapBytes(false);
    
    imageTest2 = new CanvasWidget(img_sample_120.height, img_sample_120.width);

    imageTest3 = new CanvasWidget(img_sample2_120.height, img_sample2_120.width);
    imageTest3->canvas->setSwapBytes(true);
    imageTest3->canvas->pushImage(0,0,img_sample2_120.width,img_sample2_120.height,(uint16_t *)img_sample2_120.pixel_data);
    imageTest3->canvas->setSwapBytes(false);


    tft->setWindow(0,0,TFT_WIDTH,TFT_HEIGHT);
    //tft->setAddrWindow(0,0,TFT_WIDTH,TFT_HEIGHT);
    buffer->canvas->pushSprite(0,0);

    buffer->canvas->fillSprite(TFT_BLACK);
    tft->setWindow(100,50,80,TFT_HEIGHT);
    //tft->setWindow(100,50,80,TFT_HEIGHT);
    //uint16_t *colorBuffer=(uint16_t *)ps_calloc(100*100,sizeof(uint16_t));
    //tft->pushRect(50,50,100,100,colorBuffer);
    //free(colorBuffer);
    //tft->setAddrWindow(100,100,80,128);
    //buffer->canvas->pushSprite(0,0);


    imageDeepTest = new CanvasZWidget(img_sample_400.height, img_sample_400.width,0.5);
    imageDeepTest->canvas->setSwapBytes(true);
    imageDeepTest->canvas->pushImage(0,0,img_sample_400.width,img_sample_400.height,(uint16_t *)img_sample_400.pixel_data);
    imageDeepTest->canvas->setSwapBytes(false);

}

// typedef std::function<bool (int,int, int, int, double, int, void*)> DescribeCircleCallback;
int16_t carrousel = 0;
bool GyroTest(int x,int y, int cx, int cy, int angle, int step, void* payload) {
    PlaygroundApplication * obj = (PlaygroundApplication * )payload;
    
    
    if ( int(angle) == ((120+carrousel)%360) ) {
        obj->buffer->canvas->fillRect(
                            (x-(obj->imageTest->canvas->width()/2))-3,
                            (y-(obj->imageTest->canvas->height()/2))-3,
                            obj->imageTest->canvas->width()+6,
                            obj->imageTest->canvas->height()+6,
                            TFT_BLACK);

        obj->imageTest->DrawTo(obj->buffer->canvas,x-(obj->imageTest->canvas->width()/2), y-(obj->imageTest->canvas->height()/2));
        //obj->buffer->canvas->fillCircle(x,y,5,TFT_BLACK);
    }
    if ( int(angle) == ((240+carrousel)%360) ) {

        obj->buffer->canvas->fillRect(
                            (x-(obj->imageTest->canvas->width()/2))-3,
                            (y-(obj->imageTest->canvas->height()/2))-3,
                            obj->imageTest2->canvas->width()+6,
                            obj->imageTest2->canvas->height()+6,
                            TFT_BLACK);

        obj->imageTest2->DrawTo(obj->buffer->canvas,x-(obj->imageTest->canvas->width()/2),y-(obj->imageTest->canvas->height()/2));
    }
    if ( int(angle) == ((carrousel)%360)) {

        obj->buffer->canvas->fillRect(
                            (x-(obj->imageTest->canvas->width()/2))-3,
                            (y-(obj->imageTest->canvas->height()/2))-3,
                            obj->imageTest3->canvas->width()+6,
                            obj->imageTest3->canvas->height()+6,
                            TFT_BLACK);

        obj->imageTest3->DrawTo(obj->buffer->canvas,x-(obj->imageTest->canvas->width()/2),y-(obj->imageTest->canvas->height()/2));
    }
    return true;
}
bool PlaygroundApplication::Tick() {
    if (millis() > nextRedraw ) {



        float currZ = imageDeepTest->z+0.05;
        if ( currZ > 1.5 ) { currZ = 0.05; buffer->canvas->fillSprite(TFT_BLACK); }
        imageDeepTest->DrawTo(buffer->canvas,120-(imageTest->canvas->width()/2),120-(imageTest->canvas->height()/2));
        //(imageDeepTest->canvas->width()),
        //                            (imageDeepTest->canvas->height()),currZ);


        unsigned long b4 = millis();
        for (int x=0;x<img_sample_120.width;x++){
            for (int y=0;y<img_sample_120.height;y++){
                    uint16_t originalPixel16 = imageTest->canvas->readPixel(x,y);
                    uint16_t originalPixel16_2 = imageTest3->canvas->readPixel(x,y);
                    uint16_t color16 = canvas->alphaBlend(currentAlpha,originalPixel16,originalPixel16_2);
                    imageTest2->canvas->drawPixel(x,y,color16);
            }
        }
        //Serial.printf("Time: %d Alpha: %d\n", (millis()-b4),currentAlpha);
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
        // dont want mask color
        //imageTest2->DrawTo(buffer->canvas,60,100,-1);


        buffer->DrawTo(canvas,0,0,-1);

        nextRedraw=millis()+(1000/18);
        return true;
    }
    return false;
}
