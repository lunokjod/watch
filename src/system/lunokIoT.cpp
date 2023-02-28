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
#include <SPIFFS.h>
#include <ArduinoNvs.h>

#include <cstdio>
//#include <LilyGoWatch.h>

//#include <SPI.h>
//#include <FS.h>

#include "../lunokIoT.hpp"
#include "../lunokiot_config.hpp"
#include "UI/BootSplash.hpp"
#include "SystemEvents.hpp"
#include "Datasources/database.hpp"
#include "Datasources/perceptron.hpp"
#include "PMU/AXP202.hpp"
#include "system/Network.hpp"

#include "app/Steps.hpp" // Step manager
#include "app/LogView.hpp" // log

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
extern SemaphoreHandle_t I2cMutex;

TTGOClass *ttgo = TTGOClass::getWatch();
//TFT_eSPI * tft = nullptr;
//TFT_eSPI * tft = new TFT_eSPI();

extern bool ntpSyncDone;
extern const char *ntpServer;

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

bool LunokIoT::IsNVSEnabled() { return NVSReady; }
bool LunokIoT::IsSPIFFSEnabled() { return SPIFFSReady; }
#include <esp_console.h>

LunokIoT::LunokIoT() {
    int64_t beginBootTime = esp_timer_get_time(); // stats!!
    
    InitLogs(); // need for stdout on usb-uart

    // announce myself with build information if serial debug is enabled
    lSysLog("'компаньон' #%d//%s//\n",LUNOKIOT_BUILD_NUMBER,LUNOKIOT_KEY);
    
    // report memory
    FreeSpace();
    
    // Initialize lilygo lib (mandatory at this time)
    ttgo->begin();

    //ttgo->setTftExternal(*tft); // @TODO dream on!
    
    // storage init
    SPIFFSReady = SPIFFS.begin(); // needed for SQLite activity database and other blobs
    NVSReady = NVS.begin(); // need NVS to get the current settings
    lSysLog("Storage: NVS: %s, SPIFFS: %s\n", (NVSReady?"yes":"NO"), (SPIFFSReady?"yes":"NO"));

    uint8_t rotation = NVS.getInt("ScreenRot"); // get screen rotation user select from NVS
    //lUILog("User screen rotation: %d\n", rotation);
    ttgo->tft->setRotation(rotation); // user selected rotation (0 by default)

    size_t themeOffset = NVS.getInt("lWTheme"); // load current theme offset
    currentColorPalette = &AllColorPaletes[themeOffset]; // set the color palette (informative)
    currentThemeScheme = &AllThemes[themeOffset]; // set the GUI colors

    SplashAnnounce(); // simple eyecandy meanwhile boot (themed)

    bool alreadyFormattedSPIFFS = NVS.getInt("spiffsReady"); // get special key from NVS
    if ( false == alreadyFormattedSPIFFS ) {
        lSysLog("SPIFFS: user wants format disk\n");
        SPIFFSReady=false; // mark as clean forced
    }
    // format SPIFFS if needed
    if ( false == SPIFFSReady ) {
        lSysLog("SPIFFS: Format SPIFFS....\n");        
        SplashFormatSPIFFSAnnounce();
        SPIFFSReady = SPIFFS.format();
        if ( false == SPIFFSReady ) {
            lSysLog("SPIFFS: ERROR: Unable to format!!!\n");
            SPIFFSReady=false;
        } else {
            SPIFFSReady = SPIFFS.begin(); // mount again
            NVS.setInt("spiffsReady",true); // assume format reached and disable it in next boot
        }
    }
    // banner storages again
    lSysLog("Storage: NVS: %s, SPIFFS: %s\n", (NVSReady?"yes":"NO"), (SPIFFSReady?"yes":"NO"));
    ListSPIFFS(); // show contents to serial
    /*
    esp_err_t taskStatus = esp_task_wdt_status(NULL);
    if ( ESP_ERR_INVALID_STATE != taskStatus ) {
        esp_task_wdt_deinit();
        lEvLog(" TWDT enabled by arduinoFW???\n");
    }*/
    
    // https://github.com/espressif/esp-idf/blob/9ee3c8337d3c4f7914f62527e7f7c78d7167be95/examples/system/task_watchdog/main/task_watchdog_example_main.c
    esp_err_t taskWatchdogResult = esp_task_wdt_init(CONFIG_ESP_TASK_WDT_TIMEOUT_S, true); // CONFIG_ESP_TASK_WDT_TIMEOUT_S
    if ( ESP_OK != taskWatchdogResult ) {
        lEvLog("ERROR: Unable to start Task Watchdog\n");
        FreeSpace();
    }
    BootReason(); // announce boot reason and from what partition to serial

    /* useless on arduinofw
    //#ifndef CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU0
    taskWatchdogResult = esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(0));
    if ( ESP_OK != taskWatchdogResult ) {
        //  maybe due the ArduinoFW esp-idf sdk see:
        // '.platformio/packages/framework-arduinoespressif32/tools/sdk/esp32/include/config/sdkconfig.h'
        lEvLog("WARNING: Unable to start Task Watchdog on PRO_CPU(0)\n");
    }*/
    //#endif
    
    //#if CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU1 && !CONFIG_FREERTOS_UNICORE
    taskWatchdogResult = esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(1));
    if ( ESP_OK != taskWatchdogResult ) {
        lEvLog("WARNING: Unable to start Task Watchdog on APP_CPU(1)\n");
    }
    //#endif

    #ifdef LILYGO_WATCH_2020_V3
    ttgo->motor_begin(); // start the motor for haptic notifications
    #endif

    lSysLog("System initializing...\n");

    // get if are in summertime
    int daylight = NVS.getInt("summerTime");
    // get the GMT timezone
    long timezone = NVS.getInt("timezoneTime");
    // announce values to log
    lEvLog("RTC: Config: Summer time: '%s', GMT: %+d\n",(daylight?"yes":"no"),timezone);
    configTime(timezone*3600, daylight*3600, ntpServer); // set ntp server query
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
        }
        xSemaphoreGive(I2cMutex);
    }

    StartDatabase(); // must be started after RTC sync (timestamped inserts need it to be coherent)

    SplashAnnounceBegin(); // anounce user about boot begins

    userTall= NVS.getInt("UserTall");        // get how high is the user?
    if ( 0 == userTall ) { userTall = 120; } // almost midget value
    
    userMaleFemale = NVS.getInt("UserSex");  // maybe is better use "hip type A/B" due to no offend any gender
    if ( true == userMaleFemale ) { stepDistanceCm = userTall * WOMAN_STEP_PROPORTION; }
    else { stepDistanceCm = userTall * MAN_STEP_PROPORTION; }

    // Build the lunokiot message bus
    SystemEventsStart();
    // Start the interface with the user via the screen and buttons!!! (born to serve xD)
    UIStart();
    NetworkHandler();   // already provisioned? start the network timed tasks loop
    InstallStepManager();
    StartPMUMonitor();
    selectedWatchFace=NVS.getInt("UWatchF");
    if ( selectedWatchFace >= WatchFacesAvailiable() ) {
        selectedWatchFace=0; // reset to defaults if out of range
    }
    SplashAnnounceEnd();
    SystemEventBootEnd(); // notify to system end boot procedure (SystemEvents must launch watchface here)
    FreeSpace();

    int64_t endBootTime = esp_timer_get_time()-beginBootTime;
    lSysLog("loaded in %lld us\n",endBootTime);
};
/*
static int free_mem(int argc, char **argv)
{
    printf("%d\n", esp_get_free_heap_size());
    return 0;
}*/

