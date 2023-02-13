#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <Arduino.h>
#include <LilyGoWatch.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>

#include "../../app/LogView.hpp"
#include "database.hpp"
#include "../../lunokIoT.hpp"
#include "../SystemEvents.hpp"
//#include "lunokIoT.hpp"

const char JournalFile[]="/lwatch.db-journal";
const char DBFile[]="/lwatch.db";

//StaticTask_t SQLTaskHandler;
//StackType_t SQLStack[LUNOKIOT_TASK_STACK_SIZE];

char *zErrMsg = 0;
volatile bool sqliteDbProblem=false;

sqlite3 *lIoTsystemDatabase = nullptr;
SemaphoreHandle_t SqlLogSemaphore = xSemaphoreCreateMutex();

// https://www.sqlite.org/syntaxdiagrams.html

static void SQLerrorLogCallback(void *pArg, int iErrCode, const char *zMsg){
    lSysLog("SQL: (%d) %s\n", iErrCode, zMsg);
}

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


static int callback(void *data, int argc, char **argv, char **azColName) {
   int i;
   lSysLog("SQL: Data dump:\n");
   for (i = 0; i<argc; i++){
       lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
   }
   return 0;
}
// @TODO if SD is availiable, must use it to save read/write operations (internal flash cannot be replaced)
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


int db_exec(sqlite3 *db, const char *sql) {
   //lSysLog("SQL: '%s'\n",sql);
   unsigned long startT = micros();
   int rc = sqlite3_exec(db, sql, callback, nullptr, &zErrMsg);
   if (SQLITE_OK != rc) {
       lSysLog("SQL: ERROR: %s\n", zErrMsg);
       lSysLog("SQL: ERROR QUERY: '%s'\n", sql);
       sqlite3_free(zErrMsg);
       if ( SQLITE_CORRUPT == rc ) {
            //lSysLog("SQL: ERROR: Data lost! (corruption)\n");
            sqliteDbProblem=true;
       } else if ( SQLITE_IOERR == rc ) {
            //lSysLog("SQL: ERROR: I/O ERROR!\n");
            //sqliteDbProblem=true; //<==============================================================
       }
   } else {
       //lSysLog("SQL: Operation done successfully\n");
   }
   lSysLog("SQL: Time taken: %lu\n", micros()-startT);
   return rc;
}

static void __internalSqlSend(void *args) {
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
        int  rc = db_exec(lIoTsystemDatabase,query);
        if (rc != SQLITE_OK) {
            lSysLog("SQL: ERROR: Unable exec: '%s'\n", query);
        }/*
        delay(50);*/
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

void SqlAddBluetoothDevice(const char * mac, double distance) {
    CheckDatabase();
    if ( nullptr == lIoTsystemDatabase ) {
        lSysLog("SQL: No database\n");
        return;
    }
    const char fmtStrDelete[]="DELETE FROM bluetooth WHERE address = '%s';";
    size_t totalsz = strlen(fmtStrDelete)+strlen(mac)+10;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    //lSysLog("SQL: Query alloc size: %u\n", totalsz);
    sprintf(query,fmtStrDelete,mac);

    
    BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_TASK_STACK_SIZE, query, uxTaskPriorityGet(NULL), NULL,1);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
    
    //xTaskCreateStaticPinnedToCore( _intrnalSql,"",LUNOKIOT_TASK_STACK_SIZE,query,tskIDLE_PRIORITY,SQLStack,&SQLTaskHandler,1);
    //delay(100);

    // https://github.com/siara-cc/esp32_arduino_sqlite3_lib#compression-with-unishox
    // use shox96_0_2c/shox96_0_2d for compression
    const char fmtStr[]="INSERT INTO bluetooth VALUES (NULL,CURRENT_TIMESTAMP,'%s',%g);";
    totalsz = strlen(fmtStr)+strlen(mac)+10;
    query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    //lSysLog("SQL: Query alloc size: %u\n", totalsz);
    sprintf(query,fmtStr,mac,distance);
    //xTaskCreateStaticPinnedToCore( _intrnalSql,"",LUNOKIOT_TASK_STACK_SIZE,query,tskIDLE_PRIORITY,SQLStack,&SQLTaskHandler,1);
    intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_TASK_STACK_SIZE, query, uxTaskPriorityGet(NULL), NULL,1);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
}

void SqlJSONLog(const char * from, const char * logLine) {
    CheckDatabase();
    if ( nullptr == lIoTsystemDatabase ) {
        lSysLog("SQL: No database\n");
        return;
    }
    // https://github.com/siara-cc/esp32_arduino_sqlite3_lib#compression-with-unishox
    // use shox96_0_2c/shox96_0_2d for compression
    const char fmtStr[]="INSERT INTO jsonLog VALUES (NULL,CURRENT_TIMESTAMP,'%s',unishox1c('%s'));";
    size_t totalsz = strlen(fmtStr)+strlen(from)+strlen(logLine)+1;
    char * query=(char*)ps_malloc(totalsz);
    if ( nullptr == query ) {
        lSysLog("SQL: Unable to allocate: %u bytes\n", totalsz);
        return;
    }
    //lSysLog("SQL: Query alloc size: %u\n", totalsz);
    sprintf(query,fmtStr,from,logLine);
    //xTaskCreateStaticPinnedToCore( _intrnalSql,"",LUNOKIOT_TASK_STACK_SIZE,query,tskIDLE_PRIORITY,SQLStack,&SQLTaskHandler,1);
    BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_TASK_STACK_SIZE, query, uxTaskPriorityGet(NULL), NULL,1);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
}

