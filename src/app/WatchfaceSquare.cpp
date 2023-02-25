//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//    Watchface Author: @hpsaturn
//    License: GPLv3
//----------------------------------------------------------------

#include "WatchfaceSquare.hpp"
#include "WatchfaceHandlers.hpp"

#define MINIMUM_BACKLIGHT 10
#define MARGIN_TOP 3
#define MARGIN_LFT 2

char const *DAY[]={"SUN","MON","TUE","WED","THU","FRI","SAT"};

WatchfaceSquare::WatchfaceSquare() {
    lAppLog("WatchfaceSquare On\n");
    directDraw=false;

    topRightButton = new ActiveRect(160, 0, 80, 80, [](void *unused) {
        LaunchApplication(new BatteryApplication());
    });
    if ( nullptr == topRightButton ) {
        lAppLog("Unable to allocate topRightButton\n");
        return;
    }

    bottomRightButton = new ActiveRect(160, 160, 80, 80, [](void *unused) {
        LaunchApplication(new MainMenuApplication());
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
    Handlers();
    // Tick(); // OR call this if no splash 
}

void WatchfaceSquare::FreeRTOSEventReceived(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    WatchfaceSquare *self = reinterpret_cast<WatchfaceSquare*>(handler_args);
    if (WIFI_EVENT == base) {
        if (WIFI_EVENT_STA_START == id) {
            self->wifiEnabled = true;
        }
        else if (WIFI_EVENT_STA_STOP == id) {
            self->wifiEnabled = false;
        }
    }
}

WatchfaceSquare::~WatchfaceSquare() {
  esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, WatchfaceSquare::FreeRTOSEventReceived);
  // please remove/delete/free all to avoid leaks!
  //delete mywidget;
  delete topRightButton;
  delete bottomRightButton;
  delete topLeftButton;
  delete bottomLeftButton;
  lAppLog("Watchface Square is gone\n");
}

bool WatchfaceSquare::Tick() {

    topRightButton->Interact(touched, touchX, touchY);
    bottomRightButton->Interact(touched, touchX, touchY);
    topLeftButton->Interact(touched, touchX, touchY);
    bottomLeftButton->Interact(touched, touchX, touchY);
    // low the brightness if system changes it
    if ( MINIMUM_BACKLIGHT != ttgo->bl->getLevel() ) { ttgo->setBrightness(MINIMUM_BACKLIGHT); }

    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(TFT_BLACK); // use theme colors

        time_t now;
        struct tm * tmpTime;
        struct tm timeinfo;
        time(&now);
        tmpTime = localtime(&now);
        memcpy(&timeinfo,tmpTime, sizeof(struct tm));
        char buffer[64] = { 0 };  // TODO: buffer using malloc
        canvas->fillSprite(TFT_BLACK);
        // update day and month        
        canvas->setTextSize(4);
        canvas->setFreeFont(&TomThumb);
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(TL_DATUM);
        sprintf(buffer,"%02d.%02d", timeinfo.tm_mday, timeinfo.tm_mon+1);
        canvas->drawString(buffer, MARGIN_LFT, MARGIN_TOP);
        // update day of week
        int weekday=timeinfo.tm_wday;
        canvas->setTextColor(TFT_CYAN);
        canvas->setTextDatum(TC_DATUM);
        sprintf(buffer,"%s",DAY[weekday]);
        canvas->drawString(buffer, TFT_WIDTH/2+15, MARGIN_TOP);
        // update hours
        canvas->setTextSize(3);
        canvas->setFreeFont(&FreeMonoBold18pt7b);
        canvas->setTextColor(TFT_CYAN);
        canvas->setTextDatum(TR_DATUM);
        sprintf(buffer,"%02d", timeinfo.tm_hour);
        canvas->drawString(buffer, TFT_WIDTH+7, 30);
        // update minutes
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(CR_DATUM);
        sprintf(buffer,"%02d",timeinfo.tm_min);
        canvas->drawString(buffer, TFT_WIDTH+7, TFT_HEIGHT/2+17);
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
            sprintf(buffer, "%2d", battFiltered);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString(buffer, TFT_WIDTH-5, MARGIN_TOP);
            if(battFiltered == 0) battFiltered=1;
            canvas->fillRect(TFT_WIDTH-16,MARGIN_TOP,12,(int)(20/(100/battFiltered)),battColor);
        }
        else {
            canvas->setTextColor(TFT_DARKGREY);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString("NB", TFT_WIDTH-10, MARGIN_TOP);
        }

        if (weatherSyncDone) {
          char *textBuffer = (char *)ps_malloc(255);
          // weather icon
          canvas->setBitmapColor(ThCol(mark), TFT_BLACK);
          // watchFaceCanvas->canvas->fillRect(120 - (img_weather_200.width/2),52,80,46,TFT_BLACK);
          if (-1 != wfhandler.weatherId) {
            if ((200 <= wfhandler.weatherId) && (300 > wfhandler.weatherId)) {
              canvas->pushImage(MARGIN_LFT+5, 35, img_weather_200.width, img_weather_200.height, (uint16_t *)img_weather_200.pixel_data);
            } else if ((300 <= wfhandler.weatherId) && (400 > wfhandler.weatherId)) {
              canvas->pushImage(MARGIN_LFT+5, 35, img_weather_300.width, img_weather_300.height, (uint16_t *)img_weather_300.pixel_data);
            } else if ((500 <= wfhandler.weatherId) && (600 > wfhandler.weatherId)) {
              canvas->pushImage(MARGIN_LFT+5, 35, img_weather_500.width, img_weather_500.height, (uint16_t *)img_weather_500.pixel_data);
            } else if ((600 <= wfhandler.weatherId) && (700 > wfhandler.weatherId)) {
              canvas->pushImage(MARGIN_LFT+5, 35, img_weather_600.width, img_weather_600.height, (uint16_t *)img_weather_600.pixel_data);
            } else if ((700 <= wfhandler.weatherId) && (800 > wfhandler.weatherId)) {
              //lAppLog("@TODO Watchface: openweather 700 condition code\n");
              canvas->pushImage(MARGIN_LFT+5, 35, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
            } else if ((800 <= wfhandler.weatherId) && (900 > wfhandler.weatherId)) {
              canvas->pushImage(MARGIN_LFT+5, 35, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
            }
          }
          // temperature
          if (-1000 != wfhandler.weatherTemp) {
            sprintf(textBuffer, "%3.1f", wfhandler.weatherTemp);
            canvas->setFreeFont(&TomThumb);
            canvas->setTextSize(4);
            canvas->setTextColor(TFT_WHITE);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString(textBuffer,MARGIN_LFT+img_weather_200.width+80, 35);  //, 185, 96);
          }

          if (-1000 != wfhandler.weatherFTemp) {
            sprintf(textBuffer, "%3.1f", wfhandler.weatherFTemp);
            canvas->setFreeFont(&TomThumb);
            canvas->setTextSize(4);
            canvas->setTextColor(TFT_WHITE);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString(textBuffer,MARGIN_LFT+img_weather_200.width+80, 60);  //, 185, 96);
          }

          if (nullptr != wfhandler.weatherMain) {
            canvas->setTextFont(0);
            canvas->setTextSize(2);
            canvas->setTextDatum(TL_DATUM);
            canvas->setTextWrap(false, false);
            canvas->setTextColor(TFT_WHITE);
            canvas->drawString(wfhandler.weatherMain, MARGIN_LFT, 100);
          }

          free(textBuffer);

          //   if (nullptr != weatherDescription) {
          //     canvas->setTextFont(0);
          //     canvas->setTextSize(1);
          //     canvas->setTextDatum(TL_DATUM);
          //     canvas->setTextWrap(false, false);
          //     canvas->setTextColor(TFT_BLACK);
          //     canvas->drawString(weatherDescription, MARGIN_LFT+2, 107);
          //     canvas->setTextColor(TFT_WHITE);
          //     canvas->drawString(weatherDescription, MARGIN_LFT, 105);
          //   }
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