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
#include <LilyGoWatch.h>
#include <Arduino_JSON.h>

#include "Notifications.hpp"
#include "UI/UI.hpp"
#include "LogView.hpp"
#include "../system/Datasources/database.hpp"
#include "../system/Application.hpp"
#include "../static/img_trash_32.xbm"
// App icons
#include "../../static/img_icon_telegram_48.c"
#include "../../static/img_icon_mail_48.c"

extern void NotificationLogSQL(const char * sqlQuery);
volatile int NotificationDbID=-1;
char *NotificationDbData=nullptr;
bool NotificationDbObtained=false;

NotificacionsApplication::~NotificacionsApplication() {
    if ( nullptr != NotificationDbData ) { free(NotificationDbData); NotificationDbData=nullptr; }
    NotificationDbObtained=false;
    delete btnMarkRead;
    //if ( nullptr != notification ) { delete notification; }
}

static int NotificacionsApplicationSQLiteCallback(void *data, int argc, char **argv, char **azColName) {
   int i;
   lSysLog("SQL: Data dump:\n");
   for (i = 0; i<argc; i++){
        if ( 0 == strcmp(azColName[i],"id")) { NotificationDbID=atoi(argv[i]); }
        else if ( 0 == strcmp(azColName[i],"data")) {
            if ( nullptr != NotificationDbData ) { free(NotificationDbData); NotificationDbData=nullptr; }
            NotificationDbData=(char*)ps_malloc(strlen(argv[i])+1);
            if ( nullptr != NotificationDbData) { strcpy(NotificationDbData,argv[i]); }
            NotificationDbObtained=true;
        }
        lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
   }
   return 0;
}

NotificacionsApplication::NotificacionsApplication() {
    NotificationDbID=-1;
    NotificationDbObtained=false;
    parsedJSON=false;
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        //int db_exec(sqlite3 *db, const char *sql,sqlite3_callback resultCallback=SQLiteCallback)
        db_exec(lIoTsystemDatabase,"SELECT * FROM notifications WHERE data LIKE '{\"t\":\"notify\",%' ORDER BY timestamp DESC LIMIT 1;",NotificacionsApplicationSQLiteCallback);
        xSemaphoreGive( SqlLogSemaphore ); // free
    }
    btnMarkRead=new ButtonImageXBMWidget(canvas->width()-48,canvas->height()-48,48,48,[&,this](void *bah){
        if ( -1 != NotificationDbID ) {
            const char SQLDeleteEntry[] = "DELETE FROM notifications WHERE id=%d;";
            char * buff=(char*)ps_malloc(255);
            if ( nullptr != buff ) {
                sprintf(buff,SQLDeleteEntry,NotificationDbID);
                NotificationLogSQL(buff);
                free(buff);
            }
            // force relaunch myself
            LaunchApplication(new NotificacionsApplication(),true,false,true);
        }
    },img_trash_32_bits,img_trash_32_height,img_trash_32_width,ThCol(text),ThCol(high),false);
    TemplateApplication::btnBack->xbmColor=TFT_WHITE;
    TemplateApplication::btnBack->InternalRedraw();
    Tick();
}

