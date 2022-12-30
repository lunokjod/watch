#include <Arduino.h>
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo lib
#include "LogView.hpp" // log capabilities

#include "Lamp.hpp"

LampApplication::LampApplication() {
    // Init here any you need
    //lAppLog("Hello from Lamp!\n");

    canvas->fillSprite(TFT_WHITE);
    ttgo->setBrightness(255);

    // Remember to dump anything to canvas to get awesome splash screen
    // draw of TFT_eSPI canvas for splash here

    Tick(); // OR call this if no splash 
}

LampApplication::~LampApplication() {
    // please remove/delete/free all to avoid leaks!
}

bool LampApplication::Tick() {
    if ( 255 != ttgo->bl->getLevel() ) {
        ttgo->setBrightness(ttgo->bl->getLevel()+1);
    }
    return false;
}
