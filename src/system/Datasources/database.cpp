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
#include <freertos/queue.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>


#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <LilyGoWatch.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>
#include <esp_task_wdt.h>
#include "../../app/LogView.hpp"
#include "database.hpp"
#include "../../lunokIoT.hpp"
#include "../SystemEvents.hpp"
//#include "lunokIoT.hpp"
#define DATABASE_CORE 1
Database * systemDatabase=nullptr;
const char JournalFile[]="/lwatch.db-journal";
const char DBFile[]="/lwatch.db";

//"CREATE TABLE if not exists bluetooth ( id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, address text NOT NULL, distance INT DEFAULT 0, locationGroup INT DEFAULT -1);";
const char *queryCreateRAWLog=(const char *)"CREATE TABLE if not exists rawlog ( id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, message text NOT NULL);";
const char *queryCreateJSONLog=(const char *)"CREATE TABLE if not exists jsonLog ( id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, origin text NOT NULL, message text NOT NULL);";
const char *queryCreateNotifications=(const char *)"CREATE TABLE if not exists notifications ( id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, data text NOT NULL);";

// address text as PRIMARY KEY can be better :) @TODO
const char *BLEDeleteQuery=(const char *)"DELETE FROM bluetooth WHERE address = '%s';";
const char *BLEUpdateQuery=(const char *)"UPDATE OR IGNORE bluetooth SET timestamp = CURRENT_TIMESTAMP, locationGroup = %d WHERE address='%s';"; 
const char *BLEInsertQuery=(const char *)"INSERT OR IGNORE INTO bluetooth VALUES (NULL,CURRENT_TIMESTAMP,'%s',%g,%d);";
const char *BLEGetQuery=(const char *)"SELECT address,distance,locationGroup FROM bluetooth WHERE address='%s' ORDER BY id LIMIT 1;";


//StaticTask_t SQLTaskHandler;
//StackType_t SQLStack[LUNOKIOT_TASK_STACK_SIZE];

char *zErrMsg = 0;
//volatile bool sqliteDbProblem=false;

//sqlite3 *lIoTsystemDatabase = nullptr;
SemaphoreHandle_t SqlLogSemaphore = xSemaphoreCreateMutex();

// https://www.sqlite.org/syntaxdiagrams.html

int db_exec(sqlite3 *db, const char *sql,sqlite3_callback resultCallback=nullptr, void* payload=nullptr);

// default callback if don't specify
static int SQLiteDumpSerialCallback(void *data, int argc, char **argv, char **azColName) {
   int i;
   lSysLog("SQL: Data dump:\n");
   for (i = 0; i<argc; i++){
       lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
   }
   return 0;
}
// default error hanlder
static void SQLerrorLogCallback(void *pArg, int iErrCode, const char *zMsg) {
    lSysLog("SQL: ERROR: (%d) %s\n", iErrCode, zMsg);
}

// sqlite "primitives"
int db_open(const char *filename, sqlite3 **db) {
   int rc = sqlite3_open(filename, db);
   if (rc) {
       lSysLog("SQL ERROR: Can't open database: %d '%s'\n", sqlite3_extended_errcode(*db), sqlite3_errmsg(*db));
       return rc;
   } else {
       lSysLog("SQL: Opened database successfully\n");
   }
   return rc;
}

int db_exec(sqlite3 *db, const char *sql,sqlite3_callback resultCallback, void* payload) {
    //lSysLog("SQL: '%s'\n",sql);
    if ( nullptr == resultCallback ) { resultCallback=SQLiteDumpSerialCallback; }
    esp_task_wdt_reset();
    unsigned long startT = millis();
    int rc = sqlite3_exec(db, sql, resultCallback, payload, &zErrMsg);
    if (SQLITE_OK != rc) {
        lSysLog("SQL: ERROR: '%s' (%d)\n", zErrMsg, rc);
        lSysLog("SQL: ERROR QUERY: '%s'\n", sql);
        sqlite3_free(zErrMsg);

        /*
        if ( SQLITE_CORRUPT == rc ) {
            //lSysLog("SQL: ERROR: Data lost! (corruption)\n");
            //sqliteDbProblem=true;
        } else if ( SQLITE_NOTADB == rc ) {
            //sqliteDbProblem=true;
        } else if ( SQLITE_IOERR == rc ) {
            //lSysLog("SQL: ERROR: I/O ERROR!\n");
            //sqliteDbProblem=true; //<==============================================================
        }*/
    } else {
        //lSysLog("SQL: Operation done successfully\n");
    }
    lSysLog("SQL: Time taken: %lu ms\n", millis()-startT);
    return rc;
}