bool NotificacionsApplication::Tick() {
    //UINextTimeout = millis()+UITimeout;
    TemplateApplication::btnBack->Interact(touched,touchX,touchY);
    btnMarkRead->Interact(touched,touchX,touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        canvas->setTextFont(1);
        canvas->setTextSize(2);
        canvas->setTextColor(ThCol(text));
        canvas->setTextDatum(TL_DATUM);
        if (( NotificationDbObtained ) && (nullptr != NotificationDbData )) {
            if ( false == parsedJSON ) {
                lAppLog("Parsing: '%s'\n", NotificationDbData);
                notification = JSON.parse(NotificationDbData);
                NotificationDbObtained=false;
                free(NotificationDbData);
                NotificationDbData=nullptr;
                parsedJSON=true;
            }
            // second chance, nothing to show, return to watchface
            if ( false == parsedJSON ) { LaunchWatchface(false); }
        }
        if ( parsedJSON ) {
            if ( false == btnMarkRead->GetEnabled() ) { btnMarkRead->SetEnabled(true); }

            const char NotifyChar[] = "notify";
            if ((notification.hasOwnProperty("t"))&&(0 == strncmp((const char*)notification["t"],NotifyChar,strlen(NotifyChar)))) {
                //lAppLog("Is notify\n");
                if (notification.hasOwnProperty("src")) { // customize view

                    const char MailSrcChar[]="Gmail";
                    if (0 == strncmp((const char*)notification["src"],MailSrcChar,strlen(MailSrcChar))) {
                        CanvasWidget *mailImg = new CanvasWidget(img_icon_mail_48.height, img_icon_mail_48.width);
                        mailImg->canvas->pushImage(0,0,img_icon_mail_48.width,img_icon_mail_48.height,(uint16_t *)img_icon_mail_48.pixel_data);
                        mailImg->DrawTo(canvas,12,12);
                        if (notification.hasOwnProperty("title")) {
                            canvas->setTextDatum(TL_DATUM);
                            canvas->setTextSize(1);
                            canvas->drawString("From:", 85, 40);
                            const char * from = (const char *)notification["title"];
                            canvas->setTextSize(3);
                            if ( 8 < strlen(from) ) {
                                //lAppLog("Reduced Who font due doesn't fit\n");
                                canvas->setTextSize(2);
                            }
                            canvas->drawString(from, 74, 50);
                        }
                        delete(mailImg);
                    }

                    const char TelegramSrcChar[]="Telegram";
                    if (0 == strncmp((const char*)notification["src"],TelegramSrcChar,strlen(TelegramSrcChar))) {
                        CanvasWidget *telegramImg = new CanvasWidget(img_icon_telegram_48.height, img_icon_telegram_48.width);
                        telegramImg->canvas->pushImage(0,0,img_icon_telegram_48.width,img_icon_telegram_48.height,(uint16_t *)img_icon_telegram_48.pixel_data);
                        telegramImg->DrawTo(canvas,12,12);
                        delete(telegramImg);
                        if (notification.hasOwnProperty("title")) {
                            // parse tile (search channel name)
                            char* token;
                            const char Separator[] = ":";
                            uint8_t count=0;
                            // copy const to normal char
                            char *ptrShit=strdup((const char *)notification["title"]);
                            for (token = strtok(ptrShit, Separator); token; token = strtok(NULL, Separator)) {
                                if ( 0 == count ) {
                                    canvas->setTextSize(1);
                                    canvas->drawString("Channel:", 85, 5);
                                    canvas->setTextDatum(TL_DATUM);
                                    canvas->setTextSize(2);
                                    canvas->drawString(token, 74, 15);
                                } else {
                                    // ltrim
                                    char * ptrClean = token;
                                    while ( ' ' == *ptrClean) { ptrClean++; }
                                    // draw real sender user (assume a normal a-z string, otherwise scrambled at this time :P)
                                    //@TODO :icons: and other shit support like unicode UTF fucking nightmare
                                    canvas->setTextDatum(TL_DATUM);
                                    canvas->setTextSize(1);
                                    canvas->drawString("Who:", 85, 40);

                                    canvas->setTextSize(3);
                                    if ( 8 < strlen(ptrClean) ) {
                                        //lAppLog("Reduced Who font due doesn't fit\n");
                                        canvas->setTextSize(2);
                                    }
                                    canvas->drawString(ptrClean, 74, 50);
                                    break; // only the first part of name
                                }

                                //lAppLog("Token: %u '%s'\n",count,token);
                                count++;
                            }
                            free(ptrShit);
                        }
                    }
                } else if (notification.hasOwnProperty("title")) {
                    //lAppLog("Have title: '%s'\n",(const char*)notification["title"]);
                    canvas->setTextDatum(TL_DATUM);
                    canvas->setTextSize(3);
                    canvas->drawString((const char*)notification["title"], 74, 15);
                }
                //{"t":"notify","id":1676306282,"src":"Telegram","title":"Cacharreando!: Paco Carabaza","body":"le hiciste caso a la madre de Fran???"}
            } else {
                if (notification.hasOwnProperty("subject")) {
                    lAppLog("Have subject: '%s'\n",(const char*)notification["subject"]);
                }

                if (notification.hasOwnProperty("sender")) {
                    lAppLog("Have sender: '%s'\n",(const char*)notification["sender"]);
                }
                if (notification.hasOwnProperty("tel")) {
                    lAppLog("Have tel: '%s'\n",(const char*)notification["tel"]);
                }
            }

            // draw body
            if (notification.hasOwnProperty("body")) {
                //lAppLog("Have body: '%s'\n",(const char*)notification["body"]);
                canvas->setTextSize(1);
                canvas->drawString("What:", 85, 80);
                canvas->setTextSize(2);
                const char *bodyText = (const char*)notification["body"];
                size_t bodyLen = strlen(bodyText);
                int16_t x=10;
                int16_t y=90;
                size_t curr=0;
                //lAppLog("bodyLen: %u\n",bodyLen);
                for(size_t o=0;o<=bodyLen;o++) {
                    canvas->setCursor(x,y);
                    canvas->print(bodyText[o]);
                    x+=11;
                    curr++;
                    if ( x >= TFT_WIDTH-15 ) {
                        y+=15;
                        x=10;
                    }
                        // out of bounds?
                    if ( ( curr > 145 )&&(bodyLen> 148)) {
                        canvas->print("...");
                        break;
                    }
                }
            }
        } else {
            canvas->setTextSize(4);
            canvas->setTextDatum(CC_DATUM);
            canvas->drawString("Empty", canvas->width()/2, canvas->height()/2);
            btnMarkRead->SetEnabled(false);
        }


        /*
        if ( nullptr != msgfrom ) {
            if ( 0 == strcmp("Telegram",msgfrom)) {
                //canvas->setSwapBytes(true);
                //canvas->readRect(0,0,img_icon_telegram_64.width,img_icon_telegram_64.height,(uint16_t *)img_icon_telegram_64.pixel_data);
                canvas->pushRect(0,0,img_icon_telegram_64.width,img_icon_telegram_64.height,(uint16_t *)img_icon_telegram_64.pixel_data);
                //canvas->pushImage(0,0,img_icon_telegram_64.width,img_icon_telegram_64.height,(uint16_t *)img_icon_telegram_64.pixel_data);
                //canvas->setSwapBytes(false);
            }
            canvas->drawString(msgfrom, 74, 15);
        }

        */
        btnMarkRead->DrawTo(canvas);
        TemplateApplication::btnBack->DrawTo(canvas);
        nextRedraw=millis()+(2000);
        return true;
    }
    return false;
}
