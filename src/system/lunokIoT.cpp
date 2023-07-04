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

#include <stdio.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <esp_task_wdt.h>
#include <esp_ota_ops.h>

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoNvs.h>
#include <WiFi.h>
#include <cstdio>
//#include <libraries/TFT_eSPI/TFT_eSPI.h>

#include <LilyGoWatch.h>
#include "driver/uart.h"

//#include <SPI.h>
//#include <FS.h>

#include "../lunokIoT.hpp"
#include "../lunokiot_config.hpp"
#include "UI/BootSplash.hpp"
#include "SystemEvents.hpp"
#include "Datasources/database.hpp"
//#include "Datasources/perceptron.hpp"
#include "PMU/AXP202.hpp"
#include "system/Network.hpp"
#include "system/Storage/Settings.hpp"
#include "ulp.hpp"
class NTPWifiTask;

#include "app/Steps.hpp" // Step manager
#include "app/LogView.hpp" // log

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "system/lua.hpp"
#include "app/Stopwatch.hpp"
//#include <esp32/himem.h>
//#include <esp32/spiram.h>

extern SemaphoreHandle_t I2cMutex;

/* monkey patch x'D
// build_flags = -Wl,--wrap=ps_malloc
void ARDUINO_ISR_ATTR *__real_ps_malloc(size_t size);
void ARDUINO_ISR_ATTR *__wrap_ps_malloc(size_t size) {
    void * address = __real_ps_malloc(size);
    lLog("PSRAM: malloc(%u) = %p\n", size, address);
    return address;
}*/

TTGOClass *ttgo = TTGOClass::getWatch();
//TFT_eSPI * tft = nullptr;
//TFT_eSPI * tft = new TFT_eSPI();
TFT_eSPI * tft = nullptr;
PCF8563_Class * rtc = nullptr;

extern bool ntpSyncDone;
//extern const char *ntpServer;

volatile RTC_NOINIT_ATTR int16_t 	lt_sys_reset;//Global var
RTC_NOINIT_ATTR char lt_sys_tsk_ovf[16];//Global var

// requires configCHECK_FOR_STACK_OVERFLOW ¿ArduinoFW?
void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName ) {
    snprintf(lt_sys_tsk_ovf, 16, "%s", pcTaskName);
    lt_sys_reset = 21;

    ets_printf("\n\n\n@TODO ***ERROR*** Stack overflow in task '");
    ets_printf((char *)pcTaskName);
    ets_printf("'\n\n\n\n");

    abort();
}

extern "C" void esp_task_wdt_isr_user_handler(void) {
/***************************************************************************//**
\brief This function is called by task_wdt_isr function (ISR for when TWDT times out).
\details It can be redefined in user code to handle twdt events.
Note: It has the same limitations as the interrupt function.
Do not use ESP_LOGI functions inside.
*******************************************************************************/

    ets_printf("\n\n\n@TODO ERROR: Watchdog TIMEOUT triggered\n\n\n\n");
}



void foo(int x) {
    lLog("Hello %d!\n", x);
}

void *my_dlsym(void *handle, const char *name) {
    if (strcmp(name, "foo") == 0) return (void*)foo;
    return NULL;
}

// Swap function
void Swap(int *arr,int i,int j){
    int temp=arr[i];
      arr[i]=arr[j];
      arr[j]=temp;
}
  
// A function to implement bubble sort
void ReverseBubbleSort(int arr[], int n) {
    int i, j;
    for (i = 0; i < n - 1; i++)
  
        // Last i elements are already 
        // in place
        for (j = 0; j < n - i - 1; j++)
            if (arr[j] < arr[j + 1])
                Swap(arr,j,j + 1);
}

bool LunokIoT::IsLittleFSEnabled() { return LittleFSReady; }

//#include "esp_console.h"

