
#include "Playground4.hpp"
#include "../UI/UI.hpp"
#include <list>

extern std::list<TFT_eSprite *>ScreenShots;

PlaygroundApplication4::~PlaygroundApplication4() {
    directDraw=false;
}

PlaygroundApplication4::PlaygroundApplication4() {
    directDraw=true;
    testBackground = new CanvasWidget(100,100);
    /*
    for(int y=0;y<100;y++){
        for(int x=0;x<100;x++){
            uint8_t tone = random(80,255);
            uint16_t color = canvas->color565(tone,tone,tone);
            testBackground->canvas->drawPixel(x,y,color);
        }
    }*/
    test = new ScrollViewWidget(40,40, 240-80, 240-80, canvas->color24to16(0x212121),testBackground);

    for (auto const& capture : ScreenShots) {
        TFT_eSprite * reduced = ScaleSprite(capture,0.5);
        Thumbnails.push_back(reduced);
    }
}
bool PlaygroundApplication4::Tick() {

    canvas->fillSprite(canvas->color24to16(0x212121));
    int16_t x=test->offsetX;
    int16_t y=test->offsetY;
    for (TFT_eSprite * thumb : Thumbnails) {
        thumb->setPivot(x,y);
        thumb->pushRotated(canvas,0);
        x+=thumb->width()+10;
    }
    test->Interact(touched,touchX,touchY);
    test->DrawTo(this->canvas);

/*
    if ( directDraw ) { 
        canvas->pushSprite(x,y);
      
                          if ( touched ) {
                        ttgo->tft->fillScreen(TFT_GREEN);
                        ttgo->tft->setWindow(x,y,x+w,y+h);
                        directDraw=true;
                    }
                    else {
                        ttgo->tft->setWindow(0,0,TFT_WIDTH,TFT_HEIGHT);
                        directDraw=false;
                    }

      
        return;
    }
*/

/*
    if ( directDraw ) {
        //ttgo->tft->setWindow(test->x,test->y,test->h,test->w);
        canvas->fillSprite(canvas->color24to16(0x212121));
        //test->DrawTo(this->canvas);
        //canvas->pushSprite(test->x,test->y);
        test->canvas->pushSprite(test->x,test->y);
        return false;
    }*/



    //if ( 1 == (millis() % 3000)) {
    //    testBackground->canvas->fillSprite(random(0,(2^16)-1));
    //}



   // if (millis() > nextRedraw ) {
/*
        reduced->setPivot(thx,thy);
    int32_t thx = 5;
    int32_t thy = 5;
        reduced->pushRotated(testBackground->canvas,0);
        thx+=(reduced->width()+5);
*/
        test->DrawTo(this->canvas);
        nextRedraw=millis()+(1000/16);
        return true;
    //}
    //return false;
}