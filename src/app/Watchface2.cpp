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
#include "LuIMainMenu.hpp"
#include "Steps.hpp"
#include "Battery.hpp"
#include "Settings.hpp"

#include "Stopwatch.hpp"
#include "resources.hpp"
#include "../system/Datasources/database.hpp"

extern float PMUBattDischarge;

extern bool weatherSyncDone;
extern int weatherId;
extern double weatherTemp;
extern char *weatherMain;
extern char *weatherDescription;
extern char *weatherIcon;
// for led display
int16_t Watchface2Application::bannerOffset=0;
CanvasWidget * Watchface2Application::StateDisplay = nullptr;

int currentDay;
int currentMonth;
int currentSec;
int currentMin;
int currentHour;

Watchface2Application::~Watchface2Application() {
    esp_event_handler_instance_unregister_with(uiEventloopHandle,UI_EVENTS,UI_EVENT_CONTINUE,ListenerUIContinue);
    esp_event_handler_instance_unregister_with(uiEventloopHandle,UI_EVENTS,UI_EVENT_REFRESH,ListenerUIRefresh);
    //taskRunning=false; // send "signal" to draw thread
    delete topLeftButton;
    delete topRightButton;
    delete bottomRightButton;
    delete bottomLeftButton;

    delete SphereBackground;
    delete SphereForeground;
    DestroyDoubleBuffer();
    //delete StateDisplay;
    delete SphereHands;
    free(textBuffer);
    textBuffer=nullptr;
    lAppLog("Watchface is gone\n");
}

void Watchface2Application::DrawDot(TFT_eSprite * view, int32_t x, int32_t y, int32_t r, uint16_t color, uint16_t capColor) {
    view->fillCircle(x, y, r+2, color);
    view->fillCircle(x, y, r, capColor);
    view->drawCircle(x, y, r+3, TFT_BLACK);

}

void Watchface2Application::RedrawHands(struct tm *timeinfo) {

    // this draws directly to canvas (small change)
    // seconds hand
    int secAngle = (timeinfo->tm_sec * 6);
    DescribeCircle(middleX, middleY, 110,
    [&, this](int x, int y, int cx, int cy, int angle, int step, IGNORE_PARAM) {
        if (secAngle == angle) {
            DescribeLine(cx,cy,x,y,[&, this](int x, int y, IGNORE_PARAM) {
                // obtain distance between two points
                int16_t vectorX=(x-cx);
                int16_t vectorY=(y-cy);
                double currentDistance = sqrt(pow(vectorX, 2) + pow(vectorY, 2) * 1.0);
                if ( currentDistance <= MaxSecondsLen) { return true; } // ignore
                canvas->fillCircle(x,y,SecondsTickness,ThCol(clock_hands_second));
                return true;
            });
            DrawDot(canvas,x,y,SecondsTickness,SecondsColor,SecondsBrightColor);
            return false;
        }
        return true;
    });

    // continue only if minute change or pass 15 seconds
    //static long int lastRefresh=0;
    int newMin = timeinfo->tm_min;
    //if ( (millis() < lastRefresh) || ( lastMin == newMin ) )  { return; }
    if ( lastMin == newMin )  { return; }
    lastMin=newMin;
    //lastRefresh=millis()+15000;

    // clean
    SphereHands->canvas->fillSprite(Drawable::MASK_COLOR);
    const int cCenterX=SphereHands->canvas->width()/2;
    const int cCenterY=SphereHands->canvas->height()/2;

    float hourAngle = ((timeinfo->tm_hour) % 12) * 30;
    float minuteAngle = (timeinfo->tm_min * 6);


    // hour hand
    float correctedHoureAngle = hourAngle + (minuteAngle / 12.0);
    // lAppLog("hourAngle: %f\n",correctedHoureAngle);
    DescribeCircle(cCenterX, cCenterY, 100, [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload) {
        if (int(correctedHoureAngle) == angle) {
            DescribeLine(cx,cy,x,y,[&, this](int x, int y, IGNORE_PARAM) {
                // obtain distance between two points
                int16_t vectorX=(x-cx);
                int16_t vectorY=(y-cy);
                double currentDistance = sqrt(pow(vectorX, 2) + pow(vectorY, 2) * 1.0);
                if ( currentDistance >= 60) {
                    DrawDot(SphereHands->canvas,x,y,HoursTickness,HoursColor,HoursBrightColor);
                    return false;
                } // stop the draw
                if ( currentDistance <= MinHourLen) {
                    SphereHands->canvas->fillCircle(x,y,HoursTickness/2,HoursColor);
                    return true;
                }
                if ( currentDistance >= MaxHourLen) { return true; } // dont draw more
                SphereHands->canvas->fillCircle(x,y,HoursTickness,HoursColor);
                return true;
            });
            return false;
        }
        return true;
    });

    // minutes hand
    float correctedMinuteAngle = minuteAngle + (secAngle / 60.0);
    DescribeCircle(cCenterX, cCenterY, 100, [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload) {
        if (int(correctedMinuteAngle) == angle) {
            DescribeLine(cx,cy,x,y,[&, this](int x, int y, void * obj) {
                // obtain distance between two points
                int16_t vectorX=(x-cx);
                int16_t vectorY=(y-cy);
                double currentDistance = sqrt(pow(vectorX, 2) + pow(vectorY, 2) * 1.0);

                if ( currentDistance >= 80-(MinutesTickness+3)) {
                    DrawDot(SphereHands->canvas,x,y,MinutesTickness,MinutesColor,MinutesBrightColor);
                    return false;
                }

                if ( currentDistance <= MinMinutesLen) {
                    SphereHands->canvas->fillCircle(x,y,MinutesTickness,MinutesColor);
                    return true;
                }
                if ( currentDistance >= MaxMinutesLen) { return true; }
                SphereHands->canvas->fillCircle(x,y,MinutesTickness/2,MinutesColor);
                return true;
            });
            return false;
        }
        return true;
    });
    SphereHands->canvas->drawCircle(cCenterX, cCenterY, 10, SecondsColor); // second cap
    SphereHands->canvas->drawCircle(cCenterX, cCenterY, 9, TFT_BLACK);
    SphereHands->canvas->fillCircle(cCenterX, cCenterY, 8, MinutesColor); // minute cap
    SphereHands->canvas->drawCircle(cCenterX, cCenterY, 4, TFT_BLACK);
    SphereHands->canvas->fillCircle(cCenterX, cCenterY, 3, HoursColor); // hour cap
    // SphereHands updated
}

