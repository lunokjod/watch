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

#include "Watchface2.hpp"
#include "../lunokIoT.hpp"
#include <Arduino.h>
#include <LilyGoWatch.h> // thanks for the warnings :/
extern TTGOClass *ttgo;
#include <WiFi.h>

#include "../app/LogView.hpp" // for lLog functions
#include "../system/SystemEvents.hpp"
#include "MainMenu.hpp"

#include "../static/img_hours_hand.c"
#include "../static/img_minutes_hand.c"
#include "../static/img_seconds_hand.c"

#include "../static/img_wifi_24.xbm"
#include "../static/img_bluetooth_24.xbm"
#include "../static/img_bluetooth_peer_24.xbm"
#include "../static/img_usb_24.xbm"

#include "../static/img_weather_200.c"
#include "../static/img_weather_300.c"
#include "../static/img_weather_500.c"
#include "../static/img_weather_600.c"

#include "../static/img_weather_800.c"
#include "Stopwatch.hpp"

int currentSec = random(0, 60);
int currentMin = random(0, 60);
int currentHour = random(0, 24);
int currentDay = random(1, 30);
int currentMonth = random(0, 11);

extern bool bleEnabled;        //@TODO this is a crap!!!
extern volatile bool bleServiceRunning; //@TODO this is a crap!!!
extern bool blePeer;

extern float PMUBattDischarge;

extern bool weatherSyncDone;
extern int weatherId;
extern double weatherTemp;
extern char *weatherMain;
extern char *weatherDescription;
extern char *weatherIcon;

Watchface2Application::~Watchface2Application() {
    delete bottomRightButton;
    delete hourHandCanvas;
    delete minuteHandCanvas;
    delete innerSphere;
    delete outherSphere;
    delete colorBuffer;
    lAppLog("Watchface is gone\n");
}

Watchface2Application::Watchface2Application() {
    directDraw=false;
    middleX = canvas->width() / 2;
    middleY = canvas->height() / 2;
    radius = (canvas->width() + canvas->height()) / 4;

    bottomRightButton = new ActiveRect(160, 160, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new MainMenuApplication()); });
    if ( nullptr == bottomRightButton ) {
        lAppLog("Unable to allocate bottomRightButton\n");
        return;
    }

    colorBuffer = new CanvasZWidget(canvas->width(), canvas->height());
    if ( nullptr == colorBuffer ) {
        lAppLog("Unable to allocate colorbuffer at: %p\n",this);
        return;
    }
    colorBuffer->canvas->fillSprite(TFT_BLACK);
    outherSphere = new CanvasZWidget(canvas->width(), canvas->height());
    if ( nullptr == outherSphere ) {
        lAppLog("Unable to allocate outherSphere at: %p\n",this);
        return;
    }
    outherSphere->canvas->fillSprite(TFT_BLACK);

    hourHandCanvas = new CanvasZWidget(img_hours_hand.height, img_hours_hand.width);
    if ( nullptr == hourHandCanvas ) {
        lAppLog("Unable to allocate hourHandCanvas at: %p\n",this);
        return;
    }
    hourHandCanvas->canvas->pushImage(0, 0, img_hours_hand.width, img_hours_hand.height, (uint16_t *)img_hours_hand.pixel_data);
    hourHandCanvas->canvas->setPivot(12, 81);

    minuteHandCanvas = new CanvasZWidget(img_minutes_hand.height, img_minutes_hand.width);
    if ( nullptr == minuteHandCanvas ) {
        lAppLog("Unable to allocate minuteHandCanvas at: %p\n",this);
        return;
    }
    minuteHandCanvas->canvas->pushImage(0, 0, img_minutes_hand.width, img_minutes_hand.height, (uint16_t *)img_minutes_hand.pixel_data);
    minuteHandCanvas->canvas->setPivot(12, 107);

    // Buttons
    outherSphere->canvas->fillRect(0, 0, middleX, middleY, ttgo->tft->color24to16(0x000408));
    outherSphere->canvas->fillRect(middleX, 0, middleX, middleY, ttgo->tft->color24to16(0x000408));
    outherSphere->canvas->fillRect(0, middleY, middleX, middleY, ttgo->tft->color24to16(0x000408));
    outherSphere->canvas->fillRect(middleX, middleY, middleX, middleY, ttgo->tft->color24to16(0x102048));
    outherSphere->canvas->fillCircle(middleX, middleY, radius + (margin * 2), TFT_BLACK);

    // outher circle
    outherSphere->canvas->fillCircle(middleX, middleY, radius - margin, ttgo->tft->color24to16(0x384058));
    outherSphere->canvas->fillCircle(middleX, middleY, radius - (margin * 2), TFT_BLACK);

    DescribeCircle(middleX, middleY, radius - (margin * 2.5),
        [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload) {
            if (3 == (angle % 6))
            { // dark bites
                outherSphere->canvas->fillCircle(x, y, 5, TFT_BLACK);
            }
            return true;
        });
    DescribeCircle(middleX, middleY, radius - (margin * 2),
        [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload) {
            if (0 == (angle % 30))
            { // blue 5mins bumps
                outherSphere->canvas->fillCircle(x, y, 5, ttgo->tft->color24to16(0x384058));
            }
            return true;
        });
    outherSphere->canvas->fillRect(0, middleY - 40, 40, 80, TFT_BLACK); // left cut

    outherSphere->DrawTo(colorBuffer->canvas, 0, 0, 1.0, false, TFT_BLACK);

    // inner quarter
    innerSphere = new CanvasZWidget(canvas->width() / 2, canvas->height() / 2, 1.0, 8);
    innerSphere->canvas->fillSprite(TFT_BLACK);
    innerSphere->canvas->fillCircle(0, 0, 95, ttgo->tft->color24to16(0x182858));
    innerSphere->canvas->fillCircle(0, 0, 45, TFT_BLACK);
    innerSphere->DrawTo(colorBuffer->canvas, middleX, middleY, 1.0, false, TFT_BLACK);

    colorBuffer->DrawTo(canvas, 0, 0); // for splash
    // directDraw=true;
}

