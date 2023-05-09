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
#include <SPI.h>
#include <FS.h>
#include <LittleFS.h>


#include <sqlite3.h>
#include <LilyGoWatch.h>

#include <esp_task_wdt.h>
#include "../../app/LogView.hpp"
#include "database.hpp"
#include "../../lunokIoT.hpp"
#include "../SystemEvents.hpp"
//#include "lunokIoT.hpp"
Database * systemDatabase=nullptr;
const char JournalFile[]="/lwatch.db-journal";
const char DBFile[]="/lwatch.db";
const char DBFileMask[]="/lwatch_%d-%d.db";
int8_t Database::openedDatabases=0;


// address text as PRIMARY KEY can be better :) @TODO
const char *BLEDeleteQuery=(const char *)"DELETE FROM bluetooth WHERE address = '%s';";
const char *BLEUpdateQuery=(const char *)"UPDATE OR IGNORE bluetooth SET timestamp = CURRENT_TIMESTAMP, locationGroup = %d WHERE address='%s';"; 
const char *BLEInsertQuery=(const char *)"INSERT OR IGNORE INTO bluetooth VALUES (CURRENT_TIMESTAMP,'%s',%g,%d);";
const char *BLEGetQuery=(const char *)"SELECT address,distance,locationGroup FROM bluetooth WHERE address='%s' ORDER BY address DESC LIMIT 1;";
const char *rawLogCleanUnusedQuery=(const char *)"DELETE FROM rawlog WHERE (timestamp <= datetime('now', '-8 days'));";
//const char *jsonLogCleanUnusedQuery=(const char *)"DELETE FROM jsonlog WHERE (timestamp <= datetime('now', '-8 days'));";
const char *notificationsLogCleanUnusedQuery=(const char *)"DELETE FROM notifications WHERE (timestamp <= datetime('now', '-8 days'));";
const char *BLECleanUnusedQuery=(const char *)"DELETE FROM bluetooth WHERE locationGroup=0 AND (timestamp <= datetime('now', '-8 days'));";
const char *queryDumpSessionLog=(const char *)"INSERT INTO rawlog SELECT timestamp,message FROM rawlogSession;";
//const char *queryCreateJSONLog=(const char *)"CREATE TABLE if not exists jsonLog (  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, origin text NOT NULL, message text NOT NULL);";
const char *DropSessionTable=(const char *)"DROP TABLE rawlogSession;";
const char *queryCreateSessionRAWLog=(const char *)"CREATE TABLE if not exists rawlogSession ( timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, message text NOT NULL);";


char *zErrMsg = nullptr;

SemaphoreHandle_t SqlSemaphore = xSemaphoreCreateMutex();

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
       lSysLog("SQL ERROR: Can't open database: E:%d '%s'\n",sqlite3_errcode(*db), sqlite3_errmsg(*db));
       return rc;
   } else {
       lSysLog("SQL: Opened database successfully\n");
   }
   return rc;
}

int db_exec(sqlite3 *db, const char *sql,sqlite3_callback resultCallback, void* payload) {
    //lLog("@DEBUG DATABASE EXEC DISABLED\n");
    //return SQLITE_OK;
    //lSysLog("EXEC SQL: '%s'\n",sql);
    if ( nullptr == resultCallback ) { resultCallback=SQLiteDumpSerialCallback; }
    esp_task_wdt_reset();
    unsigned long startT = millis();
    int rc = sqlite3_exec(db, sql, resultCallback, payload, &zErrMsg);
    if (SQLITE_OK != rc) {
        int extErrorCode = sqlite3_extended_errcode(db);
        lSysLog("SQL: ERROR: '%s' extended: %d (%d)\n", zErrMsg, extErrorCode, rc);
        lSysLog("SQL: ERROR QUERY: '%s'\n", sql);
        sqlite3_free(zErrMsg);
        /*
        if ( SQLITE_IOERR == rc ) {
            lSysLog("SQL: ERROR: I/O ERROR!\n");
            if (SQLITE_IOERR_WRITE == extErrorCode) {

            }

        }*/

//SQLITE_IOERR_WRITE
        /*
        if ( SQLITE_CORRUPT == rc ) {
            //lSysLog("SQL: ERROR: Data lost! (corruption)\n");
            //sqliteDbProblem=true;
        } else if ( SQLITE_NOTADB == rc ) {
            //sqliteDbProblem=true;
            
    } else {
        //lSysLog("SQL: Operation done successfully\n");
        */
    }
    lSysLog("SQL: Time taken: %lu ms\n", millis()-startT);
    return rc;
}

