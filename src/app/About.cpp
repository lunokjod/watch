//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//
// LunokWatch is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later 
// version.
//
// LunokWatch is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
// details.
//
// You should have received a copy of the GNU General Public License along with 
// LunokWatch. If not, see <https://www.gnu.org/licenses/>. 
//

#include <Arduino.h>
#include <LilyGoWatch.h>
//#include <libraries/TFT_eSPI/TFT_eSPI.h>


#include "Settings.hpp"
#include "../lunokiot_config.hpp"
#include "About.hpp"
#include "LogView.hpp"

extern TTGOClass *ttgo; // ttgo lib
const unsigned long AboutBoxTextScrollDelay = 1000/12;
const unsigned long AboutBoxTextScrollDelayScroll = 1000/24;
//const unsigned long 
size_t AboutBoxTextScrollOffset = 0;
/* @TODO show all of them!
|-- Sqlite3Esp32 @ 2.3.0+sha.9f35709
|-- TTGO TWatch Library_alt @ 0.0.1+sha.f9d72c5
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
|-- miniz @ 0.0.1+sha.bbaaaf6
|-- ESP8266Audio @ 1.9.7
|   |-- SPIFFS @ 2.0.0
|   |   |-- FS @ 2.0.0
|   |-- I2S @ 1.0
|   |-- FS @ 2.0.0
|   |-- HTTPClient @ 2.0.0
|   |   |-- WiFi @ 2.0.0
|   |   |-- WiFiClientSecure @ 2.0.0
|   |   |   |-- WiFi @ 2.0.0
|   |-- SD @ 2.0.0
|   |   |-- FS @ 2.0.0
|   |   |-- SPI @ 2.0.0
|   |-- SPI @ 2.0.0
|-- Ticker @ 2.0.0
|-- FS @ 2.0.0
|-- HTTPClient @ 2.0.0
|   |-- WiFi @ 2.0.0
|   |-- WiFiClientSecure @ 2.0.0
|   |   |-- WiFi @ 2.0.0
|-- LittleFS @ 2.0.0
|   |-- FS @ 2.0.0
|-- SPI @ 2.0.0
|-- WiFi @ 2.0.0
|-- WiFiProv @ 2.0.0
|   |-- WiFi @ 2.0.0
|   |-- SimpleBLE @ 2.0.0
|-- ESP32 BLE Arduino @ 2.0.0
*/
const char *AboutBoxTextEmptyLine="";

const char *AboutBoxTextLogo=":lIoT:";
#include "../resources.hpp"

