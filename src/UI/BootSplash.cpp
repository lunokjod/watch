#include "BootSplash.hpp"
#include "lunokiot_config.hpp"
extern LunokIoTThemeColorPalette * currentTheme;

#ifdef LILYGO_WATCH_2020_V3
#include <driver/i2s.h>
#include "AudioFileSourcePROGMEM.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#endif
#include "../app/LogView.hpp"
#include "../static/img_lunokiot_logo.xbm"

#ifdef LILYGO_WATCH_2020_V3
extern const uint8_t boot_sound_start[] asm("_binary_asset_boot_sound_mp3_start");
extern const uint8_t boot_sound_end[] asm("_binary_asset_boot_sound_mp3_end");

void SplashFanfare() {
#ifdef LUNOKIOT_SILENT_BOOT
    lUILog("Audio: Not initialized due Silent boot is enabled\n");
    return;
#endif

#ifdef LILYGO_WATCH_2020_V3
    // Audio fanfare x'D
    lUILog("Audio: Initialize");
    ttgo->enableAudio();

    // from https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/examples/UnitTest/HardwareTest/HardwareTest.ino
    AudioGeneratorMP3 *mp3;
    AudioFileSourcePROGMEM *file;
    AudioOutputI2S *out;
    AudioFileSourceID3 *id3;
    // file = new AudioFileSourcePROGMEM(image, sizeof(image));
    file = new AudioFileSourcePROGMEM(boot_sound_start, (uint32_t)(boot_sound_end-boot_sound_start));
    id3 = new AudioFileSourceID3(file);
    out = new AudioOutputI2S();
    out->SetPinout(TWATCH_DAC_IIS_BCK, TWATCH_DAC_IIS_WS, TWATCH_DAC_IIS_DOUT);
    lUILog("Audio: MP3 boot sound");
    mp3 = new AudioGeneratorMP3();
    mp3->begin(id3, out);
    while (true) {
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();
            }
        } else {
            lUILog("Audio: MP3 done");
            break;
        }
    }
    delete file;
    delete id3;
    delete out;
    delete mp3;

    ttgo->disableAudio();

    i2s_driver_uninstall(I2S_NUM_0);
#endif
}
#endif

void SplashAnnounce() {
    ttgo->setBrightness(0); // low brightness
    ttgo->tft->fillScreen(ttgo->tft->color24to16(currentTheme->boot_splash_background));
    // coords from gimp :) manual stetic-centered same as the group logo on telegram https://t.me/lunowatch!!! come with us if you are read this!!! :)
    ttgo->tft->drawXBitmap(52,73,img_lunokiot_logo_bits, img_lunokiot_logo_width,img_lunokiot_logo_height, ttgo->tft->color24to16(currentTheme->boot_splash_foreground));

#ifdef LUNOKIOT_DEBUG
    // text stuff
    ttgo->tft->setTextSize(1);
    ttgo->tft->setFreeFont(&FreeMonoBold9pt7b);
    ttgo->tft->setTextDatum(TR_DATUM);
    ttgo->tft->setTextWrap(false,false);
    int32_t posX = TFT_WIDTH-10;
    int32_t posY = 10;
    // more than needed buffer (in stack for guarantee their destruction)
    char buildNumberAsString[16] = { 0 };
    // dump my shit string buffer
    sprintf(buildNumberAsString,"#%u", LUNOKIOT_BUILD_NUMBER );
    ttgo->tft->setTextColor(TFT_DARKGREY);
    ttgo->tft->drawString(buildNumberAsString, posX, posY);
#endif

    ttgo->setBrightness(0);
    ttgo->openBL(); // turn on the lights!
    for(int i=0;i<255;i++) {
        ttgo->setBrightness(i);
        delay(5);
    }
    ttgo->setBrightness(BaseBackLightBrightness); // default brightness
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
#ifdef LILYGO_WATCH_2020_V3
    SplashFanfare(); // sound and shake
#endif
}