void Watchface2Application::DestroyDoubleBuffer() {
    lAppLog("Double buffer clean\n");
    if ( nullptr != lastCanvas ) {
        TFT_eSprite * bkp = lastCanvas;
        lastCanvas=nullptr;
        bkp->deleteSprite();
        delete bkp;
    }
}

Watchface2Application::Watchface2Application() {
    // get center and size
    middleX = canvas->width() / 2;
    middleY = canvas->height() / 2;
    radius = (canvas->width() + canvas->height()) / 4;

    esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_CONTINUE, [](void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
        lLog("Watchface2: UI CONTINUE\n");
        Watchface2Application * self = (Watchface2Application*)handler_args;
        self->markForDestroy=true;
    }, this, &ListenerUIContinue);
    esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_REFRESH, [](void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
        lLog("Watchface2: UI REFRESH\n");
        Watchface2Application * self = (Watchface2Application*)handler_args;
        self->markForDestroy=true;
    }, this, &ListenerUIRefresh);

    // initialize corner buttons
    bottomLeftButton = new ActiveRect(0, 160, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new BatteryApplication()); });
    topLeftButton = new ActiveRect(0, 0, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new SettingsApplication()); });
    topRightButton = new ActiveRect(160, 0, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new StepsApplication()); });
    bottomRightButton = new ActiveRect(160, 160, 80, 80, [](IGNORE_PARAM) { LaunchApplication(new LuIMainMenuApplication()); });

    // generate proceduraly the backface (no bitmap wasting space on flash)
    SphereBackground = new CanvasWidget(canvas->height(),canvas->width());
    SphereBackground->canvas->fillSprite(TFT_BLACK);
    //
    SphereForeground = new CanvasWidget(canvas->height(), canvas->width());
    SphereForeground->canvas->fillSprite(TFT_PINK);
    //
    CanvasWidget * outherSphere = new CanvasWidget(canvas->height(),canvas->width());
    outherSphere->canvas->fillSprite(TFT_PINK);

    // led display
    if( xSemaphoreTake( StatusSemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS) == pdTRUE )  {
        if ( nullptr == StateDisplay ) {
            StateDisplay = new CanvasWidget(40, 90+DisplayFontWidth,1); // space for one letter
            xSemaphoreGive( StatusSemaphore );
            SetStatus("Hi! (: ");
        } else {
            xSemaphoreGive( StatusSemaphore );
        }
    }
    
    // Buttons
    outherSphere->canvas->fillRect(0, 0, middleX, middleY, tft->color24to16(0x000408));
    outherSphere->canvas->fillRect(middleX, 0, middleX, middleY, tft->color24to16(0x000408));
    outherSphere->canvas->fillRect(0, middleY, middleX, middleY, tft->color24to16(0x000408));
    outherSphere->canvas->fillRect(middleX, middleY, middleX, middleY, tft->color24to16(0x102048));
    outherSphere->canvas->fillCircle(middleX, middleY, radius + (margin * 2), TFT_PINK);

    // outher circle
    outherSphere->canvas->fillCircle(middleX, middleY, radius - margin, tft->color24to16(0x384058));
    outherSphere->canvas->fillCircle(middleX, middleY, radius - (margin * 2), TFT_PINK);

    DescribeCircle(middleX, middleY, radius - (margin * 2.5), [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload) {
        if (3 == (angle % 6)) { // dark bites
            outherSphere->canvas->fillCircle(x, y, 5, TFT_PINK);
        }
        return true;
    });
    DescribeCircle(middleX, middleY, radius - (margin * 2), [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload) {
        if (0 == (angle % 30)) { // blue 5mins bumps
            uint16_t color = tft->color24to16(0x384058);
            int heightLine=0;
            DescribeLine(x,y,cx,cy,[&,this](int x, int y, IGNORE_PARAM) {
                heightLine++;
                if (heightLine > 15 ) { return false; }
                outherSphere->canvas->drawPixel(x,y,color);
                return true;
            });
            outherSphere->canvas->fillCircle(x, y, 5, color);
        }
        return true;
    });
    outherSphere->canvas->fillRect(0, middleY - 40, 40, 80, TFT_PINK); // left cut

    outherSphere->DumpTo(SphereBackground->canvas, 0, 0,TFT_PINK);
    delete outherSphere;


    // inner quarter right+down decoration (maybe future data panel)
    CanvasWidget * innerSphere = new CanvasWidget(canvas->width() / 2, canvas->height() / 2);
    innerSphere->canvas->fillSprite(TFT_PINK);
    innerSphere->canvas->fillCircle(0, 0, 95, tft->color24to16(0x182858));
    innerSphere->canvas->fillCircle(0, 0, 45, TFT_PINK);
    innerSphere->DumpTo(SphereBackground->canvas, middleX, middleY, TFT_PINK);
    delete innerSphere;

    { // draw time 12/3/6/9 marks on cardinals
        SphereForeground->canvas->setFreeFont(NumberFreeFont);

        SphereForeground->canvas->setTextSize(NumberSize);

        SphereForeground->canvas->setTextDatum(TC_DATUM);
        SphereForeground->canvas->setTextColor(NumberColorBright);
        SphereForeground->canvas->drawString("12", (TFT_WIDTH / 2)-1, (NumberMargin)-1);
        SphereForeground->canvas->setTextColor(NumberColorShadow);
        SphereForeground->canvas->drawString("12", (TFT_WIDTH / 2)+1, (NumberMargin)+1);
        SphereForeground->canvas->setTextColor(NumberColor);
        SphereForeground->canvas->drawString("12", (TFT_WIDTH / 2), NumberMargin);

        SphereForeground->canvas->setTextDatum(CR_DATUM);
        SphereForeground->canvas->setTextColor(NumberColorBright);
        SphereForeground->canvas->drawString("3", (TFT_WIDTH - NumberMargin)-1, (TFT_HEIGHT / 2)-1);
        SphereForeground->canvas->setTextColor(NumberColorShadow);
        SphereForeground->canvas->drawString("3", (TFT_WIDTH - NumberMargin)+1, (TFT_HEIGHT / 2)+1);
        SphereForeground->canvas->setTextColor(NumberColor);
        SphereForeground->canvas->drawString("3", TFT_WIDTH - NumberMargin, TFT_HEIGHT / 2);

        SphereForeground->canvas->setTextDatum(BC_DATUM);
        SphereForeground->canvas->setTextColor(NumberColorBright);
        SphereForeground->canvas->drawString("6", (TFT_WIDTH / 2)-1, (TFT_HEIGHT - NumberMargin)-1);
        SphereForeground->canvas->setTextColor(NumberColorShadow);
        SphereForeground->canvas->drawString("6", (TFT_WIDTH / 2)+1, (TFT_HEIGHT - NumberMargin)+1);
        SphereForeground->canvas->setTextColor(NumberColor);
        SphereForeground->canvas->drawString("6", TFT_WIDTH / 2, TFT_HEIGHT - NumberMargin);

        SphereForeground->canvas->setTextDatum(CL_DATUM);
        SphereForeground->canvas->setTextColor(NumberColorBright);
        SphereForeground->canvas->drawString("9", NumberMargin-1, (TFT_HEIGHT / 2)-1);
        SphereForeground->canvas->setTextColor(NumberColorShadow);
        SphereForeground->canvas->drawString("9", NumberMargin+1, (TFT_HEIGHT / 2)+1);
        SphereForeground->canvas->setTextColor(NumberColor);
        SphereForeground->canvas->drawString("9", NumberMargin, TFT_HEIGHT / 2);
    }

    // time mark
    DescribeCircle(middleX, middleY, radius - 1, [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload) {
        int angleEnd = (markAngle + 90) % 360;
        if ((angle > markAngle) && (angle < angleEnd)) { SphereBackground->canvas->fillCircle(x, y, 1, tft->color24to16(0x5890e8)); }
        return true;
    });
    textBuffer = (char *)ps_malloc(255);
    // little bit more small
    SphereHands = new CanvasWidget(160,160);
    Tick();
    //Redraw();
}

