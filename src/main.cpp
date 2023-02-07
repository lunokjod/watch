/*
 * Hi!!! any developer, issuer, ideas, tips are welcome!!!
 *
 * Note: If you want get ArduinoFW compatible code, try to use Applications, have a "setup()" and "loop()" equivalences
 * More info: https://github.com/lunokjod/watch/blob/devel/src/app/README.md#develop-new-application
 *
 */

// convenient debug from espressif
#ifdef LUNOKIOT_DEBUG_ESP32

#ifdef CORE_DEBUG_LEVEL
#undef CORE_DEBUG_LEVEL
#endif

#ifdef LOG_LOCAL_LEVEL
#undef LOG_LOCAL_LEVEL
#endif

#define CORE_DEBUG_LEVEL 0
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#endif

#include <Arduino.h>           // more portable than esp-idf
#include <freertos/task.h>     // vTaskDelete
#include <esp_log.h>
#include <esp_task_wdt.h>

#include "lunokIoT.hpp"

void setup() {
    LoT(); // thats all, folks! 
}
extern TaskHandle_t BMAInterruptControllerHandle;
extern TaskHandle_t AXPInterruptControllerHandle;

void loop() {
    /*
    if ( nullptr == BMAInterruptControllerHandle ) {
        Serial.printf("@TODO BMAInterruptControllerHandle DOWN!!!!\n");
    }
    if ( nullptr == AXPInterruptControllerHandle ) {
        Serial.printf("@TODO AXPInterruptControllerHandle DOWN!!!!\n");
    }
    esp_task_wdt_reset();
    */
    vTaskDelete( NULL );
}
