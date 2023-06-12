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
#include "CalendarLogDetail.hpp"
#include "Calendar.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/Text.hpp"
#include "../resources.hpp"
#include "LogView.hpp" // logs
#include <LittleFS.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <FS.h>
#include <esp_task_wdt.h>
using namespace LuI;

CalendarLogDetailApplication::CalendarLogDetailApplication(int year, int month, int day) {
    lAppLog("Show Log Detail for: %d-%d-%d\n",day,month,year);
    char fileName[255];
    sprintf(fileName,"/littlefs/lwatch_%d-%d.db",day,month);
    lAppLog("Must open filename '%s'\n",fileName);
        /*
    if ( LoT().IsLittleFSEnabled() ) {
        if ( nullptr != cacheFileName ) { 
            free(cacheFileName);
            cacheFileName=nullptr;
        }
        cacheFileName = (char*)ps_malloc(255);
        sprintf(cacheFileName,"/_cache/img_calendar_%d-%d.raw",day,month);
        if ( LittleFS.exists(cacheFileName) ) { // no file cache
            lAppLog("File cache found '%s'\n",cacheFileName);
            fs::File cacheFile = LittleFS.open(cacheFileName,"r",false);
            if ( 0 != cacheFile ) {
                lAppLog("Alloc memory...(%u)\n",cacheFile.size());
                char *imageData=(char*)ps_malloc(cacheFile.size());
                cacheFile.readBytes(imageData,cacheFile.size());
                uint16_t * height=(uint16_t*)imageData;
                uint16_t * width=(uint16_t*)imageData;
                uint16_t * data=width+1;
                width++;
                lAppLog("Loading (%dx%d)...\n",*width,*height);
                logView = new Buffer(*height,*width);
                logView->showBars=false;
                lAppLog("Draw on logView...\n");
                //logView->GetCanvas()->pushRect(0,0,*width,*height,data);
                viewContainer->AddChild(logView);
                logView->dirty=true;
                free(imageData);
                cacheFile.close();
            }
        } else { // file cache exists
            lAppLog("No cache file found at '%s', generating new one...\n",cacheFileName);
            */
        currentDB=new Database(fileName);
        if ( nullptr != currentDB ) {
            currentDB->SendSQL("SELECT COUNT(1) FROM rawlogSession;",[](void *data, int argc, char **argv, char **azColName) {
                CalendarLogDetailApplication *self=(CalendarLogDetailApplication*)data;
                for (int i = 0; i<argc; i++){
                    if ( 0 == strcmp("COUNT(1)",azColName[i]) ) {
                        self->entries=atoi(argv[i]);
                        lLog("Day database total entries: %d\n",self->entries);
                        self->beginScrub=true;
                        return 0;
                    }
                }
                return 0;
            },this);
        }
       // }
    //}
    directDraw=false;
    canvas->fillSprite(ThCol(background));
    Container * screen = new Container(LuI_Horizontal_Layout,2); 
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,2);
    // main view space
    viewContainer = new Container(LuI_Horizontal_Layout,1);

    screen->AddChild(viewContainer,1.6);
    screen->AddChild(bottomButtonContainer,0.4);

    // add back button to dismiss
    Button *backButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
    backButton->border=0;
    backButton->tapCallback=[](void * obj){ LaunchApplication(new CalendarApplication()); }; // callback when tap
    // load icon in XBM format
    XBM * backButtonIcon = new XBM(img_backscreen_42_width,img_backscreen_42_height,img_backscreen_42_bits);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.4);

    char buffer[255];
    sprintf(buffer,"log %d-%d-%d", day,month,year);
    bottomButtonContainer->AddChild(new Text(buffer),1.6);

    AddChild(screen);
    directDraw=true;
}

CalendarLogDetailApplication::~CalendarLogDetailApplication() {
    if ( nullptr != currentDB ) { delete currentDB; }
    if ( nullptr != PowerGraph ) { delete PowerGraph; }
    if ( nullptr != ActivityGraph ) { delete ActivityGraph; }
    if ( nullptr != AppsGraph ) { delete AppsGraph; }
    if ( nullptr != NetworkGraph ) { delete NetworkGraph; }
    if ( nullptr != cacheFileName ) {  free(cacheFileName); }
}