void Watchface2Application::RedrawDisplay() {
    // led display
    const uint8_t lowPoint=50;
    const uint8_t highPoint=90;
    // horizontal (x coord)
    DescribeCircle2(middleX, middleY, 100, [&, this](int x, int y, int cx, int cy, int angle, int step, IGNORE_PARAM) {
        if ( angle <= 90 ) { return true; }
        if ( angle >= 180 ) { return true; }
        // vertical (y coord)
        DescribeLine(cx,cy,x,y,[&,this](int x, int y, IGNORE_PARAM) {
            int16_t vectorX=(x-cx);
            int16_t vectorY=(y-cy);
            // obtain distance between two points
            double currentDistance = sqrt(pow(vectorX, 2) + pow(vectorY, 2) * 1.0);
            if ( currentDistance < lowPoint ) { return true; } // ignore, too low
            if ( currentDistance > highPoint ) { return false; } // stop, too high
            if ( pdTRUE == xSemaphoreTake( StatusSemaphore, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS) ) {
                int32_t dx = (StateDisplay->canvas->width()-DisplayFontWidth)-(angle-90);
                int32_t dy = currentDistance-lowPoint;
                // check image bounds
                if ( dx < 0 ) {
                    xSemaphoreGive( StatusSemaphore );
                    return true;
                }
                if ( dx >= StateDisplay->canvas->width() ) {
                    xSemaphoreGive( StatusSemaphore );
                    return true;
                }
                if ( dy < 0 ) {
                    xSemaphoreGive( StatusSemaphore );
                    return true;
                }
                if ( dy >= StateDisplay->canvas->height() ) {
                    xSemaphoreGive( StatusSemaphore );
                    return true;
                }
                uint16_t newColor = StateDisplay->canvas->readPixel(dx,dy);
                if ( TFT_BLACK != newColor ) { newColor = DisplayColor; }
                canvas->drawPixel(x,y,newColor);
                xSemaphoreGive( StatusSemaphore );
            }
            return true;
        });
        return true;
    });
    // update display
    if ( pdTRUE == xSemaphoreTake( StatusSemaphore, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS) ) {
        if ( nullptr == whatToSay ) {
            xSemaphoreGive( StatusSemaphore );
            return;
        }
        if ( nullptr != systemDatabase ) {
            if ( 0 == systemDatabase->Pending() ) {
                //StateDisplay->DumpTo(canvas,0,0);
                StateDisplay->canvas->scroll(DisplaySpeed*-1,0);
                bannerSpace+=DisplaySpeed;
            }
        }
        if ( bannerSpace >= DisplayFontWidth  ) {
            StateDisplay->canvas->setTextColor(TFT_WHITE);
            StateDisplay->canvas->setTextFont(1);
            StateDisplay->canvas->setTextSize(4);
            StateDisplay->canvas->drawChar(whatToSay[bannerOffset],StateDisplay->canvas->width()-DisplayFontWidth,4);
            bannerOffset++;
            if (bannerOffset > strlen(whatToSay)) {
                // @TODO ugly
                free(whatToSay);
                whatToSay=nullptr;
                StateDisplay->canvas->fillSprite(TFT_BLACK);
                bannerOffset=0;
            }
            bannerSpace=0;
        }
        xSemaphoreGive( StatusSemaphore );
    }
}
void Watchface2Application::SetStatus(const char*what) {
    const char * frtmSring="%s          ";
    char * buffer=(char*)ps_malloc(strlen(frtmSring)+strlen(what)+1);
    sprintf(buffer,frtmSring,what);
    SetStatus(buffer);
}

