//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//    Watchface Author: @hpsaturn
//    License: GPLv3
//----------------------------------------------------------------

#include "WatchfaceSquare.hpp"
#include "../UI/UI.hpp"
#define MARGIN_TOP 3
#define MARGIN_LFT 2
#define WTI_POS_Y 37    // Weather icon
#define WT_MISC_MC_X 70    // Weather icon

char const *DAY[]={"SUN","MON","TUE","WED","THU","FRI","SAT"};

extern float PMUBattDischarge;

extern bool weatherSyncDone;
extern int weatherId;
extern double weatherTemp;
extern char *weatherMain;
extern char *weatherDescription;
extern char *weatherIcon;
extern double weatherFeelsTemp;
extern double weatherTempMin;
extern double weatherTempMax;
extern double weatherHumidity;
extern double weatherPressure;
extern double weatherWindSpeed;

WatchfaceSquare::WatchfaceSquare() {
    lAppLog("WatchfaceSquare On\n");

    topRightButton = new ActiveRect(160, 0, 80, 80, [](void *unused) {
        LaunchApplication(new BatteryApplication());
    });
    if ( nullptr == topRightButton ) {
        lAppLog("Unable to allocate topRightButton\n");
        return;
    }

    bottomRightButton = new ActiveRect(160, 160, 80, 80, [](void *unused) {
        LaunchApplication(new LuIMainMenuApplication());
    });
    if ( nullptr == bottomRightButton ) {
        lAppLog("Unable to allocate bottomRightButton\n");
        return;
    }

    topLeftButton = new ActiveRect(0, 0, 80, 80, [](void *unused) {
        LaunchApplication(new SettingsApplication());
    });

    if ( nullptr == topLeftButton ) {
        lAppLog("Unable to allocate bottomTopButton\n");
        return;
    }

    bottomLeftButton = new ActiveRect(0, 160, 80, 80, [](void *unused) {
        LaunchApplication(new StepsApplication());
    });
    if ( nullptr == bottomLeftButton ) {
        lAppLog("Unable to allocate bottomLeftButton\n");
        return;
    }
    // Tick(); // OR call this if no splash 
  SetUserBrightness();
}


WatchfaceSquare::~WatchfaceSquare() {
  // please remove/delete/free all to avoid leaks!
  if ( nullptr == topRightButton ) { delete topRightButton; } // why check? maybe the app cannot reach allocate of elements
  if ( nullptr == bottomRightButton ) { delete bottomRightButton; }
  if ( nullptr == topLeftButton ) { delete topLeftButton; }
  if ( nullptr == bottomLeftButton ) { delete bottomLeftButton; }
  lAppLog("Watchface Square is gone\n");
}