void Database::Lock() { xSemaphoreTake( lock, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS); }
void Database::UnLock() { xSemaphoreGive( lock ); }

unsigned int Database::Pending() {
    if ( NULL == queue ) { return 0; }
    //Lock();
    unsigned int val =  uxQueueMessagesWaiting(queue);
    //UnLock();
    return val;
}

void Database::_DatabaseWorkerTaskLoop() {
    taskRunning=true;
    taskEnded=false;
    esp_task_wdt_delete(NULL);
    if ( 0 == Database::openedDatabases ) {
        lSysLog("Database: Sqlite3 initializing...\n");
        int sqlbegin = sqlite3_initialize();
        if ( SQLITE_OK != sqlbegin ) {
            queue=NULL;
            taskEnded=true;
            taskRunning=false;
            return;
        }
        sqlite3_config(SQLITE_CONFIG_LOG, SQLerrorLogCallback, nullptr);
    }
    Database::openedDatabases++; // increment
    //create queue
    queue = xQueueCreate( uxQueueLength, sizeof( uxItemSize ) );


    if (db_open(filename, &databaseDescriptor)) {
        lSysLog("ERROR: Datasource: Unable to open sqlite3 '%s'!\n",filename);
        databaseDescriptor=nullptr;
        vQueueDelete(queue);
        queue=NULL;
        taskEnded=true;
        taskRunning=false;
        return;
    }
    lLog("Database: %p open\n");

    while(taskRunning) {
        esp_task_wdt_reset();
        SQLQueryData * myData=nullptr;
        BaseType_t res= xQueueReceive(queue, &myData, LUNOKIOT_EVENT_TIME_TICKS);
        if ( pdTRUE != res ) {
            TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
            xTaskDelayUntil( &nextCheck, (1 / portTICK_PERIOD_MS) );
            continue;
        }
        //Lock();
        // execute query
        int rc;
        // temporal disable watchdog for me :)
        esp_err_t susbcribed = esp_task_wdt_status(NULL);
        if ( ESP_OK == susbcribed) { esp_task_wdt_delete(NULL); }
        //lLog("@DEBUG DATABASE DISABLED, query ignored\n");
        rc = db_exec(databaseDescriptor, myData->query,myData->callback,myData->payload);
        if ( ESP_OK == susbcribed) { esp_task_wdt_add(NULL); }
        free(myData->query);
        free(myData);
        //UnLock();
        taskYIELD();
        //UBaseType_t highWater = uxTaskGetStackHighWaterMark(NULL);
        //lLog("Database: Watermark Stack: %u\n",highWater);
    }
    // close db
    sqlite3_close(databaseDescriptor);
    databaseDescriptor=nullptr;
    // destroy queue
    vQueueDelete(queue);
    queue=NULL;
    lLog("Database: %p close\n", this);
    Database::openedDatabases--;
    if ( 0 == Database::openedDatabases ) {
        lSysLog("Database: Sqlite3 stopping...\n");
        int sqlend = sqlite3_shutdown();
        if ( SQLITE_OK != sqlend ) {
            taskEnded=true;
            return;
        }
    }
    taskEnded=true;
}

Database::Database(const char *filename) {
    lLog("Database: '%s' on %p\n", filename, this);
    this->filename=filename;
    //register queue consumer on single core
    xTaskCreatePinnedToCore([](void *args) {
        Database * myDatabase=(Database *)args;
        myDatabase->_DatabaseWorkerTaskLoop(); // return when delete is called
        lLog("Database: %p Task dies here!\n",myDatabase);
        vTaskDelete(NULL); // kill myself

    }, "sql", LUNOKIOT_MID_STACK_SIZE, this, sqlWorkerPriority, &databaseQueueTask,DATABASECORE);

    lLog("Database: %p Waiting thread %p...\n",this,databaseQueueTask);
    while(false == taskRunning) {
        TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
        xTaskDelayUntil( &nextCheck, (150 / portTICK_PERIOD_MS) ); // wait a ittle bit
    }
    lLog("Database: %p Thread %p running\n",this,databaseQueueTask);
}