void Watchface2Application::SetStatus(char*what) {
    xSemaphoreTake( StatusSemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    free(whatToSay);
    StateDisplay->canvas->fillSprite(TFT_BLACK);
    whatToSay=what;
    bannerOffset=0;
    xSemaphoreGive( StatusSemaphore );
}

void Watchface2Application::RedrawBLE() {
    // lAppLog("TIMEINFO: %02d:%02d:%02d\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    if ( BLELocationZone != BLEZoneLocations::UNKNOWN ) {
        sprintf(textBuffer,"%s", BLEZoneLocationsHumanReadable[BLELocationZone]);
        canvas->setTextDatum(CC_DATUM);
        canvas->setTextFont(1);
        canvas->setTextSize(2);
        canvas->setTextColor(ThCol(text));
        canvas->drawString(textBuffer,canvas->width()/2,(canvas->height()/2)+20);
    }
}

void Watchface2Application::Redraw() {
    unsigned long bFrame=millis();
    // push the background
    canvas->fillSprite(TFT_BLACK);
    SphereBackground->DumpTo(canvas, 0, 0,TFT_BLACK);
    RedrawDisplay();


    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    currentSec = timeinfo->tm_sec;
    currentMin = timeinfo->tm_min;
    currentHour = timeinfo->tm_hour;
    currentDay = timeinfo->tm_mday;
    currentMonth = timeinfo->tm_mon;

    RedrawBLE();
    
    if (weatherSyncDone) {
        if ( false == weatherStatus ) {
            SetStatus("Weather");
            weatherStatus=true;
        }

        // weather icon
        canvas->setBitmapColor(ThCol(mark), TFT_BLACK);
        // watchFaceCanvas->canvas->fillRect(120 - (img_weather_200.width/2),52,80,46,TFT_BLACK);
        if (-1 != weatherId) {
            if ((200 <= weatherId) && (300 > weatherId)) {
                canvas->pushImage(120 - (img_weather_200.width / 2), 52, img_weather_200.width, img_weather_200.height, (uint16_t *)img_weather_200.pixel_data);
            } else if ((300 <= weatherId) && (400 > weatherId)) {
                canvas->pushImage(120 - (img_weather_300.width / 2), 52, img_weather_300.width, img_weather_300.height, (uint16_t *)img_weather_300.pixel_data);
            } else if ((500 <= weatherId) && (600 > weatherId)) {
                canvas->pushImage(120 - (img_weather_500.width / 2), 52, img_weather_500.width, img_weather_500.height, (uint16_t *)img_weather_500.pixel_data);
            } else if ((600 <= weatherId) && (700 > weatherId)) {
                canvas->pushImage(120 - (img_weather_600.width / 2), 52, img_weather_600.width, img_weather_600.height, (uint16_t *)img_weather_600.pixel_data);
            } else if ((700 <= weatherId) && (800 > weatherId)) {
                //lAppLog("@TODO Watchface: openweather 700 condition code\n");
                // watchFaceCanvas->canvas->pushImage(120 - (img_weather_800.width/2) ,52,img_weather_800.width,img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
                canvas->pushImage(120 - (img_weather_800.width / 2), 52, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
                // watchFaceCanvas->canvas->pushImage(144,52,img_weather_600.width,img_weather_600.height, (uint16_t *)img_weather_600.pixel_data);
            } else if ((800 <= weatherId) && (900 > weatherId)) {
                canvas->pushImage(120 - (img_weather_800.width / 2), 52, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
            }
        }

        // temperature
        if (-1000 != weatherTemp) {
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
    } else {
        weatherStatus=false;
    }

    // connectivity notifications
    if (LoT().GetBLE()->IsEnabled()) {
        int16_t posX = 36;
        int16_t posY = 171;
        uint32_t dotColor = ThCol(background); // enabled but service isn't up yet
        if (LoT().GetBLE()->IsScanning()) {
            dotColor = ThCol(high);
        } else if (LoT().GetBLE()->IsAdvertising()) {
            dotColor = ThCol(medium);
        } else if (LoT().GetBLE()->Clients() > 0 ) {
            dotColor = ThCol(low);
        }
        canvas->fillCircle(posX, posY, DotSize, dotColor);
        unsigned char *img = (unsigned char *)img_bluetooth_16_bits; // bluetooth logo only icon
        if (LoT().GetBLE()->Clients() > 0 ) {
            img = (unsigned char *)img_bluetooth_peer_16_bits;
        } // bluetooth with peer icon
        canvas->drawXBitmap(posX + DotSize+5, posY - (img_bluetooth_16_height/2), img, img_bluetooth_16_width, img_bluetooth_16_height, ThCol(text));
    }
    if ( LoT().GetWiFi()->IsEnabled() ) {
        int16_t posX = 51;
        int16_t posY = 189;
        uint32_t dotColor = ThCol(background);
        if ( LoT().GetWiFi()->InUse() ) {
            if ( false == wifiStatus ) {
                SetStatus("WiFi");
                wifiStatus=true;
            }
            dotColor = ThCol(low);
        }
        canvas->fillCircle(posX, posY, DotSize, dotColor);
        canvas->drawXBitmap(posX + DotSize+5, posY - (img_wifi_16_height/2), img_wifi_16_bits, img_wifi_16_width, img_wifi_16_height, ThCol(text));
    } else {
        if ( wifiStatus ) {
            wifiStatus = false;
        }
    }
    // SQLite
    if ( nullptr != systemDatabase ) {
        unsigned int pend = systemDatabase->Pending();
        if ( pend > 0 ) {
            int16_t posX = 48;
            int16_t posY = 62;
            uint32_t dotColor = ThCol(background);
            if ( pend > 4 ) { dotColor = ThCol(high); }
            if ( pend > 2 ) { dotColor = ThCol(medium); }
            else if ( pend > 0 ) { dotColor = ThCol(low); }
            canvas->fillCircle(posX, posY, DotSize, dotColor);
            canvas->drawXBitmap(posX + DotSize+5, posY - (img_database_16_height/2), img_database_16_bits, img_database_16_width, img_database_16_height, ThCol(text));
            sprintf(textBuffer,"%d",pend);
            canvas->setTextFont(1);
            canvas->setTextSize(1);
            canvas->setTextDatum(CL_DATUM);
            canvas->setTextColor(ThCol(text));
            canvas->drawString(textBuffer,posX+DotSize+10+img_database_16_width,posY);
        }
    }
    // USB
    if (vbusPresent) {
        int16_t posX = 69;  // 60;
        int16_t posY = 203; // 190;
        uint32_t color = ThCol(background);
        if (ttgo->power->isChargeing()) { color = ThCol(low); }
        canvas->fillCircle(posX, posY, DotSize, color);
        canvas->drawXBitmap(posX + DotSize+5, posY - (img_usb_16_height/2), img_usb_16_bits, img_usb_16_width, img_usb_16_height, TFT_WHITE);
    }

    RedrawHands(timeinfo);
    const int16_t pushX = (canvas->width()-SphereHands->canvas->width())/2;
    const int16_t pushY = (canvas->height()-SphereHands->canvas->height())/2;
    SphereHands->DumpTo(canvas,pushX,pushY,Drawable::MASK_COLOR);
    /*
    for(int y=0;y<canvas->height();y++) {
        for(int x=0;x<canvas->width();x++) {
            uint16_t newColor = SphereHands->canvas->readPixel(x,y);
            if ( Drawable::MASK_COLOR != newColor ) {
                canvas->drawPixel(x,y,newColor);
            }
        }
    }*/

    // steps
    uint32_t activityColor = TFT_WHITE;
    bool doingSomething = false;
    int16_t posX = 40;
    int16_t posY = 74;
    if (vbusPresent) {
        activityColor = TFT_DARKGREY;
    }
    else if (nullptr != currentActivity) {
        if (0 == strcmp("BMA423_USER_WALKING", currentActivity)) {
            activityColor = TFT_GREEN;
            doingSomething = true;
        } else if (0 == strcmp("BMA423_USER_RUNNING", currentActivity)) {
            activityColor = TFT_YELLOW;
            doingSomething = true;
        }
        if (doingSomething) {
            canvas->fillCircle(posX - 10, posY + 6, DotSize, activityColor);
        }
    }
    // steps
    canvas->setTextColor(activityColor);
    canvas->setTextFont(0);
    canvas->setTextSize(2);
    canvas->setTextDatum(TL_DATUM);
    sprintf(textBuffer, "%d", stepCount);
    canvas->drawString(textBuffer, posX, posY);


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
        if (vbusPresent) {
            battColor = TFT_GREEN;
            battActivity = true;
        } else if (batteryPercent < 10) {
            battActivity = true;
            battColor = ThCol(high);
        } else if (batteryPercent < 35) { battColor = TFT_YELLOW; }
        // else if ( batteryPercent > 70 ) { battColor = TFT_GREEN; }
        canvas->setTextColor(battColor);
        int battFiltered = batteryPercent;
        if (battFiltered > 99) { battFiltered = 99; }
        sprintf(textBuffer, "%2d%%", battFiltered);
        canvas->setTextDatum(CL_DATUM);
        canvas->drawString(textBuffer, posX + 10, posY + 1);
        canvas->fillCircle(posX, posY, DotSize, battColor); // Alarm: red dot
    } /*else {
        uint32_t battColor = TFT_DARKGREY;
        posX = 26;
        posY = 149;
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextColor(battColor);
        canvas->setTextDatum(CL_DATUM);
        canvas->drawString("No batt", posX + 10, posY + 1);
        canvas->fillCircle(posX, posY, 5, ThCol(high)); // Alarm: red dot
    }*/
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

    // push over with alpha
    if ( ShowNumbers ) { SphereForeground->DumpAlphaTo(canvas,0,0,OverSphereAlpha,TFT_PINK); }



    // alway on top

    // time
    canvas->setTextFont(1);
    canvas->setTextSize(2);
    canvas->setTextWrap(false, false);
    sprintf(textBuffer, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    canvas->setTextColor(TFT_WHITE);
    canvas->setTextDatum(CR_DATUM);
    canvas->drawString(textBuffer, middleX - 20, middleY);

    // date
    sprintf(textBuffer, "%02d/%02d", timeinfo->tm_mday, timeinfo->tm_mon + 1);
    canvas->setTextDatum(BL_DATUM);
    canvas->setTextColor(TFT_WHITE);
    canvas->drawString(textBuffer, middleX + 35, middleY - 15);

    unsigned long eFrame=millis();
    //lAppLog("Frame time: %lu\n",eFrame-bFrame);
}

bool Watchface2Application::Tick() {
    // get the ActiveArea reactions
    topRightButton->Interact(touched, touchX, touchY); // steps
    bottomRightButton->Interact(touched, touchX, touchY); // menu
    topLeftButton->Interact(touched, touchX, touchY); // settings
    bottomLeftButton->Interact(touched, touchX, touchY); // battery

    // real image refresh to buffer
    if (millis() > nextRefresh) {
        Redraw();
        if ( ( nullptr == lastCanvas ) || ( markForDestroy )) {
            // clone the canvas
            lastCanvas = ScaleSprite(canvas,1.0);
            markForDestroy=false;
            nextRefresh = millis() + (1000 / CleanupFPS);
            return true; // full screen push (doublebuffer destroyed)
        }
        uint32_t drawPixels=0;
        for(int y=0;y<canvas->height();y++) {
            for(int x=0;x<canvas->width();x++) {
                uint16_t newColor = canvas->readPixel(x,y);
                uint16_t oldColor = lastCanvas->readPixel(x,y);
                if ( oldColor != newColor ) {
                    tft->drawPixel(x,y,newColor);
                    lastCanvas->drawPixel(x,y,newColor); // and write the doublebuffer here :)
                    drawPixels++;
                }
            }
            //if ( drawPixels > 5000 ) { break; }
        }
        nextRefresh = millis() + (1000 / DesiredFPS);
    }
    return false; // no UI refresh
}