void LunokIoT::InitLogs() {
    // Get serial comms with you
    #ifdef LUNOKIOT_SERIAL
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
extern bool wifiOverride;
bool LunokIoT::IsNetworkInUse() {
    if ( ( wifiOverride ) || ( IsBLEInUse() )) { return true; }
    return false;
}
void LunokIoT::ListSPIFFS() {
    if ( false == SPIFFSReady ) { return; }
    lSysLog("SPIFFS: contents:\n");
    File root = SPIFFS.open("/");
    if (!root) {
        lLog("SPIFFS: ERROR: Failed to open directory\n");
        return;
    }
    if (!root.isDirectory()) {
        lLog("SPIFFS: ERROR: not a directory\n");
        return;
    }
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            lSysLog("<DIR> '%s'\n",file.name());
        } else {
            lSysLog("      '%s' (%u byte)\n",file.name(),file.size());
        }
        file = root.openNextFile();
    }
    size_t totalSPIFFS = SPIFFS.totalBytes();
    size_t usedSPIFFS = SPIFFS.usedBytes();
    lSysLog("SPIFFS (Free: %u KB)\n",(totalSPIFFS-usedSPIFFS)/1024);
}


void LunokIoT::BootReason() { // check boot status
    bool normalBoot = false;
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
    else if ( ESP_RST_DEEPSLEEP == lastBootStatus) { lLog("'Recovering from deep seep'\n") normalBoot = true; }
    else if ( ESP_RST_BROWNOUT == lastBootStatus) { lLog("'Brownout'\n"); }
    else if ( ESP_RST_SDIO == lastBootStatus) { lLog("'Reset over SDIO'\n"); }
    else { lLog("UNHANDLED UNKNOWN\n"); }

    if ( false == normalBoot ){
        lEvLog("/!\\ /!\\ /!\\ WARNING: Last boot FAIL\n");
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
        if ( true == normalBoot ) { SplashFanfare(); } // sound and shake
    #endif
}


bool LunokIoT::CpuSpeed(uint32_t mhz) {
    uint32_t currentMhz = getCpuFrequencyMhz();
    if ( mhz == currentMhz ) { return true; } // nothing to do :)
    lSysLog("CPU: Freq: %u Mhz to %u Mhz\n", currentMhz, mhz);
    #ifdef LUNOKIOT_SERIAL
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
