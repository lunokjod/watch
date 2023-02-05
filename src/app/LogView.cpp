#include <Arduino.h>
#include <sqlite3.h>
#include <LilyGoWatch.h>
#include "../UI/UI.hpp"
#include "LogView.hpp"
#include "../system/Datasources/database.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"
#include "../static/img_back_32.xbm"

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
            Serial.printf(buf);
            xSemaphoreGive( lLogSemaphore );
        } else {
            #ifdef LUNOKIOT_DEBUG_UI
                Serial.println("[UI] Timeout! last message wasn't included on visual log due draw timeout");
            #endif
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
    appWidget->markColor = TFT_BLACK;
    for (i = 0; i<argc; i++){
        //lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
        if ( 0 != strcmp(azColName[i],"message")) { continue; }        

        // feed activity bar
        if ( 0 == strcmp(argv[i],"Activity: Running")) { activityWidget->markColor = TFT_RED; }
        else if ( 0 == strcmp(argv[i],"Activity: Walking")) { activityWidget->markColor = TFT_YELLOW; }
        else if ( 0 == strcmp(argv[i],"Activity: None")) { activityWidget->markColor = TFT_GREEN; }
        // feed power bar
        else if ( 0 == strcmp(argv[i],"begin")) { powerWidget->markColor = TFT_GREEN; }
        else if ( 0 == strcmp(argv[i],"end")) { powerWidget->markColor = TFT_BLACK; }
        else if ( 0 == strcmp(argv[i],"wake")) { powerWidget->markColor = TFT_GREEN; }
        else if ( 0 == strcmp(argv[i],"stop")) { powerWidget->markColor = TFT_DARKGREEN; }
        
        // feed app bar
        else if ( 0 == strncmp(argv[i],appBanner,strlen(appBanner))) { appWidget->markColor = TFT_CYAN; }
    }
    powerWidget->PushValue(1);
    activityWidget->PushValue(1);
    appWidget->PushValue(1);
    return 0;
}



// callback from sqlite3 used to build the activity bar
static int LogEventsParserJSON(void *data, int argc, char **argv, char **azColName) {
    int i;
    const char appBanner[]="Data:";
    for (i = 0; i<argc; i++){
        if ( 0 != strcmp(azColName[i],"origin")) { continue; }

        if ( 0 == strcmp(argv[i],"check")) { activityWidget->markColor = TFT_BLACK; } // is check?
        else { dataWidget->markColor = TFT_YELLOW; } // event?
        dataWidget->PushValue(1);
        /*
        //lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));

        // feed activity bar
        if ( 0 == strcmp(argv[i],"Activity: Running")) { activityWidget->markColor = TFT_RED; }
        else if ( 0 == strcmp(argv[i],"Activity: Walking")) { activityWidget->markColor = TFT_YELLOW; }
        else if ( 0 == strcmp(argv[i],"Activity: None")) { activityWidget->markColor = TFT_GREEN; }
        // feed power bar
        else if ( 0 == strcmp(argv[i],"begin")) { powerWidget->markColor = TFT_GREEN; }
        else if ( 0 == strcmp(argv[i],"end")) { powerWidget->markColor = TFT_BLACK; }
        else if ( 0 == strcmp(argv[i],"wake")) { powerWidget->markColor = TFT_GREEN; }
        else if ( 0 == strcmp(argv[i],"stop")) { powerWidget->markColor = TFT_DARKGREEN; }
        
        // feed app bar
        else if ( 0 == strncmp(argv[i],appBanner,strlen(appBanner))) { appWidget->markColor = TFT_CYAN; }
        */
    }
    dataWidget->markColor = TFT_BLACK;
    dataWidget->PushValue(1);
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
    activityLog = new GraphWidget(20,200,0,1,TFT_GREEN, Drawable::MASK_COLOR);
    appLog = new GraphWidget(20,200,0,1,TFT_BLACK, Drawable::MASK_COLOR);
    dataLog = new GraphWidget(20,200,0,1,TFT_BLACK, Drawable::MASK_COLOR);

    powerWidget=powerLog;
    activityWidget=activityLog;
    appWidget=appLog;
    dataWidget=dataLog;

    char *zErrMsg;
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        //sqlite3_exec(lIoTsystemDatabase, "SELECT * FROM rawlog WHERE message LIKE 'Activity:%' ORDER BY id DESC LIMIT 200;", StepsAppInspectLogGraphGenerator, (void*)activityGraph, &zErrMsg);
        sqlite3_exec(lIoTsystemDatabase, "SELECT message FROM rawlog ORDER BY id DESC LIMIT 200;", LogEventsParser, nullptr, &zErrMsg);
        sqlite3_exec(lIoTsystemDatabase, "SELECT origin FROM jsonLog ORDER BY id DESC LIMIT 200;", LogEventsParserJSON, nullptr, &zErrMsg);
        xSemaphoreGive( SqlLogSemaphore );
    }
    Tick();
}

bool LogViewApplication::Tick() {
    UINextTimeout = millis()+UITimeout; // disable screen timeout on this app    
    btnBack->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillRect(0,0,TFT_WIDTH,TFT_HEIGHT-70,TFT_BLACK);
        //canvas->fillRect(0,TFT_HEIGHT-70,TFT_WIDTH,70,ThCol(background));

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
        canvas->fillRect(x-border,y-border,appLog->canvas->width()+(border*2),appLog->canvas->height()+(border*2),ThCol(background_alt));
        dataWidget->DrawTo(canvas,x,y);



        btnBack->DrawTo(canvas);

        nextRedraw=millis()+(1000/3);
        return true;
    }
    return false;
}
