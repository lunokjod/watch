#include <Arduino.h>
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo lib
#include "LogView.hpp" // log capabilities

#include "Lamp.hpp"

LampApplication::LampApplication() {
    canvas->fillSprite(TFT_WHITE);
    //ttgo->setBrightness(255);
    Tick(); // OR call this if no splash 
}

//LampApplication::~LampApplication() {
    // please remove/delete/free all to avoid leaks!
//}

bool LampApplication::Tick() {
    UINextTimeout = millis()+UITimeout;
    if (  255 > ttgo->bl->getLevel() ) {
        ttgo->setBrightness(ttgo->bl->getLevel()+1);
    }
    return false;
}
