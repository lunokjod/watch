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
#include "../system/Datasources/database.hpp"
#include <Arduino_JSON.h>
#include "BLEPlayer.hpp"
#include "../system/SystemEvents.hpp"
#include "../static/img_play_48.xbm"
//#include "../static/img_reload_48.xbm"
#include "../static/img_pause_48.xbm"
#include "../system/Network/BLE.hpp"
char *MusicState=nullptr;
bool MusicStateDbObtained=false;
char *MusicInfo=nullptr;
bool MusicInfoDbObtained=false;
bool playing=false; // determine if is playing
unsigned long playingMS=0; // from what time?
size_t lastMusicStateID=-1;
size_t lastMusicInfoID=-1;
bool lastMusicInfoParsed=false;
bool lastMusicStateParsed=false;

long musicNotificationTime=-1;
long musicNotificationTimeFrom=-1;

static int MusicstateBLEPlayerApplicationSQLiteCallback(void *data, int argc, char **argv, char **azColName) {
   int i;
   //lSysLog("MUSIC STATE:\n");
   for (i = 0; i<argc; i++){
        if ( 0 == strcmp(azColName[i],"id")) {
            size_t newStateID=atoi(argv[i]);
            if ( newStateID != lastMusicStateID ) {
                lastMusicStateID=newStateID;
                lastMusicStateParsed=true;
            } else { lastMusicStateParsed=false; }
        } else if ( 0 == strcmp(azColName[i],"data")) {
            if ( nullptr != MusicState ) { free(MusicState); MusicState=nullptr; }
            MusicState=(char*)ps_malloc(strlen(argv[i])+1);
            if ( nullptr != MusicState) {
                strcpy(MusicState,argv[i]);
                if ( lastMusicStateParsed ) { MusicStateDbObtained=true; }
            }
        } else if ( 0 == strcmp(azColName[i],"timestampDB")) {
            //lLog("TIMESTAMP CAPTURED!\n");
            musicNotificationTime=atol(argv[i]);
        } else if ( 0 == strcmp(azColName[i],"nowDb")) {
            //lLog("TIME CAPTURED!\n");
            musicNotificationTimeFrom=atol(argv[i]);
        } else {
            lSysLog("   SQL: '%s' = '%s'\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
        }
   }
   return 0;
}

static int MusicinfoBLEPlayerApplicationSQLiteCallback(void *data, int argc, char **argv, char **azColName) {
   int i;
   //lSysLog("MUSIC INFO\n");
   for (i = 0; i<argc; i++){
        if ( 0 == strcmp(azColName[i],"id")) {
            size_t newInfoID=atoi(argv[i]);
            if ( newInfoID != lastMusicInfoID ) {
                lastMusicInfoID=newInfoID;
                lastMusicInfoParsed=true;
            } else { lastMusicInfoParsed=false; }
        } else if ( 0 == strcmp(azColName[i],"data")) {
            if ( nullptr != MusicInfo ) { free(MusicInfo); MusicInfo=nullptr; }
            MusicInfo=(char*)ps_malloc(strlen(argv[i])+1);
            if ( nullptr != MusicInfo) {
                strcpy(MusicInfo,argv[i]);
                if ( lastMusicInfoParsed ) { MusicInfoDbObtained=true; }
            }
        } else {
            //lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
        }
   }
   return 0;
}

void BLEPlayerApplication::DBRefresh() {
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        db_exec(lIoTsystemDatabase,"SELECT id,data FROM notifications WHERE data LIKE '{\"t\":\"musicinfo\",%' ORDER BY timestamp DESC LIMIT 1;",MusicinfoBLEPlayerApplicationSQLiteCallback);
        db_exec(lIoTsystemDatabase,"SELECT strftime('%s', 'now') AS nowDb ,strftime('%s', timestamp) AS timestampDB,id,data FROM notifications WHERE data LIKE '{\"t\":\"musicstate\",%' ORDER BY timestamp DESC LIMIT 1;",MusicstateBLEPlayerApplicationSQLiteCallback);
        xSemaphoreGive( SqlLogSemaphore ); // free
    }
}
const char MusicPlayGadgetbridgeCmd[] =  "{\"t\":\"music\",\"n\":\"play\"}";
const char MusicPauseGadgetbridgeCmd[] = "{\"t\":\"music\",\"n\":\"pause\"}";

