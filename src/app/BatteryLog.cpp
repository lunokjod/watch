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
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "BatteryLog.hpp"
#include "../system/Datasources/database.hpp"

BatteryLogApplication::BatteryLogApplication() {
    batteryGraph = new GraphWidget(100,220,0,100);
    
    //char *zErrMsg;
    //if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
    //    sqlite3_exec(lIoTsystemDatabase, "SELECT message FROM rawlog ORDER BY id DESC LIMIT 220;", [](void *data, int argc, char **argv, char **azColName) {
    systemDatabase->SendSQL("SELECT message FROM rawlog,rawlogSession ORDER BY timestamp DESC LIMIT 220;", [](void *data, int argc, char **argv, char **azColName) {
        GraphWidget *batteryGraph = (GraphWidget*)data;
        int i;
        const char BatteryLine[]="Battery:";
        //const char BatteryUSBNO[]=" USB: no";
        const char BatteryUSBYES[]=" USB: yes";
        for (i = 0; i<argc; i++){
            if ( 0 == strncmp(argv[i],BatteryLine,strlen(BatteryLine))) {
                char* token;
                const char Separator[] = ",";
                char *ptrShitOriginal=strdup((const char *)argv[i]);
                int counter=0;
                char * ptrShit=ptrShitOriginal+strlen(BatteryLine)+1;
                int battLevel =-1;
                batteryGraph->markColor = TFT_YELLOW;
                for (token = strtok(ptrShit, Separator); token; token = strtok(NULL, Separator)) {
                    //lLog("TOKEN: '%s'\n", token);
                    if ( 0 == counter) {
                        battLevel=atoi(token);
                    } else if ( 1 == counter) {
                        if ( 0 == strcmp(token,BatteryUSBYES)) {
                            batteryGraph->markColor = TFT_GREEN;
                        }
                    }
                    counter++;
                }
                free(ptrShitOriginal);
                batteryGraph->PushValue(battLevel);
                //lLog("VALUE: '%s'\n", argv[i]);
            } else {
                //lSysLog("   SQL: '%s' = '%s'\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
                batteryGraph->markColor = TFT_BLACK;
                batteryGraph->PushValue(0);
            }
        }
        return 0;
    }, (void*)batteryGraph);
        //sqlite3_exec(lIoTsystemDatabase, "SELECT jsonLog.origin FROM rawlog, jsonLog WHERE rawlog.timestamp!=jsonLog.timestamp ORDER BY id DESC LIMIT 200", LogEventsParserJSON, nullptr, &zErrMsg);
        //sqlite3_exec(lIoTsystemDatabase, "SELECT origin FROM jsonLog ORDER BY id DESC LIMIT 200;", LogEventsParserJSON, nullptr, &zErrMsg);
        //xSemaphoreGive( SqlLogSemaphore );
    //}

    Tick(); // OR call this if no splash 
}

BatteryLogApplication::~BatteryLogApplication() {
    if ( nullptr != batteryGraph ) { delete batteryGraph; }
}

bool BatteryLogApplication::Tick() {
    TemplateApplication::btnBack->Interact(touched,touchX,touchY);

    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(ThCol(background)); // use theme colors
        batteryGraph->DrawTo(canvas,10,10);

        TemplateApplication::btnBack->DrawTo(canvas);
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
