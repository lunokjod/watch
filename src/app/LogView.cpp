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
#include <sqlite3.h>
#include <LilyGoWatch.h>
#include "../UI/UI.hpp"
#include "LogView.hpp"
#include "../system/Datasources/database.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"
#include "../static/img_back_32.xbm"
#include <esp_task_wdt.h>
#include <LittleFS.h>

SemaphoreHandle_t lLogAsBlockSemaphore =  xSemaphoreCreateMutex(); // used when push a log line
SemaphoreHandle_t lLogSemaphore =  xSemaphoreCreateMutex(); // used when push a chunk of log line
bool LogViewApplication::dirty=false;

void lRawLog(const char *fmt, ...) {
    //lLogCreate();
    va_list args;
    va_start(args, fmt);
    static char buf[1024];
    int resLen = vsnprintf(buf, 1024, fmt, args);

    #ifdef LUNOKIOT_DEBUG
        if( xSemaphoreTake( lLogSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  {
            //printf(buf);
            Serial.printf(buf);
            xSemaphoreGive( lLogSemaphore );
        }
    #endif

    va_end(args);
}


GraphWidget *powerWidget=nullptr;
GraphWidget *activityWidget=nullptr;
GraphWidget *appWidget=nullptr;
GraphWidget *dataWidget=nullptr;

// callback from sqlite3 used to build the activity bar
static int LogEventsParser(void *data, int argc, char **argv, char **azColName) {
    int i;
    const char appBanner[]="Application:";
    const char wifiOff[]="WiFi: disconnect";
    const char wifiOn[]="WiFi: connected";
    const char BLEScan[]="BLE: pasive scan";
    const char BLEOn[]="BLE: enabled";
    const char BLEOff[]="BLE: disabled";
    const char BLELocationFound[]="BLELocation:";
    const char TempReport[]="Temperatures:";
    const char HallReport[]="Hall:";
    const char AccelReport[]="Acceleromether:";
    const char BatteryReport[]="Battery:";
    const char UsageReport[]="Usage stats:";
    appWidget->markColor = TFT_BLACK;
    for (i = 0; i<argc; i++){
        //lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
        if ( 0 != strcmp(azColName[i],"message")) { continue; }        

        // feed activity bar
        if ( 0 == strcmp(argv[i],"Activity: Running")) { activityWidget->markColor = TFT_RED; }
        else if ( 0 == strcmp(argv[i],"Activity: Walking")) { activityWidget->markColor = TFT_YELLOW; }
        else if ( 0 == strcmp(argv[i],"Activity: None")) { activityWidget->markColor = TFT_GREEN; }

        // feed power bar
        else if ( 0 == strcmp(argv[i],"begin")) { powerWidget->markColor = TFT_YELLOW; }
        else if ( 0 == strcmp(argv[i],"end")) { powerWidget->markColor = TFT_BLACK; }
        else if ( 0 == strcmp(argv[i],"wake")) { powerWidget->markColor = TFT_GREEN; }
        else if ( 0 == strcmp(argv[i],"stop")) { powerWidget->markColor = TFT_DARKGREEN; }
        
        // feed app bar
        else if ( 0 == strncmp(argv[i],appBanner,strlen(appBanner))) { appWidget->markColor = TFT_CYAN; }
        else if ( 0 == strncmp(argv[i],TempReport,strlen(TempReport))) { appWidget->markColor = TFT_YELLOW; }
        else if ( 0 == strncmp(argv[i],HallReport,strlen(HallReport))) { appWidget->markColor = TFT_LIGHTGREY; }
        else if ( 0 == strncmp(argv[i],AccelReport,strlen(AccelReport))) { appWidget->markColor = TFT_RED; }
        else if ( 0 == strncmp(argv[i],BatteryReport,strlen(BatteryReport))) { appWidget->markColor = TFT_DARKGREEN; }
        else if ( 0 == strncmp(argv[i],UsageReport,strlen(UsageReport))) { appWidget->markColor = TFT_GREEN; }

        //else if ( 0 == strncmp(argv[i],appBanner,strlen(appBanner))) { appWidget->markColor = TFT_CYAN; }

        // feed network bar
        else if ( 0 == strncmp(argv[i],wifiOn,strlen(wifiOn))) { dataWidget->markColor = TFT_GREEN; }
        else if ( 0 == strncmp(argv[i],wifiOff,strlen(wifiOff))) { dataWidget->markColor = TFT_BLACK; }

        else if ( 0 == strncmp(argv[i],BLEScan,strlen(BLEScan))) { dataWidget->markColor = TFT_DARKGREY; }
        else if ( 0 == strncmp(argv[i],BLEOn,strlen(BLEOn))) { dataWidget->markColor = TFT_BLUE; }
        else if ( 0 == strncmp(argv[i],BLEOff,strlen(BLEOff))) { dataWidget->markColor = TFT_BLACK; }
        else if ( 0 == strncmp(argv[i],BLELocationFound,strlen(BLELocationFound))) { dataWidget->markColor = TFT_WHITE; }

        else { lLog("WARNING: unknown log message: '%s'\n", argv[i]); }
    }
    // step all graphs with event color
    powerWidget->PushValue(1);
    //esp_task_wdt_reset();
    activityWidget->PushValue(1);
    //esp_task_wdt_reset();
    appWidget->PushValue(1);
    //esp_task_wdt_reset();
    dataWidget->PushValue(1);
    //esp_task_wdt_reset();
    return 0;
}

LogViewApplication::~LogViewApplication() {
    delete powerLog;
    delete activityLog;
    delete appLog;
    delete dataLog;
}
extern SemaphoreHandle_t SqlLogSemaphore;

LogViewApplication::LogViewApplication() {
    powerLog = new GraphWidget(20,200,0,1,TFT_BLACK, Drawable::MASK_COLOR);
    powerLog->inverted=false;
    activityLog = new GraphWidget(20,200,0,1,TFT_GREEN, Drawable::MASK_COLOR);
    activityLog->inverted=false;
    appLog = new GraphWidget(20,200,0,1,TFT_BLACK, Drawable::MASK_COLOR);
    appLog->inverted=false;
    dataLog = new GraphWidget(20,200,0,1,TFT_BLACK, Drawable::MASK_COLOR);
    dataLog->inverted=false;

    powerWidget=powerLog;
    activityWidget=activityLog;
    appWidget=appLog;
    dataWidget=dataLog;
//                            "SELECT * FROM Table1 t1 INNER JOIN Table2 t2 ON t1.id = t2.id GROUP BY t1.data1;"
//    systemDatabase->SendSQL("SELECT message FROM rawlog ORDER BY timestamp DESC LIMIT 200;");//, LogEventsParser);
//    systemDatabase->SendSQL("SELECT message FROM rawlogSession ORDER BY timestamp LIMIT 200;",LogEventsParser);
    //systemDatabase->SendSQL("SELECT message FROM rawlogSession;",LogEventsParser);
    //systemDatabase->SendSQL("SELECT message FROM rawlogSession WHERE 1 ORDER BY 1 DESC LIMIT 200;",LogEventsParser);
    //systemDatabase->SendSQL("SELECT COUNT(1) FROM rawlogSession;");

    Tick();
}

void LogViewApplication::SendQuery() {
    const char sqlQuery[]="SELECT message FROM rawlogSession ORDER BY id DESC LIMIT %d;";
    char sqlQueryBuffer[strlen(sqlQuery)+10];
    sprintf(sqlQueryBuffer,sqlQuery,powerLog->canvas->width());    
    systemDatabase->SendSQL(sqlQueryBuffer,LogEventsParser,this);
}

bool LogViewApplication::Tick() {
    //UINextTimeout = millis()+UITimeout; // disable screen timeout on this app    
    btnBack->Interact(touched,touchX, touchY);

    if (millis() > nextDBQuery ) {
        // reset bar color
        powerLog->markColor = TFT_GREEN; // on by default
        activityLog->markColor = TFT_GREEN; // rest by default
        appLog->markColor = TFT_BLACK;
        dataLog->markColor=TFT_BLACK;
        SendQuery();
        totalSPIFFS = LittleFS.totalBytes();
        usedSPIFFS = LittleFS.usedBytes();        
        nextDBQuery=millis()+MsUntilRefresh; // next db refresh
    }

    if (millis() > nextRedraw ) {
        canvas->fillSprite(TFT_BLACK);
        //canvas->fillRect(0,0,TFT_WIDTH,TFT_HEIGHT-70,TFT_BLACK);
        //canvas->fillRect(0,TFT_HEIGHT-70,TFT_WIDTH,70,ThCol(background));

        char buffer[255];
        canvas->setTextFont(0);
        canvas->setTextSize(1);
        canvas->setTextDatum(TR_DATUM);
        unsigned long secsTillNextRefresh = (nextDBQuery-millis())/1000;
        sprintf(buffer,"refresh in %lu seconds",secsTillNextRefresh);
        canvas->setTextColor(TFT_WHITE,TFT_BLACK);
        canvas->drawString(buffer,canvas->width()-5,5);


        canvas->setTextFont(0);
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextSize(2);
        canvas->setTextDatum(BL_DATUM);


        int16_t x = 20;
        int16_t y = 28;
        int16_t border=4;
        canvas->drawString("Power",40, y-border);
        canvas->fillRect(x-border,y-border,powerLog->canvas->width()+(border*2),powerLog->canvas->height()+(border*2),ThCol(background_alt));
        powerLog->DrawTo(canvas,x,y);

        y+=20+28;
        canvas->drawString("Activity",40, y-border);
        canvas->fillRect(x-border,y-border,activityLog->canvas->width()+(border*2),activityLog->canvas->height()+(border*2),ThCol(background_alt));
        activityLog->DrawTo(canvas,x,y);

        y+=20+28;
        canvas->drawString("Apps",40, y-border);
        canvas->fillRect(x-border,y-border,appLog->canvas->width()+(border*2),appLog->canvas->height()+(border*2),ThCol(background_alt));
        appLog->DrawTo(canvas,x,y);

        y+=20+28;
        canvas->drawString("Network",40, y-border);
        canvas->fillRect(x-border,y-border,dataLog->canvas->width()+(border*2),dataLog->canvas->height()+(border*2),ThCol(background_alt));
        dataLog->DrawTo(canvas,x,y);

        sprintf(buffer,"Free: %u Kb",(totalSPIFFS-usedSPIFFS)/1024);
        canvas->setTextColor(TFT_WHITE,TFT_BLACK);
        canvas->drawString(buffer,60,canvas->height()-12);


        btnBack->DrawTo(canvas);

        nextRedraw=millis()+(1000/1);
        return true;
    }
    return false;
}