Database::~Database() {
    Commit();
    // kill thread
    lLog("Database: %p Killing thread %p...\n",this,databaseQueueTask);
    taskRunning=false;
    while(false == taskEnded) {
        TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
        xTaskDelayUntil( &nextCheck, (150 / portTICK_PERIOD_MS) ); // wait a ittle bit
    }
    lLog("Database: %p Thread %p dead\n",this,databaseQueueTask);
    databaseQueueTask=NULL;
}

void Database::SendSQL(const char * sqlQuery,sqlite3_callback callback, void *payload) {
    // add element to queue
    if ( NULL == queue ) {
        lLog("Database: %p ERROR: No queue valid for this database (see the logs)\n");
        return;
    }
    SQLQueryData * newQuery = (SQLQueryData *)ps_malloc(sizeof(SQLQueryData));
    char * queryCopy = (char*)ps_malloc(strlen(sqlQuery)+1);
    strcpy(queryCopy,sqlQuery);
    //sprintf(queryCopy,sqlQuery);
    newQuery->query=queryCopy;
    newQuery->callback=callback;
    newQuery->payload=payload;
    // temporal disable watchdog for me :)
    esp_err_t susbcribed = esp_task_wdt_status(NULL);
    if ( ESP_OK == susbcribed) { esp_task_wdt_delete(NULL); }
    xQueueSend(queue, &newQuery, portMAX_DELAY);
    if ( ESP_OK == susbcribed) { esp_task_wdt_add(NULL); }
    taskYIELD();
}



static int SQLiteCallback(void *data, int argc, char **argv, char **azColName) {
   int i;
   lSysLog("SQL: Data dump:\n");
   for (i = 0; i<argc; i++){
       lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
   }
   return 0;
}

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
    //if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) == pdTRUE )  {
        for (auto const& dev : BLEKnowDevices) {
            if ( dev->addr == shitBLE ) {
                dev->distance=atoi(distance);
                dev->locationGroup=atoi(locationGroup);
                //if ( -1 == dev->locationGroup ) { dev->locationGroup=BLEZoneLocations::UNKNOWN; }
                lNetLog("BLE: Updated device '%s' (zone: %d)\n",address,dev->locationGroup);
                //SqlUpdateBluetoothDevice(dev->addr.toString().c_str(),dev->distance, dev->locationGroup);
                if ( BLEZoneLocations::UNKNOWN != dev->locationGroup) {
                    if ( BLELocationZone != (BLEZoneLocations)dev->locationGroup ) {
                        char logMsg[30] = { 0 }; 
                        sprintf(logMsg,"BLELocation: %d",dev->locationGroup);
                        SqlLog(logMsg);
                    }
                    BLELocationZone = (BLEZoneLocations)dev->locationGroup;
                }
                dev->dbSync=false;
                //get out!
                //xSemaphoreGive( BLEKnowDevicesSemaphore );
                return 0;
            }
        }
    //    xSemaphoreGive( BLEKnowDevicesSemaphore );
    //}
    //lLog("BUILD NEW <------------------------------\n");
    lBLEDevice * newDev = new lBLEDevice();
    newDev->addr = shitBLE;
    newDev->firstSeen = millis();
    newDev->lastSeen = newDev->firstSeen;
    newDev->distance=atoi(distance);
    newDev->locationGroup=atoi(locationGroup);
    //if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) == pdTRUE )  {
        BLEKnowDevices.push_back(newDev);
    //    xSemaphoreGive( BLEKnowDevicesSemaphore );
    //}
    return 0;
}

void SqlRefreshBluetoothDevice(const char * mac) {
    size_t totalsz = strlen(BLEGetQuery)+strlen(mac)+10;
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

}


void SqlUpdateBluetoothDevice(const char * mac,double distance, int locationGroup) {
    size_t totalsz = strlen(BLEUpdateQuery)+strlen(mac)+20;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    sprintf(query,BLEUpdateQuery,locationGroup,mac);
    if ( nullptr != systemDatabase ) { systemDatabase->SendSQL(query); }
    free(query);
}