const char *BLECreateTable=(const char *)"CREATE TABLE if not exists bluetooth ( timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, address text NOT NULL UNIQUE, distance INT DEFAULT -1, locationGroup INT DEFAULT 0);";
//const char *queryCreateRAWLog=(const char *)"CREATE TABLE if not exists rawlog ( timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, message text NOT NULL);";
const char *queryCreateNotifications=(const char *)"CREATE TABLE if not exists notifications (id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, data text NOT NULL);";
const char *queryCreateSessionRAWLog=(const char *)"CREATE TABLE if not exists rawlogSession (id INTEGER PRIMARY KEY, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL, message text NOT NULL);";

void LunokIoT::CleanCache(const char *path) {
    if ( false == LittleFSReady ) { return; }
    fs::File root = LittleFS.open(path);
    if (root) {
        if (root.isDirectory()) {
            fs::File file = root.openNextFile();
            while (file) {
                if (file.isDirectory()) {
                    char * newPathToScan = (char*)ps_malloc(strlen(path)+strlen(file.name())+1);
                    sprintf(newPathToScan,"%s%s",path,file.name());
                    CleanCache(newPathToScan);
                    free(newPathToScan);
                } else {
                    char buffer[255];
                    sprintf(buffer,"%s/%s", path,file.name());
                    lSysLog("Destroy cache file: %s\n",buffer);
                    file.close();
                    LittleFS.remove(buffer);
                }
                file = root.openNextFile();
            }
        }
    }
    bool reachClean = LittleFS.rmdir(path);
    lSysLog("Destroy old cache folder: %s\n",(reachClean?"true":"false"));
}

void LunokIoT::DestroyOldFiles() {
    if ( false == LittleFSReady ) { return; }
    int monthToShow=0;
    int monthDay=1;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        //lAppLog("Time: %02d:%02d:%02d %02d-%02d-%04d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon, 1900 + timeinfo.tm_year);
        monthDay=timeinfo.tm_mday;
        lSysLog("DestroyOldFiles() Current day: %d\n", monthDay);
        monthToShow=timeinfo.tm_mon; // 0 is JANUARY
        lSysLog("DestroyOldFiles() Current month: %d\n", monthToShow+1); // human readable 1~12
    }
    fs::File root = LittleFS.open("/");
    if (root) {
        if (root.isDirectory()) { // only files please
            fs::File file = root.openNextFile();
            while (file) {
                if (false == file.isDirectory()) {
                    const char endStrFmt[] = "-%d.db";
                    char nameToCompare[255];
                    sprintf(nameToCompare,endStrFmt,monthToShow); // list the last month files
                    const char *filename = file.name();
                    if ( 0 != strncmp(filename+strlen(filename)-strlen(nameToCompare),nameToCompare,strlen(nameToCompare)) ) {
                        file = root.openNextFile();
                        continue;
                    }
                    lSysLog("DestroyOldFiles() @TODO DESTROY OLD FILES??? %s\n",file.name());
                    /*
[4293165][lunokIoT] @TODO DESTROY OLD FILES??? geoip.log
[4308993][lunokIoT] @TODO DESTROY OLD FILES??? lwatch.db
[4332743][lunokIoT] @TODO DESTROY OLD FILES??? lwatch_10-6.db
[4357064][lunokIoT] @TODO DESTROY OLD FILES??? lwatch_12-6.db
[4380613][lunokIoT] @TODO DESTROY OLD FILES??? oweather.log
                    */
                    /*
                    char buffer[255];
                    sprintf(buffer,"/%s",file.name());
                    lSysLog("Destroy cache file: %s\n",buffer);
                    file.close();
                    LittleFS.remove(buffer);
                    */
                }
                file = root.openNextFile();
            }
        }
    }

}