bool Watchface2Application::Tick() {
    bottomRightButton->Interact(touched, touchX, touchY);

    if (millis() > nextRefresh) {
        // hourHandCanvas->DrawTo(canvas,middleX,middleY);
        // minuteHandCanvas->DrawTo(canvas,middleX,middleY);
        // directDraw=false;
        colorBuffer->DrawTo(canvas, 0, 0);

        // time mark
        DescribeCircle(middleX, middleY, radius - 1,
                       [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload)
                       {
                           int angleDisplaced = (markAngle + 90) % 360;
                           if ((angle > markAngle) && (angle < angleDisplaced))
                           {
                               canvas->fillCircle(x, y, 1, ttgo->tft->color24to16(0x5890e8));
                           }
                           return true;
                       });

        time_t now;
        struct tm *timeinfo;
        time(&now);
        timeinfo = localtime(&now);
        char *textBuffer = (char *)ps_malloc(255);

        currentSec = timeinfo->tm_sec;
        currentMin = timeinfo->tm_min;
        currentHour = timeinfo->tm_hour;
        currentDay = timeinfo->tm_mday;
        currentMonth = timeinfo->tm_mon;

        // lAppLog("TIMEINFO: %02d:%02d:%02d\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);

        if (weatherSyncDone) {
            // weather icon
            canvas->setBitmapColor(ThCol(mark), TFT_BLACK);
            // watchFaceCanvas->canvas->fillRect(120 - (img_weather_200.width/2),52,80,46,TFT_BLACK);
            if (-1 != weatherId)
            {
                if ((200 <= weatherId) && (300 > weatherId))
                {
                    canvas->pushImage(120 - (img_weather_200.width / 2), 52, img_weather_200.width, img_weather_200.height, (uint16_t *)img_weather_200.pixel_data);
                }
                else if ((300 <= weatherId) && (400 > weatherId))
                {
                    canvas->pushImage(120 - (img_weather_300.width / 2), 52, img_weather_300.width, img_weather_300.height, (uint16_t *)img_weather_300.pixel_data);
                }
                else if ((500 <= weatherId) && (600 > weatherId))
                {
                    canvas->pushImage(120 - (img_weather_500.width / 2), 52, img_weather_500.width, img_weather_500.height, (uint16_t *)img_weather_500.pixel_data);
                }
                else if ((600 <= weatherId) && (700 > weatherId))
                {
                    canvas->pushImage(120 - (img_weather_600.width / 2), 52, img_weather_600.width, img_weather_600.height, (uint16_t *)img_weather_600.pixel_data);
                }
                else if ((700 <= weatherId) && (800 > weatherId))
                {
                    //lAppLog("@TODO Watchface: openweather 700 condition code\n");
                    // watchFaceCanvas->canvas->pushImage(120 - (img_weather_800.width/2) ,52,img_weather_800.width,img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
                    canvas->pushImage(120 - (img_weather_800.width / 2), 52, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
                    // watchFaceCanvas->canvas->pushImage(144,52,img_weather_600.width,img_weather_600.height, (uint16_t *)img_weather_600.pixel_data);
                }
                else if ((800 <= weatherId) && (900 > weatherId))
                {
                    canvas->pushImage(120 - (img_weather_800.width / 2), 52, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
                }
            }
            // temperature
            if (-1000 != weatherTemp)
            {
                sprintf(textBuffer, "%2.1f C", weatherTemp);
                int16_t posX = 150;
                int16_t posY = 74;
                canvas->setTextFont(0);
                canvas->setTextSize(2);
                canvas->setTextColor(TFT_WHITE);
                canvas->setTextDatum(TL_DATUM);
                canvas->drawString(textBuffer, posX, posY); //, 185, 96);
            }

            if (nullptr != weatherMain) {
                canvas->setTextFont(0);
                canvas->setTextSize(2);
                canvas->setTextDatum(CC_DATUM);
                canvas->setTextWrap(false, false);
                canvas->setTextColor(TFT_WHITE);
                canvas->drawString(weatherMain, 120, 40);
            }

            if (nullptr != weatherDescription) {
                canvas->setTextFont(0);
                canvas->setTextSize(1);
                canvas->setTextDatum(CC_DATUM);
                canvas->setTextWrap(false, false);
                canvas->setTextColor(TFT_BLACK);
                canvas->drawString(weatherDescription, 122, 62);
                canvas->setTextColor(TFT_WHITE);
                canvas->drawString(weatherDescription, 120, 60);
            }
        }

        // connectivity notifications
        if (bleEnabled) {
            int16_t posX = 36;
            int16_t posY = 171;
            uint32_t dotColor = TFT_DARKGREY; // enabled but service isn't up yet
            if (bleServiceRunning) {
                dotColor = ThCol(medium);
            }
            if (blePeer) {
                dotColor = ThCol(low);
            }
            canvas->fillCircle(posX, posY, 5, dotColor);
            unsigned char *img = img_bluetooth_24_bits; // bluetooth logo only icon
            if (blePeer) {
                img = img_bluetooth_peer_24_bits;
            } // bluetooth with peer icon
            canvas->drawXBitmap(posX + 10, posY - 12, img, img_bluetooth_24_width, img_bluetooth_24_height, ThCol(text));
        }
        wl_status_t whatBoutWifi = WiFi.status();
        if ( (WL_NO_SHIELD != whatBoutWifi) && (WL_IDLE_STATUS != whatBoutWifi) ) {
            //lNetLog("WiFi in use, status: %d\n", whatBoutWifi);
            int16_t posX = 51;
            int16_t posY = 189;
            uint32_t dotColor = ThCol(background);
            if ( WL_CONNECTED == whatBoutWifi ) { dotColor = ThCol(low); }
            else if ( WL_CONNECT_FAILED == whatBoutWifi ) { dotColor = ThCol(high); }
            canvas->fillCircle(posX, posY, 5, dotColor);
            canvas->drawXBitmap(posX + 10, posY - 12, img_wifi_24_bits, img_wifi_24_width, img_wifi_24_height, ThCol(text));
        }

        // seconds hand
        float secAngle = (timeinfo->tm_sec * 6);
        DescribeCircle(middleX, middleY, 110,
                       [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload)
                       {
                           if (int(secAngle) == angle)
                           {
                               canvas->fillCircle(x, y, 5, TFT_BLACK);
                               canvas->drawLine(x, y, cx, cy, ThCol(clock_hands_second));
                               canvas->fillCircle(x, y, 4, ThCol(clock_hands_second));
                               return false;
                           }
                           return true;
                       });

        float minuteAngle = (timeinfo->tm_min * 6);
        // hour hand
        float hourAngle = ((timeinfo->tm_hour) % 12) * 30;
        float correctedHoureAngle = hourAngle + (minuteAngle / 12.0);
        // lAppLog("hourAngle: %f\n",correctedHoureAngle);
        DescribeCircle(middleX, middleY, 90,
                       [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload)
                       {
                           if (int(correctedHoureAngle) == angle)
                           {
                               canvas->fillCircle(x, y, 9, TFT_BLACK);
                               canvas->drawLine(x, y, cx, cy, ThCol(highlight));
                               canvas->fillCircle(x, y, 8, ThCol(highlight));
                               // canvas->drawLine(x,y,cx,cy,ttgo->tft->color24to16(0xa0a8c0));
                               // canvas->fillCircle(x,y,8,tft->color24to16(0xa0a8c0));
                               return false;
                           }
                           return true;
                       });

        // minutes hand
        float correctedMinuteAngle = minuteAngle + (secAngle / 60.0);
        DescribeCircle(middleX, middleY, 100,
                       [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload)
                       {
                           if (int(correctedMinuteAngle) == angle)
                           {
                               canvas->fillCircle(x, y, 7, TFT_BLACK);
                               canvas->drawLine(x, y, cx, cy, ttgo->tft->color24to16(0x787ca0));
                               canvas->fillCircle(x, y, 6, ttgo->tft->color24to16(0x787ca0));
                               return false;
                           }
                           return true;
                       });
        canvas->fillCircle(middleX, middleY, 10, ThCol(clock_hands_second)); // second cap
        canvas->fillCircle(middleX, middleY, 9, TFT_BLACK);
        canvas->fillCircle(middleX, middleY, 8, ttgo->tft->color24to16(0x787ca0)); // minute cap
        canvas->fillCircle(middleX, middleY, 4, TFT_BLACK);
        canvas->fillCircle(middleX, middleY, 3, ThCol(highlight)); // hour cap

        // time
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextWrap(false, false);
        sprintf(textBuffer, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);

        // canvas->setTextColor(TFT_BLACK);
        // canvas->setTextDatum(CR_DATUM);
        // canvas->drawString(textBuffer, middleX-22,middleY+2);
        // canvas->drawString(textBuffer, middleX-18,middleY-2);

        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(CR_DATUM);
        canvas->drawString(textBuffer, middleX - 20, middleY);

        // date
        sprintf(textBuffer, "%02d/%02d", timeinfo->tm_mday, timeinfo->tm_mon + 1);
        canvas->setTextDatum(BL_DATUM);
        // canvas->setTextColor(TFT_BLACK);
        // canvas->drawString(textBuffer, middleX+22,middleY+2);
        // canvas->drawString(textBuffer, middleX+18,middleY-2);
        canvas->setTextColor(TFT_WHITE);
        canvas->drawString(textBuffer, middleX + 35, middleY - 15);

        // steps
        uint32_t activityColor = TFT_WHITE;
        bool doingSomething = false;
        int16_t posX = 40;
        int16_t posY = 74;
        if (vbusPresent)
        {
            activityColor = TFT_DARKGREY;
        }
        else if (nullptr != currentActivity)
        {
            if (0 == strcmp("BMA423_USER_WALKING", currentActivity))
            {
                activityColor = TFT_GREEN;
                doingSomething = true;
            }
            else if (0 == strcmp("BMA423_USER_RUNNING", currentActivity))
            {
                activityColor = TFT_YELLOW;
                doingSomething = true;
            }
            if (doingSomething)
            {
                canvas->fillCircle(posX - 10, posY + 6, 5, activityColor);
            }
        }
        canvas->setTextColor(activityColor);
        canvas->setTextSize(2);
        canvas->setTextDatum(TL_DATUM);
        sprintf(textBuffer, "%d", stepCount);
        canvas->drawString(textBuffer, posX, posY);

        if (vbusPresent)
        {
            posX = 69;  // 60;
            posY = 203; // 190;
            uint32_t color = ThCol(background);
            if (ttgo->power->isChargeing())
            {
                color = ThCol(low);
            }
            canvas->fillCircle(posX, posY, 5, color);
            canvas->drawXBitmap(posX + 10, posY - 12, img_usb_24_bits, img_usb_24_width, img_usb_24_height, TFT_WHITE);
        }

        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextDatum(TL_DATUM);
        canvas->setTextWrap(false, false);

        // batt pc
        if (batteryPercent > -1) {
            uint32_t battColor = TFT_DARKGREY;
            posX = 26;
            posY = 149;
            bool battActivity = false;
            if (vbusPresent)
            {
                battColor = TFT_GREEN;
                battActivity = true;
            }
            else if (batteryPercent < 10)
            {
                battActivity = true;
                battColor = ThCol(high);
            }
            else if (batteryPercent < 35)
            {
                battColor = TFT_YELLOW;
            }
            // else if ( batteryPercent > 70 ) { battColor = TFT_GREEN; }
            canvas->setTextColor(battColor);
            int battFiltered = batteryPercent;
            if (battFiltered > 99)
            {
                battFiltered = 99;
            }
            sprintf(textBuffer, "%2d%%", battFiltered);
            canvas->setTextDatum(CL_DATUM);
            canvas->drawString(textBuffer, posX + 10, posY + 1);
            canvas->fillCircle(posX, posY, 5, battColor); // Alarm: red dot

        }
        else
        {
            uint32_t battColor = TFT_DARKGREY;
            posX = 26;
            posY = 149;
            canvas->setTextFont(0);
            canvas->setTextSize(2);
            canvas->setTextColor(battColor);
            canvas->setTextDatum(CL_DATUM);
            canvas->drawString("No batt", posX + 10, posY + 1);
            canvas->fillCircle(posX, posY, 5, ThCol(high)); // Alarm: red dot
        }
        if ( 0 != StopwatchApplication::starTime ) {
            unsigned long diff= millis()-StopwatchApplication::starTime;
            int seconds = diff / 1000;
            diff %= 1000;
            int minutes = seconds / 60;
            seconds %= 60;
            int hours = minutes / 60;
            minutes %= 60;
            canvas->setTextFont(0);
            canvas->setTextSize(1);
            canvas->setTextColor(ThCol(text));
            canvas->setTextDatum(CR_DATUM);
            sprintf(textBuffer,"%02d:%02d:%02d.%03d",hours,minutes,seconds,diff);
            canvas->drawString(textBuffer, middleX - 15, middleY-14);

        }

        free(textBuffer);
        const int16_t margin = 15;
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextColor(ThCol(mark));
        canvas->setTextDatum(TC_DATUM);
        canvas->drawString("12", TFT_WIDTH / 2, margin);
        canvas->setTextDatum(CR_DATUM);
        canvas->drawString("3", TFT_WIDTH - margin, TFT_HEIGHT / 2);
        canvas->setTextDatum(BC_DATUM);
        canvas->drawString("6", TFT_WIDTH / 2, TFT_HEIGHT - margin);
        canvas->setTextDatum(CL_DATUM);
        canvas->drawString("9", margin, TFT_HEIGHT / 2);

        nextRefresh = millis() + (1000 / 4);
        return true;
    }
    return false;
}