bool CalendarLogDetailApplication::Tick() {
    if ( beginScrub ) {
        logView = new Buffer(entries ,240);
        logView->showBars=false;
        logView->GetBuffer()->fillSprite(ByteSwap(ThCol(background)));
        viewContainer->AddChild(logView);

        logView->GetBuffer()->setTextSize(1);
        logView->GetBuffer()->setTextDatum(TL_DATUM);
        logView->GetBuffer()->setTextColor(TFT_WHITE);
        PowerGraph = new GraphWidget(3,entries,0,1,TFT_BLACK);
        PowerGraph->inverted=true;
        ActivityGraph = new GraphWidget(3,entries,0,1,TFT_BLACK);
        ActivityGraph->inverted=true;
        AppsGraph = new GraphWidget(3,entries,0,1,TFT_BLACK);
        AppsGraph->inverted=true;
        NetworkGraph = new GraphWidget(3,entries,0,1,TFT_BLACK);
        NetworkGraph->inverted=true;
        logView->GetBuffer()->setFreeFont(&FreeMonoBold12pt7b);
        logView->GetBuffer()->drawString("Power:",15,10);
        logView->GetBuffer()->drawString("Activiy:",15,70);
        logView->GetBuffer()->drawString("Application:",15,130);
        logView->GetBuffer()->drawString("Network:",15,190);
        AppsGraph->markColor = ByteSwap(TFT_BLACK); // stupid monkey x'D
        logView->GetBuffer()->setFreeFont(&FreeMono9pt7b);
        currentDB->SendSQL("SELECT timestamp,message FROM rawlogSession WHERE 1;",[](void *data, int argc, char **argv, char **azColName) {
            //bool found=false;
            CalendarLogDetailApplication *self=(CalendarLogDetailApplication*)data;
            const char appBanner[]="Application:";
            const char wifiOff[]="WiFi: disconnect";
            const char wifiOn[]="WiFi: connected";
            const char BLEScan[]="BLE: pasive scan";
            const char BLEOn[]="BLE: enabled";
            const char BLEOff[]="BLE: disabled";
            const char BLELocationFound[]="BLELocation:";
            const char TempReport[]="Temperatures:";
            const char HallReport[]="Hall:";
            const char AccelReport[]="Acceleromether:";
            const char BatteryReport[]="Battery:";
            const char UsageReport[]="Usage stats:";
            for (int i = 0; i<argc; i++){
                if ( 0 == strcmp("timestamp",azColName[i])) {
                    char * date = strtok(argv[i], " ");
                    char * hour = strtok(NULL, "\0");
                    // power time marks
                    if ( self->currentPowerWidth < self->currentBufferDisplacement ) {
                        self->logView->GetBuffer()->drawString(hour,self->currentPowerWidth,55);
                        self->currentPowerWidth+=120;
                    }
                    // activity
                    if ( self->currentActivityWidth < self->currentBufferDisplacement ) {
                        self->logView->GetBuffer()->drawString(hour,self->currentActivityWidth,115);
                        self->currentActivityWidth+=120;
                    }
                    // apps
                    if ( self->currentAppsWidth < self->currentBufferDisplacement ) {
                        self->logView->GetBuffer()->drawString(hour,self->currentAppsWidth,170);
                        self->currentAppsWidth+=120;
                    }
                    // network
                    if ( self->currentNetworkWidth < self->currentBufferDisplacement ) {
                        self->logView->GetBuffer()->drawString(hour,self->currentNetworkWidth,225);
                        self->currentNetworkWidth+=120;
                    }
                } else if ( 0 == strcmp("message",azColName[i])) {
                    // feed power bar
                    if ( 0 == strcmp(argv[i],"begin")) { self->PowerGraph->markColor = ByteSwap(TFT_YELLOW); }
                    else if ( 0 == strcmp(argv[i],"end")) { self->PowerGraph->markColor = ByteSwap(TFT_BLACK); }
                    else if ( 0 == strcmp(argv[i],"wake")) { self->PowerGraph->markColor = ByteSwap(TFT_GREEN); }
                    else if ( 0 == strcmp(argv[i],"stop")) { self->PowerGraph->markColor = ByteSwap(TFT_DARKGREEN); }
                    // feed activity bar
                    else if ( 0 == strcmp(argv[i],"Activity: Running")) { self->ActivityGraph->markColor = ByteSwap(TFT_RED); }
                    else if ( 0 == strcmp(argv[i],"Activity: Walking")) { self->ActivityGraph->markColor = ByteSwap(TFT_YELLOW); }
                    else if ( 0 == strcmp(argv[i],"Activity: None")) { self->ActivityGraph->markColor = ByteSwap(TFT_GREEN); }
                    // feed app bar
                    else if ( 0 == strncmp(argv[i],appBanner,strlen(appBanner))) { self->AppsGraph->markColor = ByteSwap(TFT_CYAN); }
                    else if ( 0 == strncmp(argv[i],TempReport,strlen(TempReport))) { self->AppsGraph->markColor = ByteSwap(TFT_YELLOW); }
                    else if ( 0 == strncmp(argv[i],HallReport,strlen(HallReport))) { self->AppsGraph->markColor = ByteSwap(TFT_LIGHTGREY); }
                    else if ( 0 == strncmp(argv[i],AccelReport,strlen(AccelReport))) { self->AppsGraph->markColor = ByteSwap(TFT_RED); }
                    else if ( 0 == strncmp(argv[i],BatteryReport,strlen(BatteryReport))) { self->AppsGraph->markColor = ByteSwap(TFT_DARKGREEN); }
                    else if ( 0 == strncmp(argv[i],UsageReport,strlen(UsageReport))) { self->AppsGraph->markColor = ByteSwap(TFT_GREEN); }
                    // feed network bar
                    else if ( 0 == strncmp(argv[i],wifiOn,strlen(wifiOn))) { self->NetworkGraph->markColor = ByteSwap(TFT_GREEN); }
                    else if ( 0 == strncmp(argv[i],wifiOff,strlen(wifiOff))) { self->NetworkGraph->markColor = ByteSwap(TFT_BLACK); }

                    else if ( 0 == strncmp(argv[i],BLEScan,strlen(BLEScan))) { self->NetworkGraph->markColor = ByteSwap(TFT_DARKGREY); }
                    else if ( 0 == strncmp(argv[i],BLEOn,strlen(BLEOn))) { self->NetworkGraph->markColor = ByteSwap(TFT_BLUE); }
                    else if ( 0 == strncmp(argv[i],BLEOff,strlen(BLEOff))) { self->NetworkGraph->markColor = ByteSwap(TFT_BLACK); }
                    else if ( 0 == strncmp(argv[i],BLELocationFound,strlen(BLELocationFound))) { self->NetworkGraph->markColor = ByteSwap(TFT_WHITE); }

                    else {
                        lLog("@MESSAGE: '%s'\n", argv[i]);  
                    }
                    //found=true;
                } //else {
                //lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
                //}
            }
            self->currentBufferDisplacement++;
            self->PowerGraph->PushValue(1);
            esp_task_wdt_reset();
            self->ActivityGraph->PushValue(1);
            esp_task_wdt_reset();
            self->AppsGraph->PushValue(1);
            esp_task_wdt_reset();
            self->NetworkGraph->PushValue(1);
            esp_task_wdt_reset();
            if ( self->currentBufferDisplacement >= self->entries ) {
                self->PowerGraph->DrawTo(self->logView->GetBuffer(),0,40);
                self->ActivityGraph->DrawTo(self->logView->GetBuffer(),0,100);
                self->AppsGraph->DrawTo(self->logView->GetBuffer(),0,160);
                self->NetworkGraph->DrawTo(self->logView->GetBuffer(),0,220);
                self->logView->dirty=true;
                /*
                LittleFS.mkdir("/_cache");
                lLog("Save cache file to '%s'\n",self->cacheFileName);
                const size_t SpaceToAlloc= (self->canvas->width()*self->canvas->height()*3)+(sizeof(int16_t)*2);
                uint16_t * imageData=(uint16_t*)ps_malloc(SpaceToAlloc);
                TFT_eSprite * currentView = self->logView->GetBuffer();
                // header
                imageData[0] = currentView->height();
                imageData[1] = currentView->width();
                // bitmap
                self->canvas->readRect(0,0,currentView->width(),currentView->height(),imageData+2);
                fs::File desc = LittleFS.open(self->cacheFileName,"w",true);
                desc.write((const uint8_t*)imageData,SpaceToAlloc);
                desc.close();

                free(imageData);
                //LittleFS.open("/_cache/")
                */
                lLog("Day database scrub end\n");
            }
            /*
            static unsigned long lapse=0;
            if ( lapse < millis()) {
                //lLog("REFRESH\n");
                self->logView->dirty=true;
                lapse=millis()+2000;
            }*/
            UINextTimeout = millis()+UITimeout; // disable screen timeout
            return 0;
        },this);
        beginScrub=false;
        UINextTimeout = millis()+UITimeout*3; // enlarge screen timeout
    }
    EventHandler();
    return false;
}