LunokIoT::LunokIoT() {
    int64_t beginBootTime = esp_timer_get_time(); // stats 'bout boot time
    InitLogs(); // need for stdout on usb-uart

    // announce myself with build information if serial debug is enabled
    lSysLog("'компаньон' #%d//%s//\n",LUNOKIOT_BUILD_NUMBER,LUNOKIOT_KEY);
    // report initial memory
    FreeSpace();
    // Initialize lilygo lib (mandatory at this time)
    ttgo->begin();
    tft = ttgo->tft; // set convenient extern
    
    rtc = ttgo->rtc; // set convenient extern
    settings=new SystemSettings();

    // setup the tft orientation
    uint8_t rotation = settings->GetInt(SystemSettings::SettingKey::ScreenRotation);
    tft->setRotation(rotation); // user selected rotation (0 by default)

    size_t themeOffset = settings->GetInt(SystemSettings::SettingKey::Theme);
    currentColorPalette = &AllColorPaletes[themeOffset]; // set the color palette (informative)
    currentThemeScheme = &AllThemes[themeOffset]; // set the GUI colors

    SplashAnnounce(); // simple eyecandy meanwhile boot (themed)
    SplashAnnounceBegin(); // anounce user about boot begins
    SplashAnnounce("    Storage    ");

    LittleFSReady = LittleFS.begin(); // needed for SQLite activity database and other blobs
    lSysLog("Storage: NVS: %s, LittleFS: %s\n", (settings->IsNVSEnabled()?"yes":"NO"), (LittleFSReady?"yes":"NO"));


    bool alreadyFormattedLittleFS = settings->GetInt(SystemSettings::SettingKey::LittleFSFormat);
    if ( false == alreadyFormattedLittleFS ) {
        lSysLog("LittleFS: user wants format disk\n");
        LittleFSReady=false; // mark as clean forced
    }
    // format LittleFS if needed
    if ( false == LittleFSReady ) {
        lSysLog("LittleFS: Format LittleFS....\n");
        SplashAnnounce("LittleFS format");
        LittleFSReady = LittleFS.format();
        if ( false == LittleFSReady ) {
            lSysLog("LittleFS: ERROR: Unable to format!!!\n");
            LittleFSReady=false;
        } else {
            LittleFSReady = LittleFS.begin(); // mount again
            NVS.setInt("littleFSReady",true,false); // assume format reached and disable it in next boot
        }
    }
    //LittleFS.mkdir("/Download");
    //LittleFS.mkdir("/_cache");
    ListLittleFS(); // show contents to serial
    CleanCache();
    SplashAnnounce("   freeRTOS   ");

    // https://github.com/espressif/esp-idf/blob/9ee3c8337d3c4f7914f62527e7f7c78d7167be95/examples/system/task_watchdog/main/task_watchdog_example_main.c
    esp_err_t taskWatchdogResult = esp_task_wdt_init(CONFIG_ESP_TASK_WDT_TIMEOUT_S, false); // CONFIG_ESP_TASK_WDT_TIMEOUT_S
    if ( ESP_OK != taskWatchdogResult ) {
        lEvLog("ERROR: Unable to start Task Watchdog\n");
        FreeSpace();
    }

    taskWatchdogResult = esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(1));
    if ( ESP_OK != taskWatchdogResult ) {
        lEvLog("WARNING: Unable to start Task Watchdog on APP_CPU(1)\n");
    }
    SplashAnnounce("    Boot check    ");
    BootReason(); // announce boot reason and from what partition to serial
    //ULPRun();


    #ifdef LILYGO_WATCH_2020_V3
    ttgo->motor_begin(); // start the motor for haptic notifications
    #endif
    SplashAnnounce("    System    ");
    lSysLog("System initializing...\n");
    SplashAnnounce("  System (RTC) ");
    // get if are in summertime
    int daylight = NVS.getInt("summerTime");
    // get the GMT timezone
    long timezone = NVS.getInt("timezoneTime");
    // announce values to log
    lEvLog("RTC: Config: Summer time: '%s', GMT: %+d\n",(daylight?"yes":"no"),timezone);
    configTime(timezone*3600, daylight*3600, NTPWifiTask::ntpServer); // set ntp server query
    
    struct tm timeinfo;

    BaseType_t done = xSemaphoreTake(I2cMutex, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
    if (pdTRUE == done) {
        if ( ttgo->rtc->isValid() ) { // woa! RTC seems to guard the correct timedate from last execution :D
            lEvLog("RTC: The time-date seems valid, thanks to secondary battery :)\n");
            // inquiry RTC via i2C
            RTC_Date r = ttgo->rtc->getDateTime();
            lEvLog("RTC: Time:        %02u:%02u:%02u date: %02u-%02u-%04u\n",r.hour,r.minute,r.second,r.day,r.month,r.year);
            ttgo->rtc->syncToSystem(); // feed ESP32 with correct time
            if (getLocalTime(&timeinfo)) {
                lEvLog("ESP32: Sync Time: %02d:%02d:%02d date: %02d-%02d-%04d\n",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec,timeinfo.tm_mday,timeinfo.tm_mon+1,1900+timeinfo.tm_year);
                ntpSyncDone=true; // RTC says "I'm ok", bypass NTP as time font
            }
            SplashAnnounce("    RTC OK    ");
        }
        xSemaphoreGive(I2cMutex);
    }
    SplashAnnounce("    Cleanup    ");
    DestroyOldFiles();
    //SplashAnnounce("      VFS      ");
    //VFSInit();

    SplashAnnounce("      LUA      ");
    LuaInit();

    SplashAnnounce("    Database    ");
    JournalDatabase();
    StartDatabase(); // must be started after RTC sync (timestamped inserts need it to be coherent)
    if ( nullptr != systemDatabase ) {
        systemDatabase->SendSQL("BEGIN TRANSACTION;");
        systemDatabase->SendSQL(queryCreateNotifications);
        systemDatabase->SendSQL(BLECreateTable);
        systemDatabase->SendSQL(queryCreateSessionRAWLog);
        systemDatabase->SendSQL("END TRANSACTION;");
        systemDatabase->Commit();
    }
    SqlLog("begin");
    SplashAnnounce("   User prefs   ");
    userTall= NVS.getInt("UserTall");        // get how high is the user?
    if ( 0 == userTall ) { userTall = 120; } // almost midget value
    
    userMaleFemale = NVS.getInt("UserSex");  // maybe is better use "hip type A/B" due to no offend any gender
    if ( true == userMaleFemale ) { stepDistanceCm = userTall * WOMAN_STEP_PROPORTION; }
    else { stepDistanceCm = userTall * MAN_STEP_PROPORTION; }
    //StopwatchApplication::pauseTime=(unsigned long)NVS.getInt("sw_pause");
    //StopwatchApplication::starTime=(unsigned long)NVS.getInt("sw_start");

    SplashAnnounce("    Events     ");

    // Build the lunokiot message bus
    SystemEventsStart();

    // hooks for activities
    SplashAnnounce("   Activities   ");
    StartPMUMonitor();
    selectedWatchFace=NVS.getInt("UWatchF");
    if ( selectedWatchFace >= WatchFacesAvailiable() ) {
        selectedWatchFace=0; // reset to defaults if out of range
    }

    // databas cleanup
    SplashAnnounce("      Cleanup      ");
    CleanupDatabase();
    SplashAnnounce("     Launch GUI      ");
    UIStart();

    InstallRotateLogs();
    if ( fromDeepSleep ) {
        const float delaySeconds = 30;
        lEvLog("LogRotate: Cleanup launch after deep sleep planned in %g seconds...\n",delaySeconds);
        LunokIoTSystemLogRotationByDeepSleep.once(delaySeconds,[]() { LoT().LogRotate(); });
    }
    SplashAnnounceEnd();

    SetUserBrightness();
    SystemEventBootEnd(); // notify to system end boot procedure (SystemEvents must launch watchface here)

    FreeSpace();
    
    int64_t endBootTime = esp_timer_get_time()-beginBootTime;
    lSysLog("loaded in %lld us\n",endBootTime);
};

