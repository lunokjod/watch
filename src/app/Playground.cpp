#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Playground.hpp"
#include "../static/img_sample_120.c"
#include "../static/img_sample2_120.c"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/UI.hpp"


PlaygroundApplication::~PlaygroundApplication() {
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
    buffer->canvas->fillSprite(0x0);

    imageTest = new CanvasWidget(img_sample_120.height, img_sample_120.width);
    imageTest->canvas->setSwapBytes(true);
    imageTest->canvas->pushImage(0,0,img_sample_120.width,img_sample_120.height,(uint16_t *)img_sample_120.pixel_data);
    imageTest->canvas->setSwapBytes(false);
    
    imageTest2 = new CanvasWidget(img_sample_120.height, img_sample_120.width);

    imageTest3 = new CanvasWidget(img_sample2_120.height, img_sample2_120.width);
    imageTest3->canvas->setSwapBytes(true);
    imageTest3->canvas->pushImage(0,0,img_sample2_120.width,img_sample2_120.height,(uint16_t *)img_sample2_120.pixel_data);
    imageTest3->canvas->setSwapBytes(false);


}// typedef std::function<bool (int,int, int, int, double, int, void*)> DescribeCircleCallback;
int16_t carrousel = 0;
bool GyroTest(int x,int y, int cx, int cy, int angle, int step, void* payload) {
    PlaygroundApplication * obj = (PlaygroundApplication * )payload;
    
    
    if ( int(angle) == ((120+carrousel)%360) ) {
        obj->imageTest->DrawTo(obj->buffer->canvas,x-(obj->imageTest->canvas->width()/2),y-(obj->imageTest->canvas->height()/2));
        //obj->buffer->canvas->fillCircle(x,y,5,TFT_BLACK);
    }
    if ( int(angle) == ((240+carrousel)%360) ) {
        obj->imageTest2->DrawTo(obj->buffer->canvas,x-(obj->imageTest->canvas->width()/2),y-(obj->imageTest->canvas->height()/2));
    }
    if ( int(angle) == ((carrousel)%360)) {
        obj->imageTest3->DrawTo(obj->buffer->canvas,x-(obj->imageTest->canvas->width()/2),y-(obj->imageTest->canvas->height()/2));
    }
    return true;
}
bool PlaygroundApplication::Tick() {
    if (millis() > nextRedraw ) {
        //imageTest->canvas->fillSprite(canvas->color24to16(0xff0000));
        //imageTest3->canvas->fillSprite(canvas->color24to16(0x00ff00));
        buffer->canvas->fillSprite(0x0);
        //imageTest3->DrawTo(buffer->canvas,120,0);

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
        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}
