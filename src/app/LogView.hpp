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

#ifndef __LUNOKIOT__LOGVIEW_APP__
#define __LUNOKIOT__LOGVIEW_APP__

#include <esp_timer.h>
#include <esp_log.h>
//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include <LilyGoWatch.h>

#include "../UI/AppTemplate.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"

extern SemaphoreHandle_t lLogAsBlockSemaphore;

//const PROGMEM char * me = "lunokIoT";

void lRawLog(const char *fmt, ...); // don't use directly! best use lLog()
// main log tool
#define lLog(...) { if( xSemaphoreTake( lLogAsBlockSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  { lRawLog(__VA_ARGS__); xSemaphoreGive( lLogAsBlockSemaphore ); } }
// system log

// Info log
#ifdef LUNOKIOT_DEBUG
#define liLog(...) { if( xSemaphoreTake( lLogAsBlockSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  { lRawLog("[%lld][ *i ] ",esp_timer_get_time()); lRawLog(__VA_ARGS__); xSemaphoreGive( lLogAsBlockSemaphore ); } }
#else 
#define liLog(...)
#endif

// system log
#ifdef LUNOKIOT_DEBUG_SYSTEM
#define lSysLog(...) { if( xSemaphoreTake( lLogAsBlockSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  { lRawLog("[%lld][lunokIoT] ",esp_timer_get_time()); lRawLog(__VA_ARGS__); xSemaphoreGive( lLogAsBlockSemaphore ); } }
#else 
#define lSysLog(...)
#endif

// other log flavours
#ifdef LUNOKIOT_DEBUG_APPLICATION
#define lAppLog(...) { if( xSemaphoreTake( lLogAsBlockSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  { lRawLog("[%p]['%s'] ",this,this->AppName()); lRawLog(__VA_ARGS__); xSemaphoreGive( lLogAsBlockSemaphore ); } }
#else 
#define lAppLog(...)
#endif

#ifdef LUNOKIOT_DEBUG_EVENTS
#define lEvLog(...) { if( xSemaphoreTake( lLogAsBlockSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  { lRawLog("[%lld][event] ",esp_timer_get_time()); lRawLog(__VA_ARGS__); xSemaphoreGive( lLogAsBlockSemaphore ); } }
#else 
#define lEvLog(...)
#endif

#ifdef LUNOKIOT_DEBUG_NETWORK
#define lNetLog(...) { if( xSemaphoreTake( lLogAsBlockSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  { lRawLog("[%lld][net] ",esp_timer_get_time()); lRawLog(__VA_ARGS__); xSemaphoreGive( lLogAsBlockSemaphore ); } }
#else 
#define lNetLog(...)
#endif

#ifdef LUNOKIOT_DEBUG_UI
#define lUILog(...) { if( xSemaphoreTake( lLogAsBlockSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  { lRawLog("[UI] "); lRawLog(__VA_ARGS__); xSemaphoreGive( lLogAsBlockSemaphore ); } }
#else 
#define lUILog(...);
#endif

#ifdef LUNOKIOT_DEBUG_UI_DEEP
#define lUIDeepLog(...) { if( xSemaphoreTake( lLogAsBlockSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  { lRawLog("[UI+] "); lRawLog(__VA_ARGS__); xSemaphoreGive( lLogAsBlockSemaphore ); } }
#else 
#define lUIDeepLog(...)
#endif

extern void SqlLog(const char * logLine);

class LogViewApplication: public TemplateApplication {
    private:
        unsigned long nextRedraw=0;
        unsigned long nextDBQuery=0;
        bool lastTouch=false;
        int16_t offsetX=0;
        int16_t offsetY=0;
        void SendQuery();
        size_t totalSPIFFS=0;
        size_t usedSPIFFS=0;
        const unsigned long MsUntilRefresh=20*1000;
    public:
        const char *AppName() override { return "Log viewer"; };
        GraphWidget * powerLog = nullptr;
        GraphWidget * activityLog = nullptr;
        GraphWidget * appLog = nullptr;
        GraphWidget * dataLog = nullptr;

        //TFT_eSprite * ViewBuffer;
        static bool dirty;
        //static TFT_eSprite * LogTextBuffer;
        static const uint8_t TextHeight=9;
        static const uint8_t TextWidth=40;
        LogViewApplication();
        ~LogViewApplication();
        bool Tick() override;
};

#endif
