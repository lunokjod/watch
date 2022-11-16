#include <Arduino.h>
//#include <LilyGoWatch.h>
#include "Rotation.hpp"
#include "../UI/UI.hpp"
#include "../static/img_happy_48.xbm"
#include <ArduinoNvs.h> // persistent values

#include <libraries/TFT_eSPI/TFT_eSPI.h>
extern TFT_eSPI *tft;

RotationApplication::~RotationApplication() {
    if ( nullptr != buffer ) { delete buffer; }
    if ( nullptr != up ) { delete up; }
    if ( nullptr != right ) { delete right; }
    if ( nullptr != down ) { delete down; }
    if ( nullptr != left ) { delete left; }
}
RotationApplication::RotationApplication() {
    buffer = new CanvasWidget(TFT_HEIGHT,TFT_WIDTH);

    up = new ButtonWidget(60,10,60,TFT_WIDTH-120,[]() {
        NVS.setInt("ScreenRot",2,false);
        tft->setRotation(2);
    });
    right = new ButtonWidget((TFT_WIDTH-10)-60,60,TFT_HEIGHT-120,60,[]() {
        NVS.setInt("ScreenRot",3,false);
        tft->setRotation(3);
     });
    down = new ButtonWidget(60,(TFT_HEIGHT-10)-60,60,TFT_WIDTH-120,[]() {
        NVS.setInt("ScreenRot",0,false);
        tft->setRotation(0);
    });
    left = new ButtonWidget(10,60,TFT_HEIGHT-120,60,[]() {
        NVS.setInt("ScreenRot",1,false);
        tft->setRotation(1);
    });
    Tick();
}
bool RotationApplication::Tick() {
    up->Interact(touched,touchX,touchY);
    right->Interact(touched,touchX,touchY);
    down->Interact(touched,touchX,touchY);
    left->Interact(touched,touchX,touchY);
    if (millis() > nextRedraw ) {
        // draw code here
        canvas->fillSprite(canvas->color24to16(0x212121));

        canvas->drawXBitmap((TFT_WIDTH/2)-(img_happy_48_width/2),
                (TFT_HEIGHT/2)-(img_happy_48_height/2),
                img_happy_48_bits,img_happy_48_width,img_happy_48_height,TFT_WHITE);

        up->DrawTo(buffer->canvas);
        right->DrawTo(buffer->canvas);
        down->DrawTo(buffer->canvas);
        left->DrawTo(buffer->canvas);
        buffer->DrawTo(canvas);
        
        nextRedraw=millis()+(1000/8); // tune your required refresh
        return true;
    }
    return false;
}