void LunokIoT::InitLogs() {
    // Get serial comms with you
    #ifdef LUNOKIOT_SERIAL
    /*
    // start console
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "lunokIoT>";
    repl_config.max_cmdline_length = 80;
    esp_console_register_help_command();
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));
    */
        /*
        // start command line
        esp_console_repl_t *repl = NULL;
        esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
        repl_config.prompt = "lunokWatch>";
        repl_config.max_cmdline_length = 255;
        esp_console_register_help_command();
        esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
        esp_console_new_repl_uart(&hw_config, &repl_config, &repl);
        esp_console_start_repl(repl);
        const esp_console_cmd_t cmd = {
            .command = "free",
            .help = "Get the current size of free heap memory",
            .hint = NULL,
            .func = &free_mem,
        };
        ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
        */

        Serial.begin(LUNOKIOT_SERIAL_SPEED);
        #ifdef LUNOKIOT_DEBUG_ESP32
            Serial.setDebugOutput(true);
            esp_log_level_set("*", ESP_LOG_VERBOSE);
        #endif
    #endif
}

// https://github.com/espressif/esp-idf/tree/cab6b6d1824e1a1bf4d965b747d75cb6a77f5151/examples/bluetooth/bluedroid/classic_bt/bt_hid_mouse_device

/* IR TEST
#include <IRremoteESP8266.h>
#include <IRsend.h>

IRsend irLed(TWATCH_2020_IR_PIN);
void shit() {
  const uint16_t rawData[] = { 0, 65535, 0, 65535 };
  irLed.begin();
  while(true) {
    irLed.sendRaw(rawData, 67, 38);  // Send a raw data capture at 38kHz.
    delay(2000);
  }
}
*/

