
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <SPI.h>
#include <FS.h>
#include "SPIFFS.h"
#include "../../app/LogView.hpp"
#include "database.hpp"

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
       lSysLog("SQL ERROR: Can't open database: '%s'\n", sqlite3_errmsg(*db));
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
            sqliteDbProblem=true;
       }
   } else {
       lSysLog("SQL: Operation done successfully\n");
   }
   lSysLog("SQL: Time taken: %lu\n", micros()-start);
   return rc;
}

SemaphoreHandle_t SqlLogSemaphore = xSemaphoreCreateMutex();
void SqlLog(const char * logLine) {
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        if ( sqliteDbProblem ) {
            lSysLog("SQL: ERROR: database corrupt in last transaction, regenerating database...\n");
            StopDatabase();
            delay(100);
            lSysLog("SPIFFS: removing files...\n");
            SPIFFS.remove("/lwatch.db");
            SPIFFS.remove("/lwatch.db-journal");
            lSysLog("SPIFFS: done\n");
            delay(100);
            sqliteDbProblem=false;
            StartDatabase();
        }
        if ( nullptr == lIoTsystemDatabase ) { return; }
        const char fmtStr[]="INSERT INTO rawlog VALUES (NULL,CURRENT_TIMESTAMP,'%s');";
        char * query=(char*)ps_malloc(400);
        sprintf(query,fmtStr,logLine);
        int  rc = db_exec(lIoTsystemDatabase,query);
        if (rc != SQLITE_OK) {
            lSysLog("SQL: ERROR: Unable to log '%s'\n", logLine);
            free(query);
            xSemaphoreGive( SqlLogSemaphore ); // free
            return;
        }
        free(query);
        xSemaphoreGive( SqlLogSemaphore ); // free
    }
}

void StopDatabase() {
    if (nullptr != lIoTsystemDatabase ) {
        lSysLog("SQL: closing...\n");
        if ( false == sqliteDbProblem ) { SqlLog("end"); }
        sqlite3_close(lIoTsystemDatabase);
        lIoTsystemDatabase=nullptr;
        sqlite3_shutdown();
    }
}
void StartDatabase() {
    lSysLog("Sqlite3 opening...\n");
    int rc;
    sqlite3_initialize();
    if (db_open("/spiffs/lwatch.db", &lIoTsystemDatabase)) {
        lSysLog("ERROR: Datasource: Unable to open sqlite3 log!\n");
        lIoTsystemDatabase=nullptr;
        sqliteDbProblem=true;
        return;
    }
    rc = db_exec(lIoTsystemDatabase, "CREATE TABLE if not exists rawlog ( id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, message text NOT NULL);");
    if (rc != SQLITE_OK) {
        sqlite3_close(lIoTsystemDatabase);
        lIoTsystemDatabase=nullptr;
        sqliteDbProblem=true;
        return;
    }

    // destroy old entries and possible date errors (no ntpSync or RTC)
    rc = db_exec(lIoTsystemDatabase, "DELETE FROM rawlog WHERE timestamp < datetime('now', '-90 days');");
    if (rc != SQLITE_OK) {
        sqlite3_close(lIoTsystemDatabase);
        lIoTsystemDatabase=nullptr;
        sqliteDbProblem=true;
        return;
    }
    db_exec(lIoTsystemDatabase, "SELECT COUNT(*) FROM rawlog;");

    rc = db_exec(lIoTsystemDatabase, "SELECT * FROM rawlog ORDER BY id DESC LIMIT 5;");
    if (rc != SQLITE_OK) {
        sqlite3_close(lIoTsystemDatabase);
        lIoTsystemDatabase=nullptr;
        sqliteDbProblem=true;
        return;
    }

    SqlLog("begin");
}
