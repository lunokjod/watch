#include <Arduino.h>
#include <LilyGoWatch.h>

#include <esp_task_wdt.h> // used for reset task watchdog

#include "BootSplash.hpp" // public methods

#include "lunokiot_config.hpp" 

#if defined(LILYGO_WATCH_2020_V1)||defined(LILYGO_WATCH_2020_V3)
#include <driver/i2s.h>
#include "AudioFileSourcePROGMEM.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#endif

#include "UI.hpp"
#include "../app/LogView.hpp" // lLog shit
#include "../static/img_lunokiot_logo.xbm" // sputnik image

extern TTGOClass *ttgo; // ttgo lib

bool bootLoop = true; // this stops the animation loop
bool bootLoopEnds = false; // this is used by the splash to know bootLoop is ended


#if defined(LILYGO_WATCH_2020_V1)||defined(LILYGO_WATCH_2020_V3)
extern const PROGMEM uint8_t boot_sound_start[] asm("_binary_asset_boot_sound_mp3_start");
extern const PROGMEM uint8_t boot_sound_end[] asm("_binary_asset_boot_sound_mp3_end");

extern bool normalBoot;

void SplashFanfare() {
#ifdef LUNOKIOT_SILENT_BOOT
    lUILog("Audio: Not initialized due Silent boot is enabled\n");
    delay(927); // the delay of audio
    return;
#endif
    //unsigned long begin=millis();
    // Audio fanfare x'D
    lUILog("Audio: Initialize\n");
    
    ttgo->enableAudio();

    // from https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/examples/UnitTest/HardwareTest/HardwareTest.ino
    AudioGeneratorMP3 *mp3;
    AudioFileSourcePROGMEM *file;
    AudioOutputI2S *out;
    AudioFileSourceID3 *id3;

    file = new AudioFileSourcePROGMEM(boot_sound_start, (uint32_t)(boot_sound_end-boot_sound_start));
    id3 = new AudioFileSourceID3(file);
    out = new AudioOutputI2S();
    out->SetPinout(TWATCH_DAC_IIS_BCK, TWATCH_DAC_IIS_WS, TWATCH_DAC_IIS_DOUT);
    lUILog("Audio: MP3 boot sound\n");
    mp3 = new AudioGeneratorMP3();
    mp3->begin(id3, out);
    while (true) {
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();
            }
        } else {
            lUILog("Audio: MP3 done\n");
            break;
        }
    }
    delete file;
    delete id3;
    delete out;
    delete mp3;

    i2s_driver_uninstall(I2S_NUM_0);
    ttgo->disableAudio();
    //lLog("FANFARE TIME: %d\n",millis()-begin);
}
#endif

void SplashMeanWhile(void *data) { // task to do boot animation
    bootLoop=true;
    lUILog("Splash loop begin\n");
    TFT_eSprite *splashLoadingBar = new TFT_eSprite(ttgo->tft);
    if ( nullptr == splashLoadingBar ) {
        lUILog("SplashMeanWhile: Unable to alloc new sprite!\n");
        bootLoopEnds=true;
        vTaskDelete(NULL);
    }
    splashLoadingBar->setColorDepth(16);
    if ( NULL == splashLoadingBar->createSprite(TFT_WIDTH-60,12) ) {
        lUILog("SplashMeanWhile: Unable to create tft sprite!\n");
        bootLoopEnds=true;
        vTaskDelete(NULL);
    }
    const int32_t RADIUS=6;
    bootLoopEnds=false;
    size_t offset=0;
    //FPS=MAXFPS;
    //const TickType_t LUNOKIOT_UI_TICK_TIME =  portTICK_PERIOD_MS/4; // (FPS / portTICK_PERIOD_MS); // try to get the promised speed
    //TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
    while (bootLoop) {
        delay(20);
        splashLoadingBar->fillSprite(ThCol(boot_splash_background));
        offset++;
        if ( offset > splashLoadingBar->width() ) { break; }
        int32_t w = offset; //(offset%(splashLoadingBar->width()-RADIUS));
        splashLoadingBar->fillRoundRect(0,0,w,splashLoadingBar->height(),RADIUS,ThCol(boot_splash_foreground));
        // do something beauty meanwhile boot
        splashLoadingBar->pushSprite((TFT_WIDTH-splashLoadingBar->width())/2,(TFT_HEIGHT-splashLoadingBar->height())-10);
        offset++;
        delay(23); // this value is the speed of the loadbar
        // this task isn't subscribed to Watchdog
    }
    splashLoadingBar->deleteSprite();
    delete splashLoadingBar;
    lUILog("Splash loop end\n");
    bootLoopEnds=true;
    vTaskDelete(NULL);
}
bool SplashFormatSPIFFSAnnounce() {
    tft->setTextDatum(BC_DATUM);
    tft->drawString("SPIFFS format...",TFT_WIDTH/2,TFT_HEIGHT-30);
    return SPIFFS.begin(true);
}
void SplashAnnounce() {
    bootLoop=true;
    
    ttgo->setBrightness(0); // low brightness
    ttgo->tft->fillScreen(ThCol(boot_splash_background));
    // coords from gimp :) manual stetic-centered same as the group logo on telegram https://t.me/lunowatch!!! come with us if you read this!!! :)
    ttgo->tft->drawXBitmap(52,73,img_lunokiot_logo_bits, img_lunokiot_logo_width,img_lunokiot_logo_height, ThCol(boot_splash_foreground));
    xTaskCreate(SplashMeanWhile, "lsplash", LUNOKIOT_TINY_STACK_SIZE, nullptr, uxTaskPriorityGet(NULL), NULL);

#ifdef LUNOKIOT_DEBUG
    // text stuff
    ttgo->tft->setTextSize(1);
    ttgo->tft->setFreeFont(&FreeMonoBold9pt7b);
    ttgo->tft->setTextDatum(TR_DATUM);
    ttgo->tft->setTextWrap(false,false);
    int32_t posX = TFT_WIDTH-10;
    int32_t posY = 10;
    // more than needed buffer (in stack for guarantee their destruction)
    char buildNumberAsString[10] = { 0 };
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
    bootLoop=false;
    while(false == bootLoopEnds) {
        lUILog("Waiting splash ends...\n");
        //esp_task_wdt_reset();
        delay(100);
    }
}

void SplashAnnounceBegin() {
    // 1 second bright up
    unsigned long oneSecond = millis()+1000;
    uint16_t bright = BaseBackLightBrightness;
    while(oneSecond>millis()) {
        bright++;
        delay(1000/255);
        if ( bright > 255 ) { break; }
        ttgo->setBrightness(bright);
    }
#if defined(LILYGO_WATCH_2020_V1)||defined(LILYGO_WATCH_2020_V3)
    // do sound only if boot is normal (crash-silent)
    if ( true == normalBoot ) { SplashFanfare(); } // sound and shake
#endif
}