bool WatchfaceSquare::Tick() {

    topRightButton->Interact(touched, touchX, touchY);
    bottomRightButton->Interact(touched, touchX, touchY);
    topLeftButton->Interact(touched, touchX, touchY);
    bottomLeftButton->Interact(touched, touchX, touchY);
    // low the brightness if system changes it
    // if ( MINIMUM_BACKLIGHT != ttgo->bl->getLevel() ) { ttgo->setBrightness(MINIMUM_BACKLIGHT); }

    if ( millis() > nextRefresh ) { // redraw full canvas


        char *textBuffer = (char *)ps_malloc(255);
        canvas->fillSprite(TFT_BLACK); // use theme colors
 
        time_t now;
        struct tm * tmpTime;
        struct tm timeinfo;
        time(&now);
        tmpTime = localtime(&now);
        memcpy(&timeinfo,tmpTime, sizeof(struct tm));
        canvas->fillSprite(TFT_BLACK);
        // update day and month        
        canvas->setTextSize(4);
        canvas->setFreeFont(&TomThumb);
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(TL_DATUM);
        sprintf(textBuffer,"%02d.%02d", timeinfo.tm_mday, timeinfo.tm_mon+1);
        canvas->drawString(textBuffer, MARGIN_LFT, MARGIN_TOP);
        // update day of week
        int weekday=timeinfo.tm_wday;
        canvas->setTextColor(TFT_CYAN);
        canvas->setTextDatum(TC_DATUM);
        sprintf(textBuffer,"%s",DAY[weekday]);
        canvas->drawString(textBuffer, TFT_WIDTH/2+15, MARGIN_TOP);
        // update hours
        canvas->setTextSize(3);
        canvas->setFreeFont(&FreeMonoBold18pt7b);
        canvas->setTextColor(TFT_CYAN);
        canvas->setTextDatum(TR_DATUM);
        sprintf(textBuffer,"%02d", timeinfo.tm_hour);
        canvas->drawString(textBuffer, TFT_WIDTH+7, 30);
        // update minutes
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(CR_DATUM);
        sprintf(textBuffer,"%02d",timeinfo.tm_min);
        canvas->drawString(textBuffer, TFT_WIDTH+7, TFT_HEIGHT/2+17);
        // update battery 
        canvas->setFreeFont(&TomThumb);
        canvas->setTextSize(4);
        
        if (batteryPercent > -1) {
            uint32_t battColor = TFT_DARKGREY;
            bool battActivity = false;
            if (vbusPresent) battColor = TFT_GREEN;
            else if (batteryPercent < 10) battColor = ThCol(high);
            else if (batteryPercent < 35) battColor = TFT_YELLOW;
            else battColor = TFT_WHITE;
            canvas->setTextColor(battColor);
            int battFiltered = batteryPercent;
            if (battFiltered > 99) battFiltered = 99;
            sprintf(textBuffer, "%2d", battFiltered);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString(textBuffer, TFT_WIDTH-5, MARGIN_TOP);
            if(battFiltered == 0) battFiltered=1;
            canvas->fillRect(TFT_WIDTH-16,MARGIN_TOP,12,(int)(20/(100/battFiltered)),battColor);
        }
        else {
            canvas->setTextColor(TFT_DARKGREY);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString("NB", TFT_WIDTH-10, MARGIN_TOP);
        }

        if (weatherSyncDone) {
          // weather icon
          canvas->setBitmapColor(ThCol(mark), TFT_BLACK);
          // watchFaceCanvas->canvas->fillRect(120 - (img_weather_200.width/2),52,80,46,TFT_BLACK);
          if (-1 != weatherId) {
            if ((200 <= weatherId) && (300 > weatherId)) {
              canvas->pushImage(MARGIN_LFT+5, WTI_POS_Y, img_weather_200.width, img_weather_200.height, (uint16_t *)img_weather_200.pixel_data);
            } else if ((300 <= weatherId) && (400 > weatherId)) {
              canvas->pushImage(MARGIN_LFT+5, WTI_POS_Y, img_weather_300.width, img_weather_300.height, (uint16_t *)img_weather_300.pixel_data);
            } else if ((500 <= weatherId) && (600 > weatherId)) {
              canvas->pushImage(MARGIN_LFT+5, WTI_POS_Y, img_weather_500.width, img_weather_500.height, (uint16_t *)img_weather_500.pixel_data);
            } else if ((600 <= weatherId) && (700 > weatherId)) {
              canvas->pushImage(MARGIN_LFT+5, WTI_POS_Y, img_weather_600.width, img_weather_600.height, (uint16_t *)img_weather_600.pixel_data);
            } else if ((700 <= weatherId) && (800 > weatherId)) {
              //lAppLog("@TODO Watchface: openweather 700 condition code\n");
              canvas->pushImage(MARGIN_LFT+5, WTI_POS_Y, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
            } else if ((800 <= weatherId) && (900 > weatherId)) {
              canvas->pushImage(MARGIN_LFT+5, WTI_POS_Y, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
            }
          }
          // temperature
          if (-1000 != weatherTemp) {
            sprintf(textBuffer, "%3.1f", weatherTemp);
            canvas->setFreeFont(&TomThumb);
            canvas->setTextSize(4);
            canvas->setTextColor(TFT_WHITE);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString(textBuffer,MARGIN_LFT+img_weather_200.width+95, WTI_POS_Y); 
          }

          if (-1000 != weatherFeelsTemp) {
            sprintf(textBuffer, "%3.1f", weatherFeelsTemp);
            canvas->setFreeFont(&TomThumb);
            canvas->setTextSize(4);
            canvas->setTextColor(TFT_WHITE);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString(textBuffer,MARGIN_LFT+img_weather_200.width+95, WTI_POS_Y+23);  
          }

          if (nullptr != weatherMain) {
            canvas->setTextSize(3);
            canvas->setTextDatum(MC_DATUM);
            canvas->setTextColor(TFT_CYAN);
            canvas->drawString(weatherMain, WT_MISC_MC_X, 95);
          }

          if (-1000 != weatherTempMin && -1000 != weatherTempMax) {
            sprintf(textBuffer, "TL: %3.1f TM: %3.1f", weatherTempMin,weatherTempMax);
            canvas->setTextSize(2);
            canvas->setTextColor(TFT_WHITE);
            canvas->setTextDatum(MC_DATUM);
            canvas->drawString(textBuffer, WT_MISC_MC_X, 120);  
          }

          sprintf(textBuffer, "Wind: %4.1f Kmh", weatherWindSpeed);
          canvas->setTextSize(2);
          canvas->setTextDatum(MC_DATUM);
          canvas->setTextColor(TFT_WHITE);
          canvas->drawString(textBuffer, WT_MISC_MC_X, 137);

          free(textBuffer); 
        }

        // steps
        uint32_t activityColor = TFT_WHITE;
        bool doingSomething = false;
        if (vbusPresent) {
          activityColor = TFT_DARKGREY;
        } else if (nullptr != currentActivity) {
          if (0 == strcmp("BMA423_USER_WALKING", currentActivity)) {
            activityColor = TFT_GREEN;
            doingSomething = true;
          } else if (0 == strcmp("BMA423_USER_RUNNING", currentActivity)) {
            activityColor = TFT_YELLOW;
            doingSomething = true;
          }
          if (doingSomething) {
            canvas->fillCircle(120,TFT_HEIGHT-10, 5, activityColor);
          }
        }
        canvas->setTextColor(activityColor);
        canvas->setTextSize(4);
        canvas->setTextDatum(BR_DATUM);
        
        canvas->drawXBitmap(MARGIN_LFT+75,200,img_step_32_bits,img_step_32_width,img_step_32_height,ThCol(text));
        sprintf(textBuffer, "%06dS", stepCount);
        canvas->drawString(textBuffer, TFT_WIDTH, TFT_HEIGHT);
        int distance = (int) (stepCount * stepDistanceCm) / 100;
        sprintf(textBuffer, "%06dM", distance);
        canvas->drawString(textBuffer, TFT_WIDTH, TFT_HEIGHT-23);

        // connectivity notifications
        wl_status_t whatBoutWifi = WiFi.status();
        if ((WL_NO_SHIELD != whatBoutWifi) && (WL_IDLE_STATUS != whatBoutWifi)) {
          lNetLog("WiFi in use, status: %d\n", whatBoutWifi);
          if (WL_CONNECTED == whatBoutWifi) {
            canvas->drawXBitmap(MARGIN_LFT + 30, 205, img_wifi_24_bits, img_wifi_24_width, img_wifi_24_height, ThCol(text));
          } else if (WL_CONNECT_FAILED == whatBoutWifi) {
            canvas->drawXBitmap(MARGIN_LFT + 30, 205, img_wifi_24_bits, img_wifi_24_width, img_wifi_24_height, ThCol(text));
          }
        }

        if (LoT().GetBLE()->IsEnabled()) {
          unsigned char *img = img_bluetooth_24_bits;  // bluetooth logo only icon
          if (LoT().GetBLE()->Clients() > 0) {
            img = img_bluetooth_peer_24_bits;
          }  // bluetooth with peer icon
          canvas->drawXBitmap(MARGIN_LFT, 205, img, img_bluetooth_24_width, img_bluetooth_24_height, ThCol(text));
        }

        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI

        return true;
    }
    return false;
}

void WatchfaceSquare::LowMemory() {
    LunokIoTApplication::LowMemory(); // call parent
    lAppLog("YEAH WATCHFACE RECEIVED TOO\n");
}