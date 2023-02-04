
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <SPI.h>
#include <FS.h>
#include "SPIFFS.h"
#include "../../app/LogView.hpp"
#include "database.hpp"
#include "../SystemEvents.hpp"
const char JournalFile[]="/lwatch.db-journal";
const char DBFile[]="/lwatch.db";

char *zErrMsg = 0;
volatile bool sqliteDbProblem=false;


// https://www.sqlite.org/syntaxdiagrams.html

void ListSPIFFS() {
    lSysLog("SPIFFS contents: ")
    // list SPIFFS contents
    File root = SPIFFS.open("/");
    if (!root) {
        lLog("- failed to open directory\n");
        return;
    }
    if (!root.isDirectory()) {
        lLog("- not a directory\n");
        return;
    }
    lLog("\n")
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            lSysLog("<DIR> '%s'\n",file.name());
        } else {
            lSysLog("      '%s' (%u byte)\n",file.name(),file.size());
        }
        file = root.openNextFile();
    }
    lSysLog("SPIFFS end\n");
}

sqlite3 *lIoTsystemDatabase = nullptr;

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
   lSysLog("SQL: '%s'\n",sql);
   long start = micros();
   int rc = sqlite3_exec(db, sql, callback, nullptr, &zErrMsg);
   if (SQLITE_OK != rc) {
       lSysLog("SQL: ERROR: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
       if ( SQLITE_CORRUPT == rc ) {
            lSysLog("SQL: ERROR: Data lost! (corruption)\n");
            sqliteDbProblem=true;
       } else if ( SQLITE_IOERR == rc ) {
            lSysLog("SQL: ERROR: I/O ERROR!\n");
            //sqliteDbProblem=true; <==============================================================
       }
   } else {
       lSysLog("SQL: Operation done successfully\n");
   }
   lSysLog("SQL: Time taken: %lu\n", micros()-start);
   return rc;
}


SemaphoreHandle_t SqlLogSemaphore = xSemaphoreCreateMutex();
static void __internalSqlSend(void *args) {
    if ( nullptr == args ) {
        lSysLog("SQL: ERROR: unable to run empty query\n");
        vTaskDelete(NULL);
    }
    FreeSpace();
    char * query=(char*)args;
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        int  rc = db_exec(lIoTsystemDatabase,query);
        if (rc != SQLITE_OK) {
            lSysLog("SQL: ERROR: Unable exec: '%s'\n", query);
        }
        delay(50);
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

void SqlJSONLog(const char * from, const char * logLine) {

    if ( sqliteDbProblem ) {
        lSysLog("SQL: ERROR: database corrupt in last transaction, regenerating database...\n");
        StopDatabase();
        delay(100);
        lSysLog("SPIFFS: removing files...\n");
        SPIFFS.remove(DBFile);
        SPIFFS.remove(JournalFile);
        lSysLog("SPIFFS: done\n");
        delay(100);
        sqliteDbProblem=false;
        StartDatabase();
        delay(50);
    }
    if ( nullptr == lIoTsystemDatabase ) {
        lSysLog("SQL: No database\n");
        return;
    }
    const char fmtStr[]="INSERT INTO jsonLog VALUES (NULL,CURRENT_TIMESTAMP,'%s','%s');";
    char * query=(char*)ps_malloc(strlen(fmtStr)+strlen(from)+strlen(logLine)+200);
    sprintf(query,fmtStr,from,logLine);
    BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_APP_STACK_SIZE, query, uxTaskPriorityGet(NULL), NULL,0);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }
}

void SqlLog(const char * logLine) {
    if ( sqliteDbProblem ) {
        lSysLog("SQL: ERROR: database corrupt in last transaction, regenerating database...\n");
        StopDatabase();
        delay(100);
        lSysLog("SPIFFS: removing files...\n");
        SPIFFS.remove(DBFile);
        SPIFFS.remove(JournalFile);
        lSysLog("SPIFFS: done\n");
        delay(100);
        sqliteDbProblem=false;
        StartDatabase();
        delay(50);
    }
    if ( nullptr == lIoTsystemDatabase ) {
        return;
    }
    const char fmtStr[]="INSERT INTO rawlog VALUES (NULL,CURRENT_TIMESTAMP,'%s');";
    char * query=(char*)ps_malloc(strlen(fmtStr)+strlen(logLine)+200);
    sprintf(query,fmtStr,logLine);
    BaseType_t intTaskOk = xTaskCreatePinnedToCore(_intrnalSql, "", LUNOKIOT_APP_STACK_SIZE,  query, uxTaskPriorityGet(NULL), NULL,0);
    if ( pdPASS != intTaskOk ) { lSysLog("ERROR: cannot launch SQL task\n"); }

    /*
    int  rc = db_exec(lIoTsystemDatabase,query);
    if (rc != SQLITE_OK) {
        lSysLog("SQL: ERROR: Unable to log '%s'\n", logLine);
        free(query);
        xSemaphoreGive( SqlLogSemaphore ); // free
        return;
    }
    free(query);
    */
    delay(50);
}

void StopDatabase() {
    if (nullptr != lIoTsystemDatabase ) {
        lSysLog("SQL: closing...\n");
        if ( false == sqliteDbProblem ) { SqlLog("end"); }
        if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
            sqlite3_db_cacheflush(lIoTsystemDatabase);
            sqlite3_close(lIoTsystemDatabase);
            lIoTsystemDatabase=nullptr;
            sqlite3_shutdown();
            lSysLog("@TODO database backup here\n");
            xSemaphoreGive( SqlLogSemaphore ); // free
        }
    }
}
void StartDatabase() {
    lSysLog("Sqlite3 opening...\n");
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        if ( SPIFFS.exists(JournalFile) ) {
            lSysLog("SQL: WARNING: Jorunal file found! (cleanup)\n");
            SPIFFS.remove(JournalFile);
        }
        int rc;
        sqlite3_initialize();
        if (db_open("/spiffs/lwatch.db", &lIoTsystemDatabase)) {
            lSysLog("ERROR: Datasource: Unable to open sqlite3 log!\n");
            lIoTsystemDatabase=nullptr;
            sqliteDbProblem=true;
            xSemaphoreGive( SqlLogSemaphore ); // free
            return;
        }
        xSemaphoreGive( SqlLogSemaphore ); // free

        char *query=(char *)"CREATE TABLE if not exists rawlog ( id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, message text NOT NULL);";
        xTaskCreatePinnedToCore(_intrnalSqlStatic, "", LUNOKIOT_TASK_STACK_SIZE, (void*)query, uxTaskPriorityGet(NULL), NULL,0);
        query=(char *)"DELETE FROM rawlog WHERE timestamp < datetime('now', '-90 days');";
        xTaskCreatePinnedToCore(_intrnalSqlStatic, "", LUNOKIOT_TASK_STACK_SIZE, (void*)query, uxTaskPriorityGet(NULL), NULL,0);
        query=(char *)"CREATE TABLE if not exists jsonLog ( id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, origin text NOT NULL, message text NOT NULL);";
        xTaskCreatePinnedToCore(_intrnalSqlStatic, "", LUNOKIOT_TASK_STACK_SIZE, (void*)query, uxTaskPriorityGet(NULL), NULL,0);
        query=(char *)"DELETE FROM jsonLog WHERE timestamp < datetime('now', '-90 days');";
        xTaskCreatePinnedToCore(_intrnalSqlStatic, "", LUNOKIOT_TASK_STACK_SIZE, (void*)query, uxTaskPriorityGet(NULL), NULL,0);
    }



    /*
    db_exec(lIoTsystemDatabase, "SELECT COUNT(*) FROM rawlog;");

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