bool LunokIoT::IsNetworkInUse() {
    //@TODO || ( IsBLEInUse() )
    if ( ( GetWiFi()->InUse() ) ) { return true; }
    return false;
}

void LunokIoT::ListLittleFS(const char *path,uint8_t spaces) {
    if ( false == LittleFSReady ) { return; }
    char * spaceStrings = (char*)ps_malloc(spaces+1);
    sprintf(spaceStrings,"%*s", spaces, " ");
    if ( 0 == strcmp("/",path) ) {
        lLog("%sLittleFS '%s'\n",spaceStrings,path);
    }
    fs::File root = LittleFS.open(path);
    if (!root) {
        lLog("%sLittleFS: ERROR: Failed to open directory '%s'\n",spaceStrings,path);
        free(spaceStrings);
        return;
    }
    if (!root.isDirectory()) {
        lLog("%sLittleFS: ERROR: not a directory\n",spaceStrings);
        free(spaceStrings);
        return;
    }
    fs::File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            lLog("%s<DIR> '%s'\n",spaceStrings,file.name());
            char * newPathToScan = (char*)ps_malloc(strlen(path)+strlen(file.name())+1);
            sprintf(newPathToScan,"%s%s",path,file.name());
            ListLittleFS(newPathToScan,spaces+5);
            free(newPathToScan);
        } else {
            lLog("%s      '%s' (%u byte)\n",spaceStrings,file.name(),file.size());
        }
        file = root.openNextFile();
    }
    if ( 0 == strcmp("/",path) ) {
        size_t totalSPIFFS = LittleFS.totalBytes();
        size_t usedSPIFFS = LittleFS.usedBytes();
        lLog("%sLittleFS (Free: %u KB)\n",spaceStrings,(totalSPIFFS-usedSPIFFS)/1024);
    }
    free(spaceStrings);
}


long int GetSecondsUntil(int hour,int minute, int second) {
    struct tm* tm;
    time_t ts=time(NULL);
    if(tm=localtime(&ts)) {
            long int delta;
            tm->tm_hour = hour;
            tm->tm_min = minute;
            tm->tm_sec = second;
            delta = mktime(tm) - ts;
            if(delta<0) { delta+=24*60*60; }
            return delta;
    }
    return 0;
}

extern uint32_t weekSteps[7];
extern uint8_t lastStepsDay;
extern uint32_t lastBootStepCount;