// sql worker task (works in loop until killed)
void Database::_DatabaseWorkerTask(void *args) {
    Database * myDatabase=(Database *)args;
    while(true) {
        esp_task_wdt_reset();
        SQLQueryData * myData=nullptr;
        BaseType_t res= xQueueReceive(myDatabase->queue, &myData, portMAX_DELAY);
        if ( pdTRUE != res ) { continue; }
        if ( nullptr == myData ) { continue; }
        //lLog("Database: %p running %p '%s' and callback %p\n",myDatabase,myData,myData->query,myData->callback);
        // execute query
        int rc = db_exec(myDatabase->databaseDescriptor, myData->query,myData->callback,myData->payload);
        free(myData->query);
        free(myData);
        if (SQLITE_OK != rc) {
            //lLog("This cannot be goood.... @TODO @DEBUG <---------------\n");
        }

    }
    /*
        //lLog("Database: %p waiting orders...\n",myDatabase);
        //TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
        //xTaskDelayUntil( &nextCheck, (1000 / portTICK_PERIOD_MS) ); // wait a ittle bit (freeRTOS must breath)
        bool moreWorkToDo=true;
        while(moreWorkToDo) {
            SQLQueryData * myData=nullptr;
            esp_task_wdt_reset();
            BaseType_t res= xQueueReceive(myDatabase->queue, &myData, portMAX_DELAY);
            esp_task_wdt_reset();
            if ( pdTRUE != res ) { moreWorkToDo=false; continue; }
            if ( nullptr == myData ) { continue; }
            //lLog("Database: %p running %p '%s' and callback %p\n",myDatabase,myData,myData->query,myData->callback);
            // execute query
            int rc = db_exec(myDatabase->databaseDescriptor, myData->query,myData->callback);
            esp_task_wdt_reset();
            // wait a bit
            //TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
            //xTaskDelayUntil( &nextCheck, (150 / portTICK_PERIOD_MS) ); // wait a ittle bit (freeRTOS must breath)
            // destroy message
            free(myData->query);
            free(myData);
            if (SQLITE_OK != rc) {
                //lLog("This cannot be goood.... @TODO @DEBUG <---------------\n");
            }
        }
    }*/
    // rever return, the class killme with task descriptor
}

Database::Database(const char *filename) {
    lLog("Database: '%s' on %p\n", filename, this);
    sqlite3_config(SQLITE_CONFIG_LOG, SQLerrorLogCallback, nullptr);
    if (db_open(filename, &databaseDescriptor)) {
        lSysLog("ERROR: Datasource: Unable to open sqlite3!\n");
        databaseDescriptor=nullptr;
        xSemaphoreGive( SqlLogSemaphore ); // free
        return;
    }
    lLog("Database: %p open\n");
    //create queue
    queue = xQueueCreate( uxQueueLength, sizeof( uxItemSize ) );
    //register queue consumer on single core
    xTaskCreatePinnedToCore(Database::_DatabaseWorkerTask, "sql", LUNOKIOT_MID_STACK_SIZE, this, sqlWorkerPriority, &databaseQueueTask,DATABASE_CORE);
}

Database::~Database() {
    // destroy queue
    vQueueDelete(queue);
    queue=NULL;
    // kill thread
    vTaskDelete(databaseQueueTask);
    // close db
    sqlite3_close(databaseDescriptor);
    lLog("Database: %p close\n", this);


}

void Database::SendSQL(const char * sqlQuery,sqlite3_callback callback, void *payload) {
    // add element to queue
    if ( NULL == queue ) { return; }
    SQLQueryData * newQuery = (SQLQueryData *)ps_malloc(sizeof(SQLQueryData));
    char * queryCopy = (char*)ps_malloc(strlen(sqlQuery)+1);
    sprintf(queryCopy,sqlQuery);
    newQuery->query=queryCopy;
    newQuery->callback=callback;
    newQuery->payload=payload;
    xQueueSend(queue, &newQuery, portMAX_DELAY);
    //delay(150);
}



/*
static void __internalSqlSend(void *args,sqlite3_callback callback=nullptr);

void CheckDatabase() {
    if ( sqliteDbProblem ) {
        lSysLog("SQL: ERROR: database corrupt in last transaction, regenerating database...\n");
        StopDatabase();
        if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
            lSysLog("SPIFFS: removing database files...\n");
            SPIFFS.remove(DBFile);
            SPIFFS.remove(JournalFile);
            lSysLog("SPIFFS: done\n");
            xSemaphoreGive( SqlLogSemaphore ); // free
        }
        sqliteDbProblem=false;
        StartDatabase();
    }
}
*/
static int SQLiteCallback(void *data, int argc, char **argv, char **azColName) {
   int i;
   lSysLog("SQL: Data dump:\n");
   for (i = 0; i<argc; i++){
       lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
   }
   return 0;
}

