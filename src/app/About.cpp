#include <Arduino.h>
#include <LilyGoWatch.h>
//#include <libraries/TFT_eSPI/TFT_eSPI.h>

#include "Settings.hpp"
#include "../static/img_back_32.xbm"
#include "../lunokiot_config.hpp"
#include "About.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "LogView.hpp"

extern TTGOClass *ttgo; // ttgo lib
const unsigned long AboutBoxTextScrollDelay = 1000/24;
//const unsigned long 
size_t AboutBoxTextScrollOffset = 0;
/* @TODO show all of them!
Dependency Graph
|-- TTGO TWatch Library @ 1.4.2
|   |-- Wire @ 2.0.0
|   |-- SPI @ 2.0.0
|   |-- SPIFFS @ 2.0.0
|   |   |-- FS @ 2.0.0
|   |-- FS @ 2.0.0
|   |-- SD @ 2.0.0
|   |   |-- FS @ 2.0.0
|   |   |-- SPI @ 2.0.0
|   |-- Ticker @ 2.0.0
|-- ArduinoNvs @ 2.5.0
|-- QRCode @ 0.0.1
|-- Arduino_JSON @ 0.1.0
|-- NimBLE-Arduino @ 1.4.1
|-- ESP8266Audio @ 1.9.7
|   |-- SPIFFS @ 2.0.0
|   |   |-- FS @ 2.0.0
|   |-- FS @ 2.0.0
|   |-- HTTPClient @ 2.0.0
|   |   |-- WiFi @ 2.0.0
|   |   |-- WiFiClientSecure @ 2.0.0
|   |   |   |-- WiFi @ 2.0.0
|   |-- SD @ 2.0.0
|   |   |-- FS @ 2.0.0
|   |   |-- SPI @ 2.0.0
|   |-- SPI @ 2.0.0
|-- WiFi @ 2.0.0
|-- WiFiProv @ 2.0.0
|   |-- WiFi @ 2.0.0
|   |-- SimpleBLE @ 2.0.0
|-- Ticker @ 2.0.0
|-- HTTPClient @ 2.0.0
|   |-- WiFi @ 2.0.0
|   |-- WiFiClientSecure @ 2.0.0
|   |   |-- WiFi @ 2.0.0
|-- ESP32 BLE Arduino @ 2.0.0
*/
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

    "",
    "lunokWatch is",
    "open-source",
    "software",
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
    "lewisxhe",
    "PCF8563_Library",
    "AXP202X_Libraries",
    "",
    "Bosch",
    "bma423",
    "",
    "Tal H",
    "ccperceptron",
    "",
    "",
    "",
    "Game tileset",
    "--------",
    "dungeontileset-ii", // https://0x72.itch.io/dungeontileset-ii
    "from:",
    "Robert",
    "",
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
    "",
    "",
    "Thanks all!",
    "",
    "",
    "",
    "",
    "",
    "Join telegram",
    "channel!!!",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};
AboutApplication::~AboutApplication() {
    directDraw=false;
    if ( nullptr != colorBuffer ) { delete colorBuffer; }
    if ( nullptr != textBuffer ) {
        textBuffer->deleteSprite();
        delete textBuffer;
    }
    if ( nullptr != perspectiveTextBuffer ) { delete perspectiveTextBuffer; }
}

AboutApplication::AboutApplication() {
    directDraw=true;
    canvas->fillSprite(TFT_BLACK);
    AboutBoxTextScrollOffset=0;

    perspectiveTextBuffer = new CanvasWidget(TFT_HEIGHT-100,TFT_WIDTH);
    perspectiveTextBuffer->canvas->fillSprite(TFT_BLACK);

    colorBuffer = new CanvasWidget(perspectiveTextBuffer->canvas->height(),perspectiveTextBuffer->canvas->width());

    // here goes text (color/nocolor)
    textBuffer = new TFT_eSprite(ttgo->tft);
    textBuffer->setColorDepth(1);
    textBuffer->createSprite((colorBuffer->canvas->width()/2),(colorBuffer->canvas->height()/2)+9); // font height
    textBuffer->fillSprite(TFT_BLACK);
    textBuffer->setTextFont(0);
    textBuffer->setTextColor(TFT_WHITE);
    textBuffer->setTextSize(1);
    textBuffer->setTextDatum(BC_DATUM);
    // change the button color before draw
    TemplateApplication::btnBack->xbmColor=TFT_WHITE;
    TemplateApplication::btnBack->InternalRedraw();
    TemplateApplication::Tick(); // draw back button for splash

}

bool AboutApplication::Tick() {
    UINextTimeout = millis()+UITimeout; // disable screen timeout
    if ( millis() > nextRedraw) {
        // update the source canvas (plain text scroll)
        textBuffer->scroll(0,-2);

        // redraw the final canvas
        perspectiveTextBuffer->canvas->fillSprite(TFT_BLACK);
        for(int32_t y=0;y<perspectiveTextBuffer->canvas->height();y++) {
            uint8_t bytepc = ((y*255)/perspectiveTextBuffer->canvas->height());
            //float divisor = 1.0+(bytepc/255.0);

            uint16_t dcolor = canvas->alphaBlend(bytepc,TFT_YELLOW,TFT_CYAN); // darken with distance
            for(int32_t x=0;x<perspectiveTextBuffer->canvas->width();x++) {
                int32_t rx = x; //((TFT_WIDTH/2)*-1)+(x*divisor);
                if (( rx < 0 ) || ( rx/2 >= textBuffer->width() )) { continue; } // origin out of bounds
                uint16_t ocolor = textBuffer->readPixel(rx/2,y/2);
                int32_t dx = x; //(TFT_WIDTH/2)-(x*divisor);
                if (( dx < 0 ) || ( dx >= perspectiveTextBuffer->canvas->width() )) { continue; } // destination out of bounds
                if ( TFT_BLACK != ocolor ) { perspectiveTextBuffer->canvas->drawPixel(dx,y,dcolor); }
            }
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
            textBuffer->drawString(AboutBoxTextScroll[AboutBoxTextScrollOffset],textBuffer->width()/2, textBuffer->height());

            AboutBoxTextScrollOffset++;
            int AboutBoxTextSize = sizeof(AboutBoxTextScroll)/sizeof(AboutBoxTextScroll[0])-1;
            if ( AboutBoxTextScrollOffset > AboutBoxTextSize ) {
                AboutBoxTextScrollOffset=0;
            }
        }
        TemplateApplication::Tick();
        perspectiveTextBuffer->canvas->pushSprite(0,50);

        nextRedraw=millis()+AboutBoxTextScrollDelay;
    }
    return false; // directdraw pointless response
}