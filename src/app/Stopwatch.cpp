#include <Arduino.h>
#include "Stopwatch.hpp"
#include "../lunokiot_config.hpp"

#include "../static/img_play_48.xbm"
#include "../static/img_reload_48.xbm"
#include "../static/img_pause_48.xbm"

#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo lib

unsigned long StopwatchApplication::pauseTime=0;
unsigned long StopwatchApplication::starTime=0;

StopwatchApplication::~StopwatchApplication() {
    delete resetBtn;
    delete pauseBtn;
    delete startBtn;
}

StopwatchApplication::StopwatchApplication() {
    TemplateApplication();
    canvas->setTextFont(0);
    canvas->setTextSize(3);
    canvas->setTextColor(TFT_WHITE);
    canvas->setTextDatum(CC_DATUM);

    startBtn = new ButtonImageXBMWidget(10,100,70,70,[this]() {
        starTime=millis();
        pauseTime=0;
    },img_play_48_bits,img_play_48_height,img_play_48_width);

    resetBtn = new ButtonImageXBMWidget(10+74,100,70,70,[this]() {
        starTime=0;
        pauseTime=0;
    },img_reload_48_bits,img_reload_48_height,img_reload_48_width);

    pauseBtn = new ButtonImageXBMWidget(10+(74*2),100,70,70,[this]() {
        if ( 0 == starTime ) { return; }
        if ( 0 == pauseTime ) {
            pauseTime=millis();
            return;
        }
        unsigned long diff=pauseTime-starTime;
        starTime=millis()-diff;
        pauseTime=0;

    },img_pause_48_bits,img_pause_48_height,img_pause_48_width);
    Tick();
}
bool StopwatchApplication::Tick() {
    TemplateApplication::Tick();
    startBtn->Interact(touched,touchX,touchY);
    resetBtn->Interact(touched,touchX,touchY);
    pauseBtn->Interact(touched,touchX,touchY);
    if ( 0 != starTime ) { // dont sleep when is in use
        UINextTimeout = millis()+UITimeout;
    }
    if (millis() > nextRedraw ) {
        char displayText[16] = { 0 };
        if ( starTime > 0 ) {
            unsigned long diff= millis()-starTime;
            if ( pauseTime > 0 ) { diff=pauseTime-starTime; blink=(!blink); }
            else { blink = false; }
            int seconds = diff / 1000;
            diff %= 1000;
            int minutes = seconds / 60;
            seconds %= 60;
            int hours = minutes / 60;
            minutes %= 60;
            if ( false == blink ) {
                sprintf(displayText,"%02d:%02d:%02d.%03d",hours,minutes,seconds,diff);
            }
        } else {
            sprintf(displayText,"00:00:00.000");
        }
        canvas->fillRoundRect(0,40,TFT_WIDTH,40,10,TFT_BLACK); // ThCol(background));
        canvas->drawString(displayText,TFT_WIDTH/2,60);
        TemplateApplication::Tick();

        startBtn->DrawTo(canvas);
        resetBtn->DrawTo(canvas);
        pauseBtn->DrawTo(canvas);
        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}
