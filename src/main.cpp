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
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>     // vTaskDelete
#include <Arduino.h>            // more portable than esp-idf
#include "lunokIoT.hpp"         // thats me!

void setup() { LoT(); /* thats all, folks! */ }
void loop() { vTaskDelete( NULL ); /* harakiri!!! (use apps further) */ }
