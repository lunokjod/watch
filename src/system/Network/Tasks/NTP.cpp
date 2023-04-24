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

#include "NTP.hpp"
#include "../../../app/LogView.hpp"
#include <ArduinoNvs.h>

bool ntpSyncDone=false;


const char * NTPWifiTask::ntpServer = "pool.ntp.org";
NTPWifiTask::NTPWifiTask(PCF8563_Class * rtc) : rtc(rtc) { }
NTPWifiTask::~NTPWifiTask() { }

void NTPWifiTask::Launch() {
    if (false == (bool)NVS.getInt("NTPEnabled")) {
        lNetLog("Task: %p '%s' User settings don't agree\n",this,Name());
        return;
    }
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        lNetLog("Task: %p '%s' Current ESP32 time: %02d:%02d:%02d %02d-%02d-%04d\n",this,Name(),timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon, 1900 + timeinfo.tm_year);
    }
    lNetLog("Task: %p '%s' Trying to sync with '%s'\n",this,Name(),ntpServer);
    // init and get the time
    int daylight = NVS.getInt("summerTime");
    long timezone = NVS.getInt("timezoneTime");
    lNetLog("Task: %p '%s' Summer time: '%s', GMT: %+d\n",this,Name(),(daylight?"yes":"no"),timezone);
    configTime(timezone * 3600, daylight * 3600, ntpServer);
    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +1 = 3600
    // GMT +8 = 28800
    // GMT -1 = -3600
    // GMT 0 = 0
    while (false == getLocalTime(&timeinfo, 100)) {
        // delay(100)
        TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
        xTaskDelayUntil( &nextCheck, (100 / portTICK_PERIOD_MS) ); // wait a ittle bit
    }
    lNetLog("Task: %p '%s' Synched with '%s'\n",this,Name(),ntpServer);
    if ( nullptr == rtc ) { return; }
    // user wants sync RTC also
    if (getLocalTime(&timeinfo))  {
        lNetLog("Task: %p '%s' Current ESP32 time: %02d:%02d:%02d %02d-%02d-%04d\n",this,Name(),timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon, 1900 + timeinfo.tm_year);
        //lEvLog("ESP32: Time: %02d:%02d:%02d %02d-%02d-%04d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1, 1900 + timeinfo.tm_year);
        rtc->syncToRtc();
        RTC_Date d = rtc->getDateTime();
        lNetLog("Task: %p '%s' RTC Sync '%s'\n",this,Name(),rtc->formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));
        return;
    }
    return;
}

bool NTPWifiTask::Check() {
    if ( nextShoot > millis() ) { return false; }
    nextShoot=millis()+Every24H;
    if (false == (bool)NVS.getInt("NTPEnabled")) { return false; }
    return true;
}
