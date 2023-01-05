#include "Rotation.hpp"

#include "../UI/UI.hpp"
#include "../static/img_happy_48.xbm" // center smiley to see the orientation
#include <ArduinoNvs.h> // persistent values

#include <libraries/TFT_eSPI/TFT_eSPI.h>
extern TFT_eSPI *tft;

// better save only on exit to save flash writes
RotationApplication::~RotationApplication() {
    NVS.setInt("ScreenRot",tft->getRotation(),false);
    delete up;
    delete right;
    delete down;
    delete left;
}

RotationApplication::RotationApplication() {
    down = new ButtonWidget(60,(TFT_HEIGHT-10)-60,60,TFT_WIDTH-120,[](void *unused) { tft->setRotation(0); });
    left = new ButtonWidget(10,60,TFT_HEIGHT-120,60,[](void *unused) { tft->setRotation(1); });
    up = new ButtonWidget(60,10,60,TFT_WIDTH-120,[](void *unused) { tft->setRotation(2); });
    right = new ButtonWidget((TFT_WIDTH-10)-60,60,TFT_HEIGHT-120,60,[](void *unused) { tft->setRotation(3); });
    Tick();
}

bool RotationApplication::Tick() {
    up->Interact(touched,touchX,touchY);
    right->Interact(touched,touchX,touchY);
    down->Interact(touched,touchX,touchY);
    left->Interact(touched,touchX,touchY);

    if (millis() > nextRedraw ) {
        // draw code here
        canvas->fillSprite(ThCol(background));

        canvas->drawXBitmap((TFT_WIDTH/2)-(img_happy_48_width/2),
                (TFT_HEIGHT/2)-(img_happy_48_height/2),
                img_happy_48_bits,img_happy_48_width,img_happy_48_height,TFT_WHITE);

        uint8_t rot = tft->getRotation();
        up->DrawTo(canvas);
        right->DrawTo(canvas);
        down->DrawTo(canvas);
        left->DrawTo(canvas);
        
        nextRedraw=millis()+(1000/3); // tune your required refresh
        return true;
    }
    return false;
}
