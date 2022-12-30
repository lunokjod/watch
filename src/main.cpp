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

#include "lunokIoT.hpp"

void setup() {
    LoT(); // thats all, folks! 
}
void loop() { vTaskDelete( NULL ); }