/*
void __internalSqlSend(void *args,sqlite3_callback callback) {
    if ( nullptr == args ) {
        lSysLog("SQL: ERROR: unable to run empty query\n");
        return;
    }
    if ( false == LoT().IsSPIFFSEnabled() ) { 
        lSysLog("SQL: ERROR: unable to run query due SPIFFS is disabled!\n");
        return;
    }
    //FreeSpace();
    char * query=(char*)args;
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        int  rc = db_exec(lIoTsystemDatabase,query,callback);
        if (rc != SQLITE_OK) {
            //lSysLog("SQL: ERROR: Unable exec: '%s'\n", query);
        }
        //TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
        //xTaskDelayUntil( &nextCheck, (150 / portTICK_PERIOD_MS) ); // wait a ittle bit
        xSemaphoreGive( SqlLogSemaphore ); // free
    }
}

static void _intrnalSqlStatic(void *args) {
    __internalSqlSend(args);
    vTaskDelete(NULL);
}
static void _intrnalSql(void *args) {
    __internalSqlSend(args);
    free(args);
    vTaskDelete(NULL);
}
*/

static int BLESqlGetDeviceCallback(void *data, int argc, char **argv, char **azColName) {
    int i;
    char * address=nullptr;
    char * distance=nullptr;
    char * locationGroup=nullptr;
    //lSysLog("BLEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE SQL: Data dump:\n");
    for (i = 0; i<argc; i++){
        if ( 0 == strcmp(azColName[i],"address")) { address = argv[i]; }
        else if ( 0 == strcmp(azColName[i],"distance")) { distance = argv[i]; }
        else if ( 0 == strcmp(azColName[i],"locationGroup")) { locationGroup = argv[i]; }
        //lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
    }
    if ( nullptr == address ) { return 0; }

    NimBLEAddress shitBLE = NimBLEAddress(address);
    if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) == pdTRUE )  {
        for (auto const& dev : BLEKnowDevices) {
            if ( dev->addr == shitBLE ) {
                dev->distance=atoi(distance);
                dev->locationGroup=atoi(locationGroup);
                if ( -1 == dev->locationGroup ) { dev->locationGroup=BLEZoneLocations::UNKNOWN; }
                lNetLog("BLE: Updated device '%s' (zone: %d)\n",address,dev->locationGroup);
                //SqlUpdateBluetoothDevice(dev->addr.toString().c_str(),dev->distance, dev->locationGroup);
                //get out!
                xSemaphoreGive( BLEKnowDevicesSemaphore );
                return 0;
            }
        }
        xSemaphoreGive( BLEKnowDevicesSemaphore );
    }
    lLog("BUILD NEW <------------------------------\n");
    lBLEDevice * newDev = new lBLEDevice();
    newDev->addr = shitBLE;
    newDev->firstSeen = millis();
    newDev->lastSeen = newDev->firstSeen;
    newDev->distance=atoi(distance);
    newDev->locationGroup=atoi(locationGroup);
    if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) == pdTRUE )  {
        BLEKnowDevices.push_back(newDev);
        xSemaphoreGive( BLEKnowDevicesSemaphore );
    }
    return 0;
}

/*
void SqlCleanUnusedBluetoothDevices() {
    systemDatabase->SendSQL(BLECleanUnusedQuery);
}*/


void SqlRefreshBluetoothDevice(const char * mac) {
    size_t totalsz = strlen(BLEGetQuery)+50;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    sprintf(query,BLEGetQuery,mac);
    if ( nullptr != systemDatabase ) {
        systemDatabase->SendSQL(query,BLESqlGetDeviceCallback);
    }
    free(query);
    return;

    /*
    CheckDatabase();
    if ( nullptr == lIoTsystemDatabase ) {
        lSysLog("SQL: No database\n");
        return;
    }
    size_t totalsz = strlen(BLEGetQuery)+50;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    sprintf(query,BLEGetQuery,mac);
    //lLog("BLUETOOTH DATABASE GET DEBUG: %s\n",query);
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        db_exec(lIoTsystemDatabase,query,BLESqlGetDeviceCallback);
        xSemaphoreGive( SqlLogSemaphore ); // free
    }
    free(query);
    */
}


