#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Settings.hpp"
#include "../static/img_back_32.xbm"
#include "../lunokiot_config.hpp"
#include "About.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

const unsigned long AboutBoxTextScrollDelay = 1000/16;
//const unsigned long 
size_t AboutBoxTextScrollOffset = 0;
const char *AboutBoxTextScroll[] = {
    "A short time ago",
    "in a galaxy far,",
    "far closer....",
    "",
    "",
    "",
    "",
    "",
    "lunokIoT Watch",
    "version 0.a",
    LUNOKIOT_BUILD_STRING,
    "",
    "",
    "blah blah blah",
    "yak yak yak",
    "@TODO",
    "thirdparty",
    "credits",
    "",

    "",
    "",
    "",
    "",
    "Libraries",
    "---------",
    "xinyuan-lilygo",
    "TTGO TWatch Library",
    "",
    "rpolitex",
    "ArduinoNvs",
    "",
    "ricmoo",
    "QRCode",
    "",
    "arduino-libraries",
    "Arduino_JSON",
    "",
    "earlephilhower",
    "ESP8266Audio",
    "",
    "h2zero",
    "NimBLE-Arduino",
    "",
    "crankyoldgit",
    "IRremoteESP8266",
    "",
    "",
    "Services",
    "--------",
    "OpenWeather",
    "geoplugin.net",
    "",
    "",
    "",
    "",
    "Thanks all!",
    "",
    "",
    "",
    "",
    "",
    "Join telegram",
    "channel!",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};
AboutApplication::~AboutApplication() {
    if ( nullptr != btnBack ) { delete btnBack; }
    if ( nullptr != colorBuffer ) { delete colorBuffer; }
    if ( nullptr != textBuffer ) {
        textBuffer->deleteSprite();
        delete textBuffer;
    }
    if ( nullptr != perspectiveTextBuffer ) { delete perspectiveTextBuffer; }
    directDraw=false;
}

AboutApplication::AboutApplication() {
    directDraw=true;
    canvas->fillSprite(TFT_BLACK);
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,ttgo->tft->color24to16(0x353e45),false);
    textBuffer = new TFT_eSprite(ttgo->tft);
    textBuffer->setColorDepth(1);
    textBuffer->createSprite(TFT_WIDTH,120+18);
    textBuffer->fillSprite(TFT_BLACK);
    AboutBoxTextScrollOffset=0;
//    textBuffer->setScrollRect(0,0,TFT_WIDTH,160-18,TFT_BLACK);

    colorBuffer = new CanvasWidget(120,TFT_WIDTH);
    perspectiveTextBuffer = new CanvasWidget(120,TFT_WIDTH);
    perspectiveTextBuffer->canvas->fillSprite(TFT_BLACK);

    // final interface
    canvas->fillRect(0,0,TFT_WIDTH,30,TFT_BLACK);
    canvas->fillRect(0,150,TFT_WIDTH,15,TFT_BLACK);
    canvas->fillRect(0,TFT_HEIGHT-74,TFT_WIDTH,74,canvas->color24to16(0x212121));
    btnBack->DrawTo(canvas);
    canvas->pushSprite(0,0);

}

