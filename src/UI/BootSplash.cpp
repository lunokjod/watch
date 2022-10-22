#include "BootSplash.hpp"
#include "lunokiot_config.hpp"

#include "../static/img_lunokiot_logo.xbm"

void SplashAnnounce() {
    ttgo->tft->fillScreen(TFT_WHITE);
    // coords from gimp :) manual stetic-centered same as the group logo on telegram https://t.me/lunowatch!!! come with us if you are read this!!! :)
    ttgo->tft->drawXBitmap(52,73,img_lunokiot_logo_bits, img_lunokiot_logo_width,img_lunokiot_logo_height, TFT_DARKGREY);

    // text stuff
    ttgo->tft->setTextSize(1);
    ttgo->tft->setFreeFont(&FreeMonoBold9pt7b);
    ttgo->tft->setTextDatum(TR_DATUM);
    ttgo->tft->setTextWrap(false,false);
    int32_t posX = TFT_WIDTH-10;
    int32_t posY = 10;
    // more than needed buffer (in stack for guarantee their destruction)
    char buildNumberAsString[16] = { 0 };
    // dump my shit t string buffer
    sprintf(buildNumberAsString,"#%u", LUNOKIOT_BUILD_NUMBER );
    ttgo->tft->setTextColor(TFT_DARKGREY);
    ttgo->tft->drawString(buildNumberAsString, posX, posY);
    ttgo->setBrightness(BaseBackLightBrightness);
    ttgo->openBL(); // turn on the lights!
}

void SplashAnnounceEnd() {
    // 1 second bright up
    unsigned long oneSecond = millis()+1000;
    uint16_t bright = BaseBackLightBrightness;
    while(oneSecond>millis()) {
        bright++;
        delay(1000/255);
        if ( bright > 255 ) { break; }
        ttgo->setBrightness(bright);
    }
}