void SqlUpdateBluetoothDevice(const char * mac,double distance, int locationGroup) {

    /*
    CheckDatabase();
    if ( nullptr == lIoTsystemDatabase ) {
        lSysLog("SQL: No database\n");
        return;
    }*/
    size_t totalsz = strlen(BLEUpdateQuery)+64;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    sprintf(query,BLEUpdateQuery,locationGroup,mac);
    if ( nullptr != systemDatabase ) {
        systemDatabase->SendSQL(query);
    }
    /*
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        int  rc = db_exec(lIoTsystemDatabase,query);
        if (rc != SQLITE_OK) {
            //lSysLog("SQL: ERROR: Unable exec: '%s'\n", query);
        }
        //TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
        //xTaskDelayUntil( &nextCheck, (150 / portTICK_PERIOD_MS) ); // wait a ittle bit
        xSemaphoreGive( SqlLogSemaphore ); // free
    }*/
    free(query);
    //lSysLog("SQL: Query alloc size: %u\n", totalsz);
    //sprintf(query,BLEInsertQuery,mac,distance,locationGroup);
    //lLog("BLUETOOTH DATABASE UPDATE DEBUG: %s\n",query);
    //xTaskCreateStaticPinnedToCore( _intrnalSql,"",LUNOKIOT_TASK_STACK_SIZE,query,tskIDLE_PRIORITY,SQLStack,&SQLTaskHandler,DATABASE_CORE);
    //BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_TASK_STACK_SIZE, query, tskIDLE_PRIORITY-1, NULL,DATABASE_CORE);
    //if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
}

void SqlAddBluetoothDevice(const char * mac, double distance, int locationGroup) {
    if ( nullptr == systemDatabase ) { return; }
    /*
    size_t totalsz = strlen(BLEDeleteQuery)+strlen(mac)+10;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    sprintf(query,BLEDeleteQuery,mac);
    systemDatabase->SendSQL(query);
    free(query);
    */
   size_t totalsz = strlen(BLEInsertQuery)+strlen(mac)+20;
   char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    sprintf(query,BLEInsertQuery,mac,distance,locationGroup);
    systemDatabase->SendSQL(query);
    free(query);

    /*
    
    BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_TASK_STACK_SIZE, query, tskIDLE_PRIORITY-2, NULL,DATABASE_CORE);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
    */
    // https://github.com/siara-cc/esp32_arduino_sqlite3_lib#compression-with-unishox
    // use shox96_0_2c/shox96_0_2d for compression
    /*
    const char fmtStr[]="INSERT INTO bluetooth VALUES (NULL,CURRENT_TIMESTAMP,'%s',%g,%d);";
    totalsz = strlen(fmtStr)+strlen(mac)+10;
    */
    //sprintf(query,BLEUpdateQuery,locationGroup,mac);
    //lLog("BLUETOOTH DATABASE INSERT DEBUG: %s\n",query);
    //xTaskCreateStaticPinnedToCore( _intrnalSql,"",LUNOKIOT_TASK_STACK_SIZE,query,tskIDLE_PRIORITY,SQLStack,&SQLTaskHandler,DATABASE_CORE);
    //BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_TASK_STACK_SIZE, query, tskIDLE_PRIORITY-1, NULL,DATABASE_CORE);
    //if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
}

void SqlJSONLog(const char * from, const char * logLine) {
    if ( nullptr == systemDatabase ) { return; }
    const char fmtStr[]="INSERT INTO jsonLog VALUES (NULL,CURRENT_TIMESTAMP,'%s',unishox1c('%s'));";
    size_t totalsz = strlen(fmtStr)+strlen(from)+strlen(logLine)+1;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    //lSysLog("SQL: Query alloc size: %u\n", totalsz);
    sprintf(query,fmtStr,from,logLine);
    systemDatabase->SendSQL(query);
    free(query);
    /*
    CheckDatabase();
    if ( nullptr == lIoTsystemDatabase ) {
        lSysLog("SQL: No database\n");
        return;
    }
    // https://github.com/siara-cc/esp32_arduino_sqlite3_lib#compression-with-unishox
    // use shox96_0_2c/shox96_0_2d for compression
    //xTaskCreateStaticPinnedToCore( _intrnalSql,"",LUNOKIOT_TASK_STACK_SIZE,query,tskIDLE_PRIORITY,SQLStack,&SQLTaskHandler,DATABASE_CORE);
    BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_TASK_STACK_SIZE, query, tskIDLE_PRIORITY-2, NULL,DATABASE_CORE);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
    */
}