const char *AboutBoxTextScroll[] = {
    "A short time ago",
    "in a galaxy far,",
    "far closer....",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextLogo,
    AboutBoxTextEmptyLine,
    "lunokIoT Watch",
    "version 0.1a",
    "build #",
    LUNOKIOT_BUILD_STRING,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    "lunokWatch is",
    "open-source",
    "software",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    "Libraries",
    "---------",
    "xinyuan",
    "TTGO TWatch",
    "lilygo",
    "Library",
    AboutBoxTextEmptyLine,
    "rpolitex",
    "ArduinoNvs",
    AboutBoxTextEmptyLine,
    "ricmoo",
    "QRCode",
    AboutBoxTextEmptyLine,
    "lbernstone",
    "miniz-esp32",
    AboutBoxTextEmptyLine,
    "sfranzyshen",
    "ESP-Arduino-Lua",
    AboutBoxTextEmptyLine,
    "arduino",
    "libraries",
    "--------",
    "Arduino_JSON",
    AboutBoxTextEmptyLine,
    "earlephilhower",
    "ESP8266Audio",
    AboutBoxTextEmptyLine,
    "h2zero",
    "NimBLE",
    AboutBoxTextEmptyLine,
    "crankyoldgit",
    "IRremote-",
    " ESP8266",
    AboutBoxTextEmptyLine,
    "lewisxhe",
    "PCF8563 lib",
    "AXP202X libs",
    AboutBoxTextEmptyLine,
    "Bosch",
    "bma423 lib",
    AboutBoxTextEmptyLine,
    "Tal H",
    "ccperceptron",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    "Game tileset",
    "--------",
    "dungeon-",
    "tileset-ii", // https://0x72.itch.io/dungeontileset-ii
    "from:",
    "Robert",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    "Gadgetbridge",
    "--------",
    "esprunio",
    "& banglejs",
    "from:",
    "Gordon",
    "Williams",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    "Services",
    "--------",
    "OpenWeather",
    "geoplugin.net",
    "github",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    "Sound",
    "effects",
    "--------",
    "MujiPiruji",
    ":-***",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    "Personal",
    "Thanks",
    "--------",
    "@hpsaturn",
    "colaborator",
    AboutBoxTextEmptyLine,
    "@MujiPiruji",
    "colaborator",
    AboutBoxTextEmptyLine,
    "@nikonnazi",
    "colaborator",
    "carlosformula",
    "at gmail",
    AboutBoxTextEmptyLine,
    "@Roberbike",
    "suffered beta",
    "tester ;)",
    AboutBoxTextEmptyLine,
    "@shinohai",
    "great slogan!",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    "Thanks all!",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    "Join telegram",
    "channel",
    "and get:",
    "--------",
    AboutBoxTextEmptyLine,
    "early",
    "updates",
    "and howtos",
    AboutBoxTextEmptyLine,
    "poll new",
    "features",
    AboutBoxTextEmptyLine,
    "tips and tricks",
    AboutBoxTextEmptyLine,
    "devel support",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    "buy me a coffe",
    "if you like",
    "this software",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    "GPL-v3",
    "--------",
    "get source",
    "and details",
    "on our site",
    "in github",
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
    AboutBoxTextEmptyLine,
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
    textBuffer = new TFT_eSprite(tft);
    textBuffer->setColorDepth(1);
    textBuffer->createSprite((colorBuffer->canvas->width()*TEXTZOOM),(colorBuffer->canvas->height()*TEXTZOOM)+32); // buffer to print/draw
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
    
    TemplateApplication::btnBack->Interact(touched,touchX,touchY);
    if ( millis() > nextStep) {
        // update the source canvas (plain text scroll)
        textBuffer->scroll(0,-2);

        // redraw the final canvas
        int32_t x=0;
        int32_t y=0;
        perspectiveTextBuffer->canvas->fillSprite(TFT_BLACK);
        for(x=0;x<perspectiveTextBuffer->canvas->width();x++) {
            //float divisor = 1.0+(bytepc/255.0);
            uint8_t bytepc2 = ((x*255)/perspectiveTextBuffer->canvas->width());
            uint16_t bcolor = canvas->alphaBlend(bytepc2,TFT_BLUE,TFT_ORANGE); // darken with distance
            //uint16_t dcolor = canvas->alphaBlend(bytepc,TFT_YELLOW,TFT_CYAN); // darken with distance
            for(y=0;y<perspectiveTextBuffer->canvas->height();y++) {
                uint8_t bytepc = ((y*255)/perspectiveTextBuffer->canvas->height());
                //uint8_t bytepc3 = (((x+(y*perspectiveTextBuffer->canvas->width()))*255)/(perspectiveTextBuffer->canvas->height()+(y*perspectiveTextBuffer->canvas->width())));
                //uint16_t acolor = canvas->alphaBlend(bytepc,TFT_RED,TFT_CYAN); // darken with distance
                uint16_t acolor = canvas->alphaBlend(bytepc,TFT_PURPLE,TFT_YELLOW); // darken with distance
                uint16_t dcolor = canvas->alphaBlend(128,acolor,bcolor); // darken with distance

                int32_t rx = x; //((TFT_WIDTH/2)*-1)+(x*divisor);
                if (( rx < 0 ) || ( rx*TEXTZOOM >= textBuffer->width() )) { continue; } // origin out of bounds
                uint16_t ocolor = textBuffer->readPixel(rx*TEXTZOOM,y*TEXTZOOM);
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
        
        if ( 0 == (counterLines%4)) { // every 18 lines draw new text line (font height)
            // is the logo???
            if ( 0 == strcmp(AboutBoxTextScroll[AboutBoxTextScrollOffset],AboutBoxTextLogo) ) {
                //lAppLog("LOGOOOOO\n");
                textBuffer->drawXBitmap((textBuffer->width()-lunokiot_logo_32_width)/2,textBuffer->height()-lunokiot_logo_32_height,lunokiot_logo_32_bits, lunokiot_logo_32_width,lunokiot_logo_32_height, TFT_WHITE);
            } else {
                // centered on buffer
                textBuffer->drawString(AboutBoxTextScroll[AboutBoxTextScrollOffset],textBuffer->width()/2, textBuffer->height());
                /*
                TEXTZOOM+=(upOrDown?0.01:-0.01);
                if (( TEXTZOOM > MAXTIMEZOOM)||( TEXTZOOM < MINTIMEZOOM)) { upOrDown=(!upOrDown); }
                */
            }

            AboutBoxTextScrollOffset++;
            int AboutBoxTextSize = sizeof(AboutBoxTextScroll)/sizeof(AboutBoxTextScroll[0])-1;
            if ( AboutBoxTextScrollOffset > AboutBoxTextSize ) {
                AboutBoxTextScrollOffset=0;
            }
        }
        nextStep=millis()+AboutBoxTextScrollDelayScroll;
    }

    if ( millis() > nextRedraw) {

        TemplateApplication::btnBack->DirectDraw();
        // Direct draw xD

        //TransformationMatrix imgTransf = { 0.46, 0.18, 0.06, 0.95 };
        //TFT_eSprite * transformedCopy = ShearSprite(perspectiveTextBuffer->canvas,imgTransf);
        perspectiveTextBuffer->canvas->pushSprite(
            (TFT_WIDTH-perspectiveTextBuffer->canvas->width())/2
            ,(TFT_HEIGHT-perspectiveTextBuffer->canvas->height())/2);
        //transformedCopy->deleteSprite();
        //delete transformedCopy;
        nextRedraw=millis()+AboutBoxTextScrollDelay;
    }
    return false; // directdraw pointless response
}