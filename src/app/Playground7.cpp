#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Playground7.hpp"
#include "../UI/UI.hpp"
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo library shit ;)

PlaygroundApplication7::~PlaygroundApplication7() {
    ttgo->setBrightness(128);    
}
PlaygroundApplication7::PlaygroundApplication7() {
    ttgo->setBrightness(5);
}
bool PlaygroundApplication7::Tick() {
    if (millis() > nextRedraw ) {
        // draw code here
        time_t now;
        struct tm * tmpTime;
        struct tm timeinfo;
        time(&now);
        tmpTime = localtime(&now);
        memcpy(&timeinfo,tmpTime, sizeof(struct tm));
        char buffer[64] = { 0 };
        sprintf(buffer,"%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        canvas->setTextSize(2);
        canvas->setTextDatum(CC_DATUM);
        canvas->setFreeFont(&FreeMonoBold18pt7b);
        canvas->setTextColor(TFT_WHITE);
        canvas->fillSprite(TFT_BLACK);
        canvas->drawString(buffer, TFT_WIDTH/2, TFT_HEIGHT/2);
        nextRedraw=millis()+(1000/1); // tune your required refresh
        return true;
    }
    return false;
}