BLEPlayerApplication::BLEPlayerApplication() {

    // BLESendUART("{ t:\"music\", n:\"play/pause/next/previous/volumeup/volumedown\" }");
    playBtn = new ButtonImageXBMWidget(120-35,160,70,70,[this](void *unused) {
        BLESendUART(MusicPlayGadgetbridgeCmd);
        lLog("@TODO BLEPLAYER BUTTOn PLAY\n");
        //@TODO send play to android
    },img_play_48_bits,img_play_48_height,img_play_48_width,ThCol(text),Drawable::MASK_COLOR,false);
    playBtn->SetEnabled(false);
    
    pauseBtn = new ButtonImageXBMWidget(120-35,160,70,70,[this](void *unused) {
        BLESendUART(MusicPauseGadgetbridgeCmd);
        lLog("@TODO BLEPLAYER BUTTOn PAUSE\n");
        //@TODO send play to android
    },img_pause_48_bits,img_pause_48_height,img_pause_48_width,ThCol(text),Drawable::MASK_COLOR,false);
    pauseBtn->SetEnabled(false);
    canvas->setTextFont(1);
    lastMusicStateID=-1;
    lastMusicInfoID=-1;
    lastMusicInfoParsed=false;
    lastMusicStateParsed=false;

    canvas->setTextDatum(BC_DATUM);
    canvas->setTextSize(3);
    canvas->setTextColor(ThCol(text));
    canvas->drawString("Gathering", TFT_WIDTH/2, TFT_HEIGHT/2);
    canvas->setTextDatum(TC_DATUM);
    canvas->drawString("data...", TFT_WIDTH/2, TFT_HEIGHT/2);

    //Tick();
}

BLEPlayerApplication::~BLEPlayerApplication() {
    delete playBtn;
    delete pauseBtn;
    parsedStateJSON=false;
    parsedInfoJSON=false;
}

