#ifndef ___LUNOKIOT__SYSTEM__DATABASE___
#define ___LUNOKIOT__SYSTEM__DATABASE___

#include <sqlite3.h>

void StartDatabase();
void StopDatabase();
void SqlLog(const char * logLine);
extern sqlite3 *lIoTsystemDatabase;
extern SemaphoreHandle_t SqlLogSemaphore;
void SqlJSONLog(const char * from, const char * logLine);
extern int db_exec(sqlite3 *db, const char *sql);

#endif
