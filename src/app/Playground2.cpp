#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Playground2.hpp"
#include "../static/img_game_spaceship1.c"
#include "lunokiot_config.hpp"
PlaygroundApplication2::~PlaygroundApplication2() {
    
}
PlaygroundApplication2::PlaygroundApplication2() {
    buffer = new CanvasWidget(TFT_HEIGHT, TFT_WIDTH);
    buffer->canvas->fillSprite(TFT_BLACK);
    spaceShip = new CanvasWidget(img_game_spaceship1.height,img_game_spaceship1.width);
    spaceShip->canvas->setSwapBytes(true);
    spaceShip->canvas->pushImage(0,0,img_game_spaceship1.width,img_game_spaceship1.height,(const uint16_t *)img_game_spaceship1.pixel_data);
    spaceShip->canvas->setSwapBytes(false);
    bullets = new CanvasWidget(TFT_HEIGHT,TFT_WIDTH);
}
bool PlaygroundApplication2::Tick() {
    int16_t shipX = (TFT_WIDTH/2)-(spaceShip->canvas->width()/2);
    int16_t shipY = (TFT_HEIGHT - spaceShip->canvas->height())-10;
    shipX+=180-degY;

    if (millis() > gameTick ) {
        // mountains
        int32_t horizontal=0;
        int8_t lastHeight = 0;
        while ( horizontal < 255 ){
            int8_t height = random(0,15);
            //buffer->canvas->drawLine(horizontal-15,15-lastHeight,horizontal,15-height,canvas->color24to16(0x353e45));
            buffer->canvas->drawLine(horizontal-15,15-lastHeight,horizontal,15-height,canvas->color24to16(0x0f0f0f));
            horizontal+=15;
            lastHeight=height;
        }
        //buffer->canvas->scroll(0,15);
        
        // stars
        for(int c=0;c<5;c++) {
            int16_t sx = random(0,TFT_WIDTH);
            int16_t sy = random(0,10);
            //buffer->canvas->drawPixel(sx,sy,TFT_WHITE);
            buffer->canvas->drawFastVLine(sx,sy,sy+5,TFT_BLUE);
        }
        buffer->canvas->scroll(0,15);



        if (millis() > shootTime ) {
            if (millis() > nextShootTime ) {
                bullets->canvas->fillCircle(shipX+(spaceShip->canvas->width()/2),165,3,TFT_WHITE);
                nextShootTime=millis()+1200;
            }
            bullets->canvas->scroll(0,-10);
            bullets->canvas->fillRect(0,210,240,30,CanvasWidget::MASK_COLOR);
            shootTime=millis()+(1000/8);
        }

        gameTick=millis()+(1000/12);
    }
    if (millis() > nextRedraw ) {
        buffer->DrawTo(canvas);

        // fade magic
        for(int x=0;x<spaceShip->canvas->width();x++ ) {
            for(int y=0;y<spaceShip->canvas->height();y++ ) {
                //uint16_t originalColor16 = canvas->readPixel(x+shipX,y+shipY);
                uint16_t myColor16 = spaceShip->canvas->readPixel(x,y); // obtain from canvas
                //canvas->drawPixel(x+shipX,y+shipY,canvas->alphaBlend(alpha,myColor16,originalColor16));
                // RGB
                // GBR
                //if ( myColor16 != ttgo->tft->color565(0x01,0x20,0x00) ) {
                if ( myColor16 != CanvasWidget::MASK_COLOR ) {
                    //Serial.printf("COLOR: %u\n", myColor16);
                    canvas->drawPixel(x+shipX,y+shipY,myColor16);
                }
                /*
                if ( ( x == 0 ) && ( y == 0 ) ) {
                    Serial.printf("COLOR: %u\n", myColor16);
                }*/
            }
        }
        bullets->DrawTo(canvas);
        //spaceShip->DrawTo(canvas,shipX,shipY,ttgo->tft->color565(0x01,0x00,0x20));
        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}