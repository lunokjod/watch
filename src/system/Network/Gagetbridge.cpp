#include <Arduino.h>
#include <Arduino_JSON.h>
#include "Gagetbridge.hpp"
#include "../../app/LogView.hpp"
#include <ArduinoNvs.h>
#include <time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "../../app/Notifications.hpp"
#include "../Datasources/database.hpp"

extern SemaphoreHandle_t I2cMutex;

bool ParseGadgetBridgeJSON(JSONVar &json) {
    if (json.hasOwnProperty("t")) {
        if ( 0 == strcmp((const char*)json["t"],"notify") ) {
            // save to database
            String jsonString = json.stringify(json);
            char * myShitbuffer=(char*)ps_malloc(jsonString.length()+1);
            sprintf(myShitbuffer,"%s",jsonString.c_str());
            NotificatioLog(myShitbuffer);
            
            if ((json.hasOwnProperty("subject"))&&(json.hasOwnProperty("body"))&&(json.hasOwnProperty("sender"))) {
                //'{"t":"notify","id":1676021184,"subject":"aaaaaaaaaaaa","body":"aaaaaaaaaaaa","sender":"aaaaaaaaaaaa","tel":"aaaaaaaaaaaa"}
                lNetLog("BLE: NOTIFICATION type1: (%d) Subject: '%s' Body: '%s' From: '%s'\n",
                    (int)json["id"],
                    (const char*)json["subject"],
                    (const char*)json["body"],
                    (const char*)json["sender"]);
                LaunchApplication(new NotificacionsApplication(),true,false,true); // always push the last
                return true;
            } else if ((json.hasOwnProperty("src"))&&(json.hasOwnProperty("title"))){
                //{"t":"notify","id":1676021183,"src":"aaaaaaaaaaaa","title":"aaaaaaaaaaaa","body":"aaaaaaaaaaaa"}'
                if ((json.hasOwnProperty("body"))&&(0 != strncmp((const char*)json["body"],"0",1))) {
                    lNetLog("BLE: NOTIFICATION type2: (%d) Title: '%s' Source: '%s' Body: '%s'\n",
                        (int)json["id"],
                        (const char*)json["title"],
                        (const char*)json["src"],
                        (const char*)json["body"]);
                    LaunchApplication(new NotificacionsApplication(),true,false,true); // always push the last
                    //(int)json["id"],(const char*)json["title"],(const char*)json["src"],(const char*)json["body"]));
                } else {
                    lNetLog("BLE: NOTIFICATION type3: (%d) Title: '%s' Source: '%s'\n",
                        (int)json["id"],
                        (const char*)json["title"],
                        (const char*)json["src"]);
                    LaunchApplication(new NotificacionsApplication(),true,false,true); // always push the last
                    //(int)json["id"],(const char*)json["title"],(const char*)json["src"]));
                }
                return true; // for debug
            }
        } else if ( 0 == strcmp((const char*)json["t"],"call") ) {
            //{"t":"call","cmd":"","name":"ffff","number":"ffff"})":"ffff","sender":"ffff","tel":"ffff"}
            lLog("@TODO CALL NOTIFICATION\n");
            return false;
        } else if ( 0 == strcmp((const char*)json["t"],"musicinfo") ) {
            //{"t":"musicinfo","artist":"ffff(artist)","album":"ffff(album)","track":"ffff(track)","dur":10,"c":5,"n":2}
            lLog("@TODO MUSICINFO NOTIFICATION\n");
            return false;
        } else if ( 0 == strcmp((const char*)json["t"],"musicstate") ) {
            //{"t":"musicinfo","artist":"ffff(artist)","album":"ffff(album)","track":"ffff(track)","dur":10,"c":5,"n":2}
            lLog("@TODO MUSIC STATE NOTIFICATION\n");
            return false;
        } else if ( 0 == strcmp((const char*)json["t"],"find") ) {
            lLog("@TODO FIND FUNCTION\n");
            if (json.hasOwnProperty("n")) {
                if ( true == (bool)json["n"] ) {
                    lLog("USER FINDS ME!!!\n");
                } else {
                    lLog("GREAT! Found me!\n");
                }
                return false;
            }
        }
        return false;
    }
    // no 't' entry
    return false;
}
// http://www.espruino.com/Gadgetbridge
void ParseGadgetBridgeMessage(char * jsondata) {
    //lNetLog("BLE: UART: Gadgetbridge: '%s'\n",jsondata);

    const char Separator[]=";"; 
    const char SetTimeCMD[]="setTime(";
    const char SetTimeZoneCMD[]="E.setTimeZone(";
    const char GadgetBridgeCMD[]="GB(";
    char* token;

    for (token = strtok(jsondata, Separator); token; token = strtok(NULL, Separator)) {
        if ( 0 == strncmp(token,GadgetBridgeCMD,strlen(GadgetBridgeCMD))) {
            char *buff=(char*)ps_malloc(strlen(token)+1);
            if(nullptr==buff) {
                lNetLog("Gadgetbridge: ERROR: unable to get memory for parsing message\n");
                continue;
            }
            //memset(buff,0,strlen(token));
            sprintf(buff,"%s",token+strlen(GadgetBridgeCMD));
            for(int i=strlen(buff);i>0;i--) {
                if ( ( ')' == buff[i] )  && ( '}' == buff[i-1] ) ) {
                    //lLog("FOUND ON %d\n",i);
                    buff[i]=0; // terminate string
                    break;
                }
            }
            // now buff is a json stringified
            JSONVar myObject = JSON.parse(buff);
            if (JSON.typeof(myObject) == "undefined") {
                lNetLog("Garadgebridge: ERROR: malformed JSON\n");
                lNetLog("Gadgetbridge: RAW: '%s'\n",token);
                lNetLog("Gadgetbridge: CLEAN BUFFER: '%s'\n",buff);
                free(buff);
                continue;
            }
            // here is a JSON structure
            bool parsed = ParseGadgetBridgeJSON(myObject);
            if ( false == parsed ) {
                lNetLog("Gadgetbridge: WARNING: cannot underestand message\n");
                lNetLog("Gadgetbridge: CLEAN BUFFER: '%s'\n",JSON.stringify(myObject).c_str());
                free(buff);
                continue;
            }
            free(buff);
            continue;
        } else if ( 0 == strncmp(token,SetTimeZoneCMD,strlen(SetTimeZoneCMD))) {
            char numBuffer[30]={ 0 };
            sprintf(numBuffer,"%s",token+strlen(SetTimeZoneCMD));
            numBuffer[strlen(numBuffer)-1]=0; // remove last ")"
            int timezone = NVS.getInt("timezoneTime");
            int gbTimezone = atoi(numBuffer);
            lNetLog("Gadgetbridge: Current timezone: %+d offered: %+d\n",timezone,gbTimezone);
            if ( gbTimezone != timezone ) {
                NVS.setInt("timezoneTime",gbTimezone, false);
            }
            continue;
        } else if ( 0 == strncmp(token,SetTimeCMD,strlen(SetTimeCMD))) {
            char numBuffer[10]={ 0 };
            sprintf(numBuffer,"%s",token+strlen(SetTimeCMD));
            numBuffer[strlen(numBuffer)-1]=0; // remove last ")"
            long newTime = atol(numBuffer);
            lNetLog("Gadgetbridge: setTime: '%s'\n",numBuffer);
            //lNetLog("Gadgetbridge: setTime: '%ld'\n",newTime);
            time_t utcCalc = newTime;// - 2208988800UL;

            //time_t rawtime;
            struct tm * timeinfo;
            //time(&rawtime);
            timeinfo = localtime (&utcCalc);
            //lLog("DATETIME: %s",asctime(timeinfo));

            RTC_Date test; // = ttgo->rtc->getDateTime();
            test.day = timeinfo->tm_mday;
            test.month = timeinfo->tm_mon+1;
            test.year = timeinfo->tm_year;
            test.hour = timeinfo->tm_hour;
            test.minute = timeinfo->tm_min;
            test.second = timeinfo->tm_sec;
            BaseType_t done = xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
            if (pdTRUE == done) {
                ttgo->rtc->setDateTime(test);
                ttgo->rtc->syncToSystem();
                xSemaphoreGive(I2cMutex);
            }
            continue;
        } //else { lNetLog("BLE: UART: GadgetBridge: UNKONW: '%s'\n", token); }
    }
    //lNetLog("Gadgetbridge: end\n");
}
