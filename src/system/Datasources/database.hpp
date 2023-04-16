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

#ifndef ___LUNOKIOT__SYSTEM__DATABASE___
#define ___LUNOKIOT__SYSTEM__DATABASE___

#include <sqlite3.h>
#include <freertos/queue.h>
#include <freertos/task.h>

typedef struct {
    char * query;
    sqlite3_callback callback;
    void * payload;
} SQLQueryData;

class Database {
    protected:
        TaskHandle_t databaseQueueTask = NULL;
        const UBaseType_t uxQueueLength=5;
        const UBaseType_t uxItemSize=sizeof(SQLQueryData);
        UBaseType_t sqlWorkerPriority = tskIDLE_PRIORITY;

    public:
        sqlite3 *databaseDescriptor;
        QueueHandle_t queue;
        static void _DatabaseWorkerTask(void *args);
        Database(const char *filename);
        ~Database();
        void SendSQL(const char * sqlQuery,sqlite3_callback callback=nullptr, void *payload=nullptr);
};
extern Database * systemDatabase;

void StartDatabase();
void StopDatabase();

//@TODO @DEPRECATED SHIT MUST BE PORTED TO Database class
void SqlUpdateBluetoothDevice(const char * mac,double distance=-1, int locationGroup=0);
void SqlJSONLog(const char * from, const char * logLine);
void SqlAddBluetoothDevice(const char * mac, double distance=-1, int locationGroup=0);
void SqlRefreshBluetoothDevice(const char * mac);
//void SqlCleanUnusedBluetoothDevices();

/*
void SqlLog(const char * logLine);
extern sqlite3 *lIoTsystemDatabase;
extern SemaphoreHandle_t SqlLogSemaphore;
//int db_exec(sqlite3 *db, const char *sql);
void NotificatioLog(const char * notificationData);
void SQLQuery(void *args,sqlite3_callback callback);
int db_exec(sqlite3 *db, const char *sql,sqlite3_callback resultCallback=nullptr);
*/
#endif