bool BLEPlayerApplication::Tick() {
    if ( millis() > nextDBRefresh ) {
        DBRefresh();
        nextDBRefresh=millis()+5000;
    }
    playBtn->Interact(touched,touchX,touchY);
    pauseBtn->Interact(touched,touchX,touchY);

    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(ThCol(background)); // use theme colors
        TemplateApplication::Tick();

        if ( MusicInfoDbObtained ) {
            //if ( false == parsedInfoJSON ) {
            lAppLog("INFO Parsing: '%s'\n", MusicInfo);
            lastPlayerInfoEntry = JSON.parse(MusicInfo);
            free(MusicInfo);
            MusicInfo=nullptr;
            MusicInfoDbObtained=false;
            parsedInfoJSON=true;
            //}
            // second chance, nothing to show, return to watchface
            if ( false == parsedInfoJSON ) { LaunchWatchface(false); }
        }

        if ( MusicStateDbObtained ) {
            //if ( false == parsedStateJSON ) {
            lAppLog("STATE Parsing: '%s'\n", MusicState);
            lastPlayerStateEntry = JSON.parse(MusicState);
            free(MusicState);
            MusicState=nullptr;
            MusicStateDbObtained=false;
            parsedStateJSON=true;
            //}
            // second chance, nothing to show, return to watchface
            if ( false == parsedStateJSON ) { LaunchWatchface(false); }
        }
        if ( parsedInfoJSON ) {
            if (lastPlayerInfoEntry.hasOwnProperty("artist")) {
                const char * artist = (const char *)lastPlayerInfoEntry["artist"];
                if ( nullptr != artist) {
                    canvas->setTextDatum(TL_DATUM);
                    canvas->setTextSize(1);
                    canvas->setTextColor(ThCol(text));
                    canvas->drawString("Artist:", 30, 5);
                    canvas->setTextSize(3);
                    if ( 14 < strlen(artist) ) {
                        //lAppLog("Reduced Who font due doesn't fit\n");
                        canvas->setTextSize(2);
                    }
                    canvas->setTextDatum(TR_DATUM);
                    if (strlen(artist) > 0 ) {
                        //@TODO canvas->decodeUTF8()
                        canvas->drawString(artist, TFT_WIDTH-20, 15);
                    }
                }

            }

            if (lastPlayerInfoEntry.hasOwnProperty("track")) {
                const char * track = (const char *)lastPlayerInfoEntry["track"];
                if ( nullptr != track) {
                    canvas->setTextDatum(TL_DATUM);
                    canvas->setTextSize(1);
                    canvas->setTextColor(ThCol(text));
                    canvas->drawString("Track:", 30, 40);
                    canvas->setTextSize(3);
                    if ( 14 < strlen(track) ) {
                        //lAppLog("Reduced Who font due doesn't fit\n");
                        canvas->setTextSize(2);
                    }
                    canvas->setTextDatum(TL_DATUM);
                    if ( strlen(track) > 0 ) {
                        canvas->drawString(track, 20, 50);
                    }
                }

            }
            if (lastPlayerInfoEntry.hasOwnProperty("dur")) { dur=(int)lastPlayerInfoEntry["dur"]; }
            if (lastPlayerInfoEntry.hasOwnProperty("c")) { c=lastPlayerInfoEntry["c"]; }
            if (lastPlayerInfoEntry.hasOwnProperty("n")) { n=lastPlayerInfoEntry["n"]; }
        }
        if ( parsedStateJSON ) {
            // {"t":"musicstate","state":"play","position":1,"shuffle":1,"repeat":1}
            if (lastPlayerStateEntry.hasOwnProperty("state")) {
                pauseBtn->SetEnabled(false);
                playBtn->SetEnabled(false);
                if ( strlen((const char*)lastPlayerStateEntry["state"]) > 0 ) {
                    const char playStr[]="play";
                    const char pauseStr[]="pause";
                    if ( 0 == strncmp((const char*)lastPlayerStateEntry["state"],playStr,strlen(playStr))) {
                        if ( false == playing ) {
                            playing=true;
                            difference=musicNotificationTimeFrom-musicNotificationTime;
                            playingMS=millis();
                        }
                        pauseBtn->SetEnabled(true);
                    } else if ( 0 == strncmp((const char*)lastPlayerStateEntry["state"],pauseStr,strlen(pauseStr))) {
                        playing=false;
                        playBtn->SetEnabled(true);
                        difference=0;
                    } else {
                        playing=false;
                        difference=0;
                    }
                } else {
                    // nothing playing
                    canvas->setTextDatum(CC_DATUM);
                    canvas->setTextSize(3);
                    canvas->setTextColor(ThCol(text));
                    canvas->drawString("No media", TFT_WIDTH/2, TFT_HEIGHT/2);
                }
            }
            if (lastPlayerStateEntry.hasOwnProperty("position")) { position=lastPlayerStateEntry["position"]; }
            if (lastPlayerStateEntry.hasOwnProperty("shuffle")) { shuffle=lastPlayerStateEntry["shuffle"]; }
            if (lastPlayerStateEntry.hasOwnProperty("repeat")) { repeat=lastPlayerStateEntry["repeat"]; }
        }
        int currentPos = position;
        if ( playing ) { // fake the current position (eyecandy)
            currentPos=(position+difference)+((millis()-playingMS)/1000);
            if ( currentPos > dur ) { playing=false; currentPos=dur; }
        }
        if (( dur > 0 )&&( currentPos > 0 )) {
            canvas->setTextDatum(TL_DATUM);
            canvas->setTextSize(1);
            canvas->setTextColor(ThCol(text));
            canvas->drawString("Progress:", 30, 90);
            canvas->drawRect(5,105,230,10,ThCol(text_alt));
            int posDone=(currentPos*230)/dur;
            canvas->fillRect(5,105,posDone,10,ThCol(text));

            if ( currentPos > -1 ) {
                int seconds = currentPos;
                int minutes = seconds / 60;
                seconds %= 60;
                int hours = minutes / 60;
                minutes %= 60;
                canvas->setTextDatum(TL_DATUM);
                canvas->setTextSize(2);
                canvas->setTextColor(ThCol(text));
                char buff[10];
                sprintf(buff,"%02d:%02d:%02d",hours,minutes,seconds);
                canvas->drawString(buff, 10, 130);
            }
            if ( dur > 0 ) {
                int seconds = dur;
                int minutes = seconds / 60;
                seconds %= 60;
                int hours = minutes / 60;
                minutes %= 60;
                canvas->setTextDatum(TR_DATUM);
                canvas->setTextSize(2);
                canvas->setTextColor(ThCol(text));
                char buff[10];
                sprintf(buff,"%02d:%02d:%02d",hours,minutes,seconds);
                canvas->drawString(buff, TFT_WIDTH-10, 130);
            }
        }
        if ( playBtn->GetEnabled() ) { playBtn->DrawTo(canvas); }
        if ( pauseBtn->GetEnabled() ) { pauseBtn->DrawTo(canvas); }

        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