void SqlAddBluetoothDevice(const char * mac, double distance, int locationGroup) {
    if ( nullptr == systemDatabase ) { return; }
   size_t totalsz = strlen(BLEInsertQuery)+strlen(mac)+20;
   char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    sprintf(query,BLEInsertQuery,mac,distance,locationGroup);
    systemDatabase->SendSQL(query);
    free(query);
}
/*
void SqlJSONLog(const char * from, const char * logLine) {
    if ( nullptr == systemDatabase ) { return; }
    //const char fmtStr[]="INSERT INTO jsonLog VALUES (NULL,CURRENT_TIMESTAMP,'%s',unishox1c('%s'));";
    const char fmtStr[]="INSERT INTO jsonLog VALUES (CURRENT_TIMESTAMP,'%s','%s');";
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
}
*/
void SqlLog(const char * logLine) {
    if ( nullptr == systemDatabase ) { return; }
    const char fmtStr[]="INSERT INTO rawlogSession VALUES (CURRENT_TIMESTAMP,'%s');";
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
}

/*
void NotificatioLog(const char * notificationData) {
    if ( nullptr != systemDatabase ) { return; }
    lLog("DEBUUUUUG: '%s'\n",notificationData);
    const char fmtStr[]="INSERT INTO notifications VALUES (NULL,CURRENT_TIMESTAMP,'%s');";
    size_t totalsz = strlen(fmtStr)+strlen(notificationData)+1;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    //lSysLog("SQL: Query alloc size: %u\n", totalsz);
    sprintf(query,fmtStr,notificationData);
    systemDatabase->SendSQL(query);
    free(query);
}

void NotificationLogSQL(const char * sqlQuery) {
    //const char fmtStr[]="INSERT INTO notifications VALUES (NULL,CURRENT_TIMESTAMP,'%s');";
    //size_t totalsz = strlen(fmtStr)+strlen(sqlQuery)+1;
    size_t totalsz = strlen(sqlQuery)+1;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    //sprintf(query,fmtStr,sqlQuery);
    strcpy(query,sqlQuery);
    systemDatabase->SendSQL(query);
    free(query);
    return;
}
*/
void CleanupDatabase() {
    if ( nullptr != systemDatabase ) {
        // Create tables and clean SQL old devices
        systemDatabase->SendSQL(rawLogCleanUnusedQuery);
        //systemDatabase->SendSQL(jsonLogCleanUnusedQuery);
        systemDatabase->SendSQL(notificationsLogCleanUnusedQuery);
        systemDatabase->SendSQL(BLECleanUnusedQuery);
        systemDatabase->SendSQL(queryDumpSessionLog);
        systemDatabase->SendSQL(DropSessionTable);
        systemDatabase->SendSQL(queryCreateSessionRAWLog);
    }
    systemDatabase->Commit();
}

void StopDatabase() {
    delete systemDatabase;
    systemDatabase=nullptr;
    return;
}

void StartDatabase() {
    // rotate the files
    char * DBFileLast = (char*)ps_malloc(1024);
    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    int day = timeinfo->tm_mday;
    int month = timeinfo->tm_mon;
    sprintf(DBFileLast,DBFileMask,day,month);
    if (!LittleFS.exists(DBFileLast)) {
        bool reached = LittleFS.rename(DBFile,DBFileLast);
        if ( reached ) {
            if (LittleFS.exists(JournalFile)) {
                LittleFS.remove(JournalFile);
                lLog("Database: Destroy old journaling\n");
            }
            lLog("Database: Rotated to %d-%d\n",day,month);
        }

    }
    free(DBFileLast);

    // seems be on littleFS must create the file (SPIFFS not)
    if (!LittleFS.exists(DBFile)){
        File file = LittleFS.open(DBFile, FILE_WRITE);   //  /littlefs is automatically added to the front 
        file.close();
    }

    // We also need to create the journal file.
    if (!LittleFS.exists(JournalFile)) {
        File file = LittleFS.open(JournalFile, FILE_WRITE);   //  /littlefs is automatically added to the front 
        file.close();
    }
    systemDatabase = new Database("/littlefs/lwatch.db");
}

void Database::Commit() {
    unsigned int remainDBQueries = Pending();
    if ( remainDBQueries > 0 ) {
        lLog("Database: %p Commit begin\n",this,remainDBQueries);
        unsigned long nextWarn=millis()+1000;
        while(0 != remainDBQueries ) {
            if ( millis() > nextWarn ) {
                lLog("Database: %p Commit pending (%u tasks)\n",this,remainDBQueries);
                nextWarn=millis()+1000;
            }
            delay(100);
            remainDBQueries = Pending();
        }
        lLog("Database: %p Commit end\n",this,remainDBQueries);
    }
}