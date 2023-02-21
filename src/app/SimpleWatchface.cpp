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
#include "../lunokiot_config.hpp"
extern TTGOClass *ttgo; // ttgo library shit ;)
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/ButtonWidget.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

#include "SimpleWatchface.hpp"

#include "../static/img_watchface0.c"
#include "../static/img_hours_hand.c"
#include "../static/img_minutes_hand.c"
#include "../static/img_seconds_hand.c"

#include <WiFi.h>

extern const char *ntpServer;
extern const long  gmtOffset_sec;
extern const int   daylightOffset_sec;
extern bool ntpSyncDone;
/*
// NTP sync data
const char *ntpServer       = "es.pool.ntp.org";
const long  gmtOffset_sec   = 3600;
//const int   daylightOffset_sec = 0; // winter
const int   daylightOffset_sec = 3600; // summer

bool ntpSyncDone = false;
*/

void SimpleWatchfaceApplication::NTPSync(void * data) {
    Serial.println("Watchface: Trying to get WiFi...");

    if ( ntpSyncDone ) { return; }
    WiFi.begin();
    Serial.println("NTP sync...");
    while ( WL_CONNECTED != WiFi.status()) {
        //Serial.println("WiFi: waiting for connection...");
        delay(10);
    }
    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    //timeClient.begin();
    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +1 = 3600
    // GMT +8 = 28800
    // GMT -1 = -3600
    // GMT 0 = 0
    /*
    timeClient.setTimeOffset(0);
    while(!timeClient.update()) {
        yield();
        timeClient.forceUpdate();
    }
    */
    delay(10);
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        ttgo->rtc->syncToRtc(); // WTF? magic code now works? // old: why don't work as expect? (getLocalTime to RTC)
        RTC_Date d = ttgo->rtc->getDateTime();
        if (d.year != (timeinfo.tm_year + 1900) || d.month != timeinfo.tm_mon + 1
                        || d.day !=  timeinfo.tm_mday ||  d.hour != timeinfo.tm_hour
                        || d.minute != timeinfo.tm_min) {
            Serial.printf("Write RTC Fail: '%s'\n",ttgo->rtc->formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));
        } else {
            Serial.printf("Write RTC PASS: Read RTC: '%s'\n",ttgo->rtc->formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));
        }
        Serial.println("NTP sync done!");
        WiFi.disconnect(true);
        delay(100);
        WiFi.mode(WIFI_OFF);
        ntpSyncDone = true;
    }
    Serial.println("NTP sync dies here");
    vTaskDelete(NULL);
}

SimpleWatchfaceApplication::SimpleWatchfaceApplication() {
    //LunokIoTApplication();
    watchFaceCanvas = new CanvasWidget(240,240);

    backgroundCanvas = new CanvasWidget(240,240);
    backgroundCanvas->canvas->setSwapBytes(true);
    backgroundCanvas->canvas->pushImage(0,0,img_watchface0.width,img_watchface0.height, (uint16_t *)img_watchface0.pixel_data);
    backgroundCanvas->canvas->setSwapBytes(false);

    hourHandCanvas = new CanvasWidget(img_hours_hand.height,img_hours_hand.width);
    hourHandCanvas->canvas->setSwapBytes(true);
    hourHandCanvas->canvas->pushImage(0,0,img_hours_hand.width,img_hours_hand.height, (uint16_t *)img_hours_hand.pixel_data);
    hourHandCanvas->canvas->setSwapBytes(false);
    hourHandCanvas->canvas->setPivot(10,98);



    minuteHandCanvas = new CanvasWidget(img_minutes_hand.height,img_minutes_hand.width);
    minuteHandCanvas->canvas->setSwapBytes(true);
    minuteHandCanvas->canvas->pushImage(0,0,img_minutes_hand.width,img_minutes_hand.height, (uint16_t *)img_minutes_hand.pixel_data);
    minuteHandCanvas->canvas->setSwapBytes(false);
    minuteHandCanvas->canvas->setPivot(10,102);



    secondHandCanvas = new CanvasWidget(img_seconds_hand.height,img_seconds_hand.width);
    secondHandCanvas->canvas->setSwapBytes(true);
    secondHandCanvas->canvas->pushImage(0,0,img_seconds_hand.width,img_seconds_hand.height, (uint16_t *)img_seconds_hand.pixel_data);
    secondHandCanvas->canvas->setSwapBytes(false);
    secondHandCanvas->canvas->setPivot(6,106);

    if ( false == ntpSyncDone ) {
        xTaskCreatePinnedToCore(SimpleWatchfaceApplication::NTPSync, "", LUNOKIOT_PROVISIONING_STACK_SIZE,nullptr, uxTaskPriorityGet(NULL), NULL,1);
    }

}


bool SimpleWatchfaceApplication::Tick() {
    /*
        Serial.printf("ANGLE: %.3lf\n", touchDragAngle);
        Serial.printf("Distance: %.3lf\n", touchDragDistance);
        Serial.printf("VECTOR: X: %d Y: %d\n", touchDragVectorX,touchDragVectorY);
    */

    static unsigned long nextFullredrawMs = 0;
    if ( millis() > nextFullredrawMs ) {
        backgroundCanvas->DrawTo(watchFaceCanvas->canvas);

        if ( ntpSyncDone ) {
            struct tm timeinfo;
            //char timeStr[7] = "00:00";
            if (getLocalTime(&timeinfo,10)) {


            // weekday/day/month/year
            watchFaceCanvas->canvas->setTextFont(0);
            watchFaceCanvas->canvas->setTextSize(2);
            watchFaceCanvas->canvas->setTextDatum(CC_DATUM);
            watchFaceCanvas->canvas->setTextWrap(false,false);
            int32_t posX = 0;
            int32_t posY = 120;
            struct tm timeinfo;
            char timeStr[] = "00/00/0000";
            watchFaceCanvas->canvas->setTextColor(TFT_WHITE);
            if (getLocalTime(&timeinfo)) {
                sprintf(timeStr,"%04d", 1900+timeinfo.tm_year);
                watchFaceCanvas->canvas->drawString(timeStr, posX+120+60, posY);
                sprintf(timeStr,"%02d/%02d", timeinfo.tm_mday,timeinfo.tm_mon);
                watchFaceCanvas->canvas->drawString(timeStr, posX+60, posY);
            }



                minuteHandCanvas->canvas->pushRotated(watchFaceCanvas->canvas,timeinfo.tm_min*6,CanvasWidget::MASK_COLOR);
                hourHandCanvas->canvas->pushRotated(watchFaceCanvas->canvas,timeinfo.tm_hour*30,CanvasWidget::MASK_COLOR);
                secondHandCanvas->canvas->pushRotated(watchFaceCanvas->canvas,timeinfo.tm_sec*6,CanvasWidget::MASK_COLOR); // fuck the color rotation x'D
                //sprintf(timeStr,"%02d:%02d", timeinfo.tm_hour,timeinfo.tm_min);
                //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
            }
        }

        watchFaceCanvas->DrawTo(canvas);

        nextFullredrawMs = millis()+(1000/4);
        return true;
    }
    return false;
}