bool AboutApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY);
    if ( millis() > nextScrollRedraw) {
        // update the source canvas (plain text scroll)
        textBuffer->scroll(0,-3);

        // redraw the final canvas
        perspectiveTextBuffer->canvas->fillSprite(TFT_BLACK);
        for(int32_t y=0;y<perspectiveTextBuffer->canvas->height();y++) {
            //uint16_t percent = (y*100)/perspectiveTextBuffer->canvas->height();
            uint8_t bytepc = ((y*255)/perspectiveTextBuffer->canvas->height());
            float divisor = 1.0+(bytepc/255.0);
            //Serial.printf("Divisor: %f byte: %d\n", divisor,bytepc);

            uint16_t dcolor = canvas->alphaBlend(bytepc,TFT_YELLOW,TFT_BLACK); // darken with distance
            for(int32_t x=0;x<perspectiveTextBuffer->canvas->width();x++) {
                int32_t rx = x; //((TFT_WIDTH/2)*-1)+(x*divisor);
                if (( rx < 0 ) || ( rx >= perspectiveTextBuffer->canvas->width() )) { continue; } // origin out of bounds
                uint16_t ocolor = textBuffer->readPixel(rx,y);
                int32_t dx = x; //(TFT_WIDTH/2)-(x*divisor);
                if (( dx < 0 ) || ( dx >= perspectiveTextBuffer->canvas->width() )) { continue; } // destination out of bounds
                if ( TFT_BLACK != ocolor ) { perspectiveTextBuffer->canvas->drawPixel(dx,y,dcolor); }
                //else { perspectiveTextBuffer->canvas->drawPixel(dx,y,TFT_BLUE); }
            }
            /*
            uint16_t lcolor = canvas->alphaBlend(bytepc,TFT_YELLOW,TFT_BLACK); // darken in distance
            uint16_t rcolor = canvas->alphaBlend(bytepc,TFT_WHITE,TFT_BLACK); // darken in distance
            int32_t begin = perspectiveTextBuffer->canvas->width()/2;
            for(int32_t x=begin;x>0;x--) {
                int32_t nx = x-(x*divisor);
                uint16_t ocolor = textBuffer->readPixel(nx,y);
                if ( TFT_BLACK == ocolor ) { continue; }
                perspectiveTextBuffer->canvas->drawPixel(x,y,lcolor);
            }
            for(int32_t x=begin;x<TFT_WIDTH;x++) {
                int32_t nx = x-(x*divisor);
                uint16_t ocolor = textBuffer->readPixel(nx,y);
                if ( TFT_BLACK == ocolor ) { continue; }
                perspectiveTextBuffer->canvas->drawPixel(x,y,rcolor);
            }
            */
        }
        /*
        // usefull for debug perspective
        perspectiveTextBuffer->canvas->drawLine(0,perspectiveTextBuffer->canvas->height(),perspectiveTextBuffer->canvas->width()/2,0,TFT_RED);
        perspectiveTextBuffer->canvas->drawLine(perspectiveTextBuffer->canvas->width(),perspectiveTextBuffer->canvas->height(),perspectiveTextBuffer->canvas->width()/2,0,TFT_RED);
        perspectiveTextBuffer->canvas->drawFastVLine(perspectiveTextBuffer->canvas->width()/2,0,perspectiveTextBuffer->canvas->height(),TFT_GREEN);
        */

        perspectiveTextBuffer->DrawTo(colorBuffer->canvas,0,0);
        counterLines++;
        if ( 0 == (counterLines%(18/3))) { // every 18 lines draw new text line (font height)
            textBuffer->setTextFont(0);
            textBuffer->setTextColor(TFT_WHITE);
            textBuffer->setTextSize(2);
            textBuffer->setTextDatum(BC_DATUM);
            textBuffer->drawString(AboutBoxTextScroll[AboutBoxTextScrollOffset],textBuffer->width()/2, textBuffer->height());

            AboutBoxTextScrollOffset++;
            int AboutBoxTextSize = sizeof(AboutBoxTextScroll)/sizeof(AboutBoxTextScroll[0])-1;
            if ( AboutBoxTextScrollOffset > AboutBoxTextSize ) {
                AboutBoxTextScrollOffset=0;
            }
        }
        perspectiveTextBuffer->canvas->pushSprite(0,30);

        nextScrollRedraw=millis()+AboutBoxTextScrollDelay;
    }

    if (millis() > nextRedraw ) {
        colorBuffer->DrawTo(canvas,0,30);
        btnBack->DrawTo(canvas);
        nextRedraw=millis()+(1000/10);
        return true;
    }
    return false;
}