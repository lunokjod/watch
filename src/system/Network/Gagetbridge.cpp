#include <Arduino.h>
#include "Gagetbridge.hpp"
#include "../../app/LogView.hpp"
#include <ArduinoNvs.h>
#include <time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
extern SemaphoreHandle_t I2cMutex;

// http://www.espruino.com/Gadgetbridge
void ParseGadgetBridgeMessage(char * jsondata) {
    lNetLog("Gadgetbridge: '%s'\n",jsondata);

    const char Separator[]=";"; 
    const char SetTimeCMD[]="setTime(";
    const char SetTimeZoneCMD[]="E.setTimeZone(";
    const char GadgetBridgeCMD[]="GB(";
    char* token;

    for (token = strtok(jsondata, Separator); token; token = strtok(NULL, Separator)) {
        if ( 0 == strncmp(token,GadgetBridgeCMD,strlen(GadgetBridgeCMD))) {
            //lLog("COMMAND RECEIVED '%s'\n",token);
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
            test.month = timeinfo->tm_mon;
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

            //lLog("DATE: %02u-%02u-%04u HOUR: %02u:%02u:%02u\n", day(utcCalc),month(utcCalc),year(utcCalc),hour(utcCalc ));
        } else {
            //lNetLog("GadgetBridge: Unknown: '%s'\n", token);
        }
    }
    //lNetLog("Gadgetbridge: end\n");
}
