#ifndef ___LUNOKIOT__SYSTEM__DATABASE___
#define ___LUNOKIOT__SYSTEM__DATABASE___
#include <sqlite3.h>

void StartDatabase();
void StopDatabase();
void ListSPIFFS();
void SqlLog(const char * logLine);
extern sqlite3 *lIoTsystemDatabase;

#endif