void SqlLog(const char * logLine) {
    if ( nullptr == systemDatabase ) { return; }
    const char fmtStr[]="INSERT INTO rawlog VALUES (NULL,CURRENT_TIMESTAMP,'%s');";
    size_t totalsz = strlen(fmtStr)+strlen(logLine)+1;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    //lSysLog("SQL: Query alloc size: %u\n", totalsz);
    sprintf(query,fmtStr,logLine);
    systemDatabase->SendSQL(query);
    free(query);
    return;
    /*
    CheckDatabase();
    if ( nullptr == lIoTsystemDatabase ) {
        return;
    }
    const char fmtStr[]="INSERT INTO rawlog VALUES (NULL,CURRENT_TIMESTAMP,'%s');";
    size_t totalsz = strlen(fmtStr)+strlen(logLine)+1;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    //lSysLog("SQL: Query alloc size: %u\n", totalsz);
    sprintf(query,fmtStr,logLine);
    BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_TASK_STACK_SIZE,  query, tskIDLE_PRIORITY-2, NULL,DATABASE_CORE);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
    //delay(50);
    */
}


void NotificatioLog(const char * notificationData) {
    lLog("@TODO NOTIFICATION LOG DISABLED\n");
    return;
    /*
    CheckDatabase();
    if ( nullptr == lIoTsystemDatabase ) {
        return;
    }
    const char fmtStr[]="INSERT INTO notifications VALUES (NULL,CURRENT_TIMESTAMP,'%s');";
    size_t totalsz = strlen(fmtStr)+strlen(notificationData)+1;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    //lSysLog("SQL: Query alloc size: %u\n", totalsz);
    sprintf(query,fmtStr,notificationData);
    BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_TASK_STACK_SIZE,  query, tskIDLE_PRIORITY-2, NULL,DATABASE_CORE);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
    //delay(50);
    */
}

void NotificationLogSQL(const char * sqlQuery) {
    return;
    /*
    CheckDatabase();
    if ( nullptr == lIoTsystemDatabase ) {
        return;
    }
    //const char fmtStr[]="INSERT INTO notifications VALUES (NULL,CURRENT_TIMESTAMP,'%s');";
    //strlen(fmtStr)+
    size_t totalsz = strlen(sqlQuery)+1;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    //lSysLog("SQL: Query alloc size: %u\n", totalsz);
    strcpy(query,sqlQuery);
    BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_TASK_STACK_SIZE,  query, tskIDLE_PRIORITY-2, NULL,DATABASE_CORE);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
    //delay(50);
    */
}

void StopDatabase() {
    delete systemDatabase;
    systemDatabase=nullptr;
    sqlite3_shutdown();
    return;
    /*
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) !=  pdTRUE )  { return; }
    if (nullptr == lIoTsystemDatabase ) { return; }
    lSysLog("SQL: closing...\n");
    if ( false == sqliteDbProblem ) { SqlLog("end"); }            
    //sqlite3_db_cacheflush(lIoTsystemDatabase);
    sqlite3_close(lIoTsystemDatabase);
    sqlite3_shutdown();
    lIoTsystemDatabase=nullptr;
    lSysLog("@TODO database backup here\n");
    xSemaphoreGive( SqlLogSemaphore ); // free
    */
}

void StartDatabase() {
    lSysLog("Sqlite3 opening...\n");
    int sqlbegin = sqlite3_initialize();
    if ( SQLITE_OK == sqlbegin ) {
        systemDatabase = new Database("/spiffs/lwatch.db");
        systemDatabase->SendSQL(queryCreateRAWLog);
        systemDatabase->SendSQL(queryCreateJSONLog);
        systemDatabase->SendSQL(queryCreateNotifications);
    }
    return;

    /*
    //lIoTsystemDatabase=nullptr;
    delay(100);
    query=(char *)"DELETE FROM rawlog WHERE timestamp < datetime('now', '-90 days');";
    xTaskCreatePinnedToCore(_intrnalSqlStatic, "", LUNOKIOT_TASK_STACK_SIZE, (void*)query, uxTaskPriorityGet(NULL)+2, NULL,0);
    query=(char *)"DELETE FROM jsonLog WHERE timestamp < datetime('now', '-90 days');";
    xTaskCreatePinnedToCore(_intrnalSqlStatic, "", LUNOKIOT_TASK_STACK_SIZE, (void*)query, uxTaskPriorityGet(NULL)+2, NULL,0);
    query=(char *)"DELETE FROM bluetooth WHERE timestamp < datetime('now', '-90 days');";
    xTaskCreatePinnedToCore(_intrnalSqlStatic, "", LUNOKIOT_TASK_STACK_SIZE, (void*)query, uxTaskPriorityGet(NULL)+2, NULL,0);
    */
}
