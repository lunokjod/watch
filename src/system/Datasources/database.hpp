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

void StartDatabase();
void StopDatabase();
void SqlLog(const char * logLine);
extern sqlite3 *lIoTsystemDatabase;
extern SemaphoreHandle_t SqlLogSemaphore;
void SqlJSONLog(const char * from, const char * logLine);
//extern int db_exec(sqlite3 *db, const char *sql);
void NotificatioLog(const char * notificationData);

int db_exec(sqlite3 *db, const char *sql,sqlite3_callback resultCallback=nullptr);

#endif