void LunokIoT::LogRotate() {
    if ( false == settings->IsNVSEnabled() ) {
        lEvLog("LogRotate: Unable to rotate logs due NVS is disabled\n");
        InstallRotateLogs();
        return;
    }

    time_t now;
    struct tm * tmpTime;
    time(&now);
    tmpTime = localtime(&now);

    // my weeks begins on monday not sunday
    int correctedDay = tmpTime->tm_wday-2; // use the last day recorded to save on it
    if ( correctedDay == -2 ) { correctedDay = 5; }
    else if ( correctedDay == -1 ) { correctedDay = 6; }
    int lastDate = LoT().settings->GetInt(SystemSettings::SettingKey::LastRotateLog);
    if ( tmpTime->tm_mday == lastDate ) {
        lEvLog("LogRotate: Already started on this day\n");
        InstallRotateLogs();
        return;
    }
    settings->SetInt(SystemSettings::SettingKey::LastRotateLog,tmpTime->tm_mday);
    lEvLog("LogRotate: Begin\n");
    StopDatabase();
    JournalDatabase();
    StartDatabase();
    systemDatabase->SendSQL("BEGIN TRANSACTION;");
    systemDatabase->SendSQL(queryCreateNotifications);
    systemDatabase->SendSQL(BLECreateTable);
    systemDatabase->SendSQL(queryCreateSessionRAWLog);
    systemDatabase->SendSQL("END TRANSACTION;");
    systemDatabase->Commit();

    lEvLog("LogRotate: Rotating...\n");
    
    weekSteps[correctedDay] = stepCount;
    stepCount = 0;
    lastBootStepCount = 0;
    NVS.setInt("stepCount",0,false);
    lastStepsDay = tmpTime->tm_wday; // set the last register is today in raw


    // reset all counters
    ttgo->bma->resetStepCounter();
    beginStepsBMAActivity=0; // sorry current activity is discarded
    beginBMAActivity=0;
    
    stepsBMAActivityStationary = 0; // clean all
    timeBMAActivityStationary = 0;
    stepsBMAActivityWalking = 0;
    timeBMAActivityWalking = 0;
    stepsBMAActivityRunning = 0;
    timeBMAActivityRunning = 0;
    stepsBMAActivityInvalid = 0;
    timeBMAActivityInvalid = 0;
    stepsBMAActivityNone = 0;
    timeBMAActivityNone = 0;

    for(int a=0;a<7;a++) { // Obtain the last days steps
        if ( weekSteps[a] > 0 ) {
            char keyName[16] = {0};
            sprintf(keyName,"lWSteps_%d",a);
            //lLog("SAVING: %s %d\n",keyName,weekSteps[a]);
            NVS.setInt(keyName,weekSteps[a],false);
        }
    }
    NVS.setInt("lstWeekStp",lastStepsDay,false);
    //lEvLog("LogRotate: Rotation Ends\n");

    delay(1000); // wait one second to not collide with current second
    InstallRotateLogs();
    lEvLog("LogRotate: End\n");
}

void LunokIoT::InstallRotateLogs() {
    if ( LunokIoTSystemLogRotation.active() ) {
        lSysLog("LogRotate: Already planned, re-scheduling...\n");
        LunokIoTSystemLogRotation.detach();
    }
    long int secondsUntilMidnight = GetSecondsUntil(0,0,0); //midnight
    lSysLog("LogRotate: Installed, launch planned in %d seconds\n",secondsUntilMidnight);
    // perform logrotate at midnight
    LunokIoTSystemLogRotation.once((float)secondsUntilMidnight,[]() { LoT().LogRotate(); });
}