void SqlLog(const char * logLine) {
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
    BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_TASK_STACK_SIZE,  query, uxTaskPriorityGet(NULL), NULL,1);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
    //delay(50);
}

void StopDatabase() {
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
}

const char *queryCreate0=(const char *)"CREATE TABLE if not exists rawlog ( id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, message text NOT NULL);";
const char *queryCreate1=(char *)"CREATE TABLE if not exists jsonLog ( id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, origin text NOT NULL, message text NOT NULL);";
const char *queryCreate2=(char *)"CREATE TABLE if not exists bluetooth ( id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, address text NOT NULL, distance INT DEFAULT 0);";

void StartDatabase() {
    lSysLog("Sqlite3 opening...\n");
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        //int rc;
        int sqlbegin = sqlite3_initialize();
        if ( SQLITE_OK == sqlbegin ) {
            sqlite3_config(SQLITE_CONFIG_LOG, SQLerrorLogCallback, nullptr);
            if (db_open("/spiffs/lwatch.db", &lIoTsystemDatabase)) {
                lSysLog("ERROR: Datasource: Unable to open sqlite3!\n");
                lIoTsystemDatabase=nullptr;
                sqliteDbProblem=true;
                xSemaphoreGive( SqlLogSemaphore ); // free
                return;
            }
        }
        xSemaphoreGive( SqlLogSemaphore ); // free

        //__internalSqlSend((void*)queryCreate0);
        //xTaskCreateStaticPinnedToCore( _intrnalSqlStatic,"",LUNOKIOT_TASK_STACK_SIZE,(void*)query,tskIDLE_PRIORITY,SQLStack,&SQLTaskHandler,1);
        delay(100);
        xTaskCreatePinnedToCore(_intrnalSqlStatic, "queryCreate0", LUNOKIOT_TASK_STACK_SIZE, (void*)queryCreate0, uxTaskPriorityGet(NULL), NULL,1);
        //__internalSqlSend((void*)queryCreate1);
        //xTaskCreateStaticPinnedToCore( _intrnalSqlStatic,"",LUNOKIOT_TASK_STACK_SIZE,(void*)query2,tskIDLE_PRIORITY,SQLStack,&SQLTaskHandler,1);
        delay(100);
        xTaskCreatePinnedToCore(_intrnalSqlStatic, "queryCreate1", LUNOKIOT_TASK_STACK_SIZE, (void*)queryCreate1, uxTaskPriorityGet(NULL), NULL,1);
        //__internalSqlSend((void*)queryCreate2);
        //xTaskCreateStaticPinnedToCore( _intrnalSqlStatic,"",LUNOKIOT_TASK_STACK_SIZE,(void*)query3,tskIDLE_PRIORITY,SQLStack,&SQLTaskHandler,1);
        delay(100);
        xTaskCreatePinnedToCore(_intrnalSqlStatic, "queryCreate2", LUNOKIOT_TASK_STACK_SIZE, (void*)queryCreate2, uxTaskPriorityGet(NULL), NULL,1);
        //FreeSpace();
        /*
        delay(100);
        query=(char *)"DELETE FROM rawlog WHERE timestamp < datetime('now', '-90 days');";
        xTaskCreatePinnedToCore(_intrnalSqlStatic, "", LUNOKIOT_TASK_STACK_SIZE, (void*)query, uxTaskPriorityGet(NULL)+2, NULL,0);
        query=(char *)"DELETE FROM jsonLog WHERE timestamp < datetime('now', '-90 days');";
        xTaskCreatePinnedToCore(_intrnalSqlStatic, "", LUNOKIOT_TASK_STACK_SIZE, (void*)query, uxTaskPriorityGet(NULL)+2, NULL,0);
        query=(char *)"DELETE FROM bluetooth WHERE timestamp < datetime('now', '-90 days');";
        xTaskCreatePinnedToCore(_intrnalSqlStatic, "", LUNOKIOT_TASK_STACK_SIZE, (void*)query, uxTaskPriorityGet(NULL)+2, NULL,0);
        */
    }
    //db_exec(lIoTsystemDatabase, "SELECT COUNT(*) FROM bluetooth;");



    /*
    db_exec(lIoTsystemDatabase, "SELECT COUNT(*) FROM rawlog;");
    db_exec(lIoTsystemDatabase, "SELECT COUNT(*) FROM jsonLog;");

    rc = db_exec(lIoTsystemDatabase, "SELECT * FROM rawlog ORDER BY id DESC LIMIT 5;");
    if (rc != SQLITE_OK) {
        sqlite3_close(lIoTsystemDatabase);
        lIoTsystemDatabase=nullptr;
        sqliteDbProblem=true;
        return;
    }
    rc = db_exec(lIoTsystemDatabase, "SELECT * FROM jsonLog ORDER BY id DESC LIMIT 5;");
    if (rc != SQLITE_OK) {
        sqlite3_close(lIoTsystemDatabase);
        lIoTsystemDatabase=nullptr;
        sqliteDbProblem=true;
        return;
    }*/


    SqlLog("begin");
}
