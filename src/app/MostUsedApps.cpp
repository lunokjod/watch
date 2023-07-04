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

#include "../lunokIoT.hpp"
#include "../UI/AppLuITemplate.hpp"
#include "MostUsedApps.hpp"
#include "../UI/controls/IconMenu.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/XBM.hpp"
#include "LogView.hpp"

#include <LilyGoWatch.h>
#include "../system/Datasources/database.hpp"
#include "../resources.hpp"
#include <esp_task_wdt.h>
using namespace LuI;

const char *LuIMostUsedApplication::BLACKLIST[] = {"Main Menu", "Most used"};

LuIMostUsedApplication::LuIMostUsedApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    // fill view with background color
    canvas->fillSprite(ThCol(background)); // use theme colors

    /// create root container with two slots horizontal
    Container * screen = new Container(LuI_Horizontal_Layout,2);
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,2);

    // main view space
    Container * viewContainer = new Container(LuI_Horizontal_Layout,1);

    // if use quota, all the slot quotas must sum equal the total number of elements
    // example: default quota in 2 controls result 50/50% of view space with 1.0 of quota everyone (2.0 in total)

    screen->AddChild(viewContainer,1.65);
    // add bottom button bar shirnked
    screen->AddChild(bottomButtonContainer,0.35);

    // add back button to dismiss
    Button *backButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
    backButton->border=10;
    backButton->tapCallback=[](void * obj){ LaunchWatchface(); }; // callback when tap
    // load icon in XBM format
    XBM * backButtonIcon = new XBM(img_backscreen_24_width,img_backscreen_24_height,img_backscreen_24_bits);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.35);
    bottomButtonContainer->AddChild(new Text("Most used apps"),1.65);

    AddChild(screen);

    if ( nullptr != systemDatabase ) {
        // build empty tree
        entriesCounter = JSON.parse("{}");
        //systemDatabase->SendSQL("SELECT message FROM rawlogSession WHERE message LIKE 'Application: %%:%%';");
        systemDatabase->SendSQL("SELECT message FROM rawlogSession WHERE message LIKE 'Application: %%:%%';",[](void *data, int argc, char **argv, char **azColName) {
            LuIMostUsedApplication *self=(LuIMostUsedApplication*)data;
            for (int i = 0; i<argc; i++) {
                if ( 0 == strcmp(azColName[i],"message")) {
                    //lSysLog("-> %s\n", (argv[i] ? argv[i] : "NULL"));
                    // search app name
                    char * strFrom=nullptr;
                    int currentOffset=strlen(argv[i]);
                    while(currentOffset>-1) {
                        if ( ':' == argv[i][currentOffset] ) {
                            strFrom = argv[i]+currentOffset+1;
                            //lSysLog("CURRENT APP STRING OFFSET: %d as: '%s'\n",currentOffset,strFrom);
                            break;
                        }
                        currentOffset--;
                    }
                    if ( nullptr != strFrom ) {
                        // YEAH! FOUND!!
                        //lSysLog("'%s' app found from log\n",strFrom);
                        bool alreadyExists = self->entriesCounter.hasOwnProperty(strFrom);
                        if ( alreadyExists ) {
                            // GET VALUE
                            int currVal=(int)self->entriesCounter[(const char*)strFrom];
                            currVal++; // increment counter for app
                            //lSysLog("Updating entry: '%s' to: %d\n",strFrom,currVal);
                            // update value on json property
                            self->entriesCounter[(const char*)strFrom]= currVal; //JSONVar(currVal);
                            if ( self->HighCount < currVal ) { self->HighCount = currVal; }
                        } else {
                            // create entry with 1 as value
                            //lSysLog("Adding entry: '%s'\n",strFrom);
                            self->entriesCounter[(const char*)strFrom] = 1; //JSONVar(1);
                        }
                    }
                }
            }
            return 0;
        },this);
        /*,[&,this] { pending=false; });*/
    }
    // wait until SQL response @TODO this is a timed crap
    //while(pending) {
    TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
    xTaskDelayUntil( &nextCheck, (1000 / portTICK_PERIOD_MS) ); // wait a little bit
    esp_task_wdt_reset();
    //}
    // process the JSON dictionary 
    lAppLog("JSON: '%s'\n",JSON.stringify(entriesCounter).c_str());

    JSONVar keys = entriesCounter.keys();
    lAppLog("Most hits: %d total entries: %d\n",HighCount,keys.length());

    int appsHits[keys.length()+1]; // maintain list of hits
    //lAppLog("\nDISORDERED:\n\n");
    for (int i = 0; i < keys.length(); i++) {
        JSONVar ikey = keys[i];
        JSONVar ivalue = entriesCounter[ikey];
        //check names blacklist
        bool blackListed=false;
        for (int n = 0; n < sizeof(BLACKLIST) / sizeof(*BLACKLIST); n++) {
            if ( 0 == strcmp(BLACKLIST[n],(const char *)ikey)) {
                blackListed=true;
                break;
            }
        }
        if ( blackListed ) { continue; }
        // ignore low hits 
        if ( (int)ivalue > MINHITS ) {
            appsHits[i] = (int)ivalue;
            //lAppLog("%d '%s' hits: %d\n",i, (const char*)ikey,appsHits[i]);
        } else {
            appsHits[i] = IGNORED;
        }
    }
    ReverseBubbleSort(appsHits,keys.length());
    //lAppLog("\nORDERED:\n\n");
    size_t maxEntries=0;
    const char* appsOrderedNames[keys.length()+1];
    for (int i = 0; i < keys.length(); i++) {
        if ( IGNORED == appsHits[i] ) { continue; } // ignore it
        if ( i+1 > keys.length() ) { continue; } // out of array
        for (int c=0; c < keys.length(); c++) {
            JSONVar ikey = keys[c];
            JSONVar ivalue = entriesCounter[ikey];
            if ( appsHits[i] == (int)ivalue ) {
                // remove repeated
                bool found=false;
                for ( int r=0;r<maxEntries;r++) {
                    if ( 0 == strcmp((const char*)ikey,appsOrderedNames[r]) ) {
                        found=true;
                        break;
                    }
                }
                if ( false==found ) {
                    //lAppLog("%u '%s' hits: %d\n",maxEntries, (const char*)ikey,appsHits[i]);
                    appsOrderedNames[maxEntries]=(const char * )ikey;
                    maxEntries++;
                }
                break;
            }
        }
    }
    lAppLog("MOST USED APPS:\n");
    for (int i = 0; i < maxEntries; i++) {
        lAppLog("%u '%s'\n",i,appsOrderedNames[i]);
    }
    lAppLog("Total valid entries: %u\n",maxEntries);
}

bool LuIMostUsedApplication::Tick() {
    return TemplateLuIApplication::Tick();
}