void LunokIoT::BootReason() { // check boot status
    lEvLog("Boot reason: ");
    esp_reset_reason_t lastBootStatus = esp_reset_reason();
    if ( ESP_RST_UNKNOWN == lastBootStatus) { lLog("'Unknown'\n"); }
    else if ( ESP_RST_POWERON == lastBootStatus) { lLog("'Normal poweron'\n"); normalBoot = true; }
    else if ( ESP_RST_EXT == lastBootStatus) { lLog("'External pin'\n"); }
    else if ( ESP_RST_SW == lastBootStatus) { lLog("'Normal restart'\n"); normalBoot = true; }
    else if ( ESP_RST_PANIC == lastBootStatus) { lLog("'System panic'\n"); }
    else if ( ESP_RST_INT_WDT == lastBootStatus) { lLog("'Watchdog interrupt'\n"); }
    else if ( ESP_RST_TASK_WDT == lastBootStatus) { lLog("'Watchdog TIMEOUT'\n"); }
    else if ( ESP_RST_WDT == lastBootStatus) { lLog("'Watchdog reset'\n"); }
    else if ( ESP_RST_DEEPSLEEP == lastBootStatus) { lLog("'Recovering from deep seep'\n") normalBoot = true; fromDeepSleep=true; }
    else if ( ESP_RST_BROWNOUT == lastBootStatus) { lLog("'Brownout'\n"); }
    else if ( ESP_RST_SDIO == lastBootStatus) { lLog("'Reset over SDIO'\n"); }
    else { lLog("UNHANDLED UNKNOWN\n"); }

    if (( false == normalBoot )&&( false == fromDeepSleep )) {
        lEvLog("/!\\ /!\\ /!\\ WARNING: Last boot FAIL\n");
        SplashBootMode("Recover...");
    } else {
        #ifdef LILYGO_WATCH_2020_V3
            ttgo->shake();
        #endif
    }
    const esp_partition_t *whereIAm = esp_ota_get_boot_partition();
    lEvLog("Boot from: '%s'\n",whereIAm->label);
    // default arduinoFW esp-idf config ~/.platformio/packages/framework-arduinoespressif32/tools/sdk/esp32/include/config

    #if defined(LILYGO_WATCH_2020_V1)||defined(LILYGO_WATCH_2020_V3)
        // do sound only if boot is normal (crash-silent if last boot fail)
        if (( true == normalBoot )&&( false == fromDeepSleep )) { SplashFanfare(); } // sound and shake
    #endif
    if ( fromDeepSleep ) {
        SplashBootMode("Sleepy...");
    }
}


bool LunokIoT::CpuSpeed(uint32_t mhz) {
    lLog("@TODO @DEBUG DISABLED CPUSPEED FOR TESTING\n");
    return true;
    // force flush
    uart_wait_tx_idle_polling(UART_NUM_0);
    uint32_t currentMhz = getCpuFrequencyMhz();
    if ( mhz == currentMhz ) { return true; } // nothing to do :)
    lSysLog("CPU: Freq: %u Mhz to %u Mhz\n", currentMhz, mhz);
    #ifdef LUNOKIOT_SERIAL
            //Serial.flush();
            Serial.end();
    #endif
    esp_task_wdt_reset();
    bool res= setCpuFrequencyMhz(mhz);
    esp_task_wdt_reset();
    TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
    xTaskDelayUntil( &nextCheck, (200 / portTICK_PERIOD_MS) ); // wait a ittle bit
    esp_task_wdt_reset();
    #ifdef LUNOKIOT_SERIAL
        Serial.begin(LUNOKIOT_SERIAL_SPEED);
    #endif
    return res;
}

void LunokIoT::DestroySettings() {
    if ( nullptr != settings ) {
        delete settings;
        settings = nullptr;
    }
}


void LunokIoT::DestroyBLE() {
    if ( nullptr != ble ) {
        delete ble;
        ble = nullptr;
    }
}

LoTBLE * LunokIoT::CreateBLE() {
    LoTBLE * response = new LoTBLE();
    return response;
}


void LunokIoT::DestroyWiFi() {
    if ( nullptr != wifi ) {
        delete wifi;
        wifi = nullptr;
    }
}

LoTWiFi * LunokIoT::CreateWiFi() {
    LoTWiFi * response = new LoTWiFi();
    response->AddTask(new NTPWifiTask(rtc));
    response->AddTask(new GeoIPWifiTask());
    response->AddTask(new WeatherWifiTask());
    if ( false == NVS.getInt("WifiEnabled") ) {
        lNetLog("WiFi: %p User settings don't agree\n",this);
        return response;
    }
    // force first launch soon
    lNetLog("Starting network tasks in %.0f seconds...\n",BootWifiPerformSeconds);
    BootWifiPerform.once<LoTWiFi*>(BootWifiPerformSeconds,[](LoTWiFi* instance){
        xTaskCreatePinnedToCore([](void *obj) { // in core 0 please
            LoTWiFi* instance = (LoTWiFi*)obj;
            instance->PerformTasks();
            vTaskDelete(NULL);
        },"lwifi",LUNOKIOT_TASK_STACK_SIZE,instance,tskIDLE_PRIORITY, NULL,WIFICORE);
    },response);
    return response;
}
