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

#ifndef __LUNOKIOT__SYSTEM_EVENTS__
#define __LUNOKIOT__SYSTEM_EVENTS__

#include <Arduino.h>
#include <esp_event_base.h>
#include <list>
#include <functional>

#include "../app/LogView.hpp"
#include <freertos/portable.h>
#include <freertos/task.h>
#include <esp32/himem.h>

const uint8_t SYSTEMCORE = 1;
const UBaseType_t SYSTEMPRIORITY = tskIDLE_PRIORITY+2;
const uint8_t INTHANDLERSCORE = SYSTEMCORE;
const UBaseType_t INTHANDLERPRIORITY = tskIDLE_PRIORITY+12;
/*
 * Event loop for the system
 */
ESP_EVENT_DECLARE_BASE(SYSTEM_EVENTS);

// Events for system loop
enum {
    SYSTEM_EVENT_READY,       // boot ends (only after boot, for cyclic call see SYSTEM_EVENT_WAKE)
    SYSTEM_EVENT_TICK,        // periodical, meanwhile the screen is on
    SYSTEM_EVENT_STOP,        // when the system is power down
    SYSTEM_EVENT_WAKE,        // received when the system is wake up
    SYSTEM_EVENT_LIGHTSLEEP,  // received when the system is begin sleep
    SYSTEM_EVENT_TIMER,       // ESP32 timed wakeup
    SYSTEM_EVENT_UPDATE,      // Update comes!!
    SYSTEM_EVENT_LOWMEMORY,   // All apps receive this call when system memory goes low
    SYSTEM_EVENT_TEMP,        // Announche the current CPU temperature changed

    PMU_EVENT_BATT_CHARGING,  // notify when the battery is charging
    PMU_EVENT_BATT_FULL,      // the batt is full
    PMU_EVENT_BATT_ACTIVE,    // the battery becomes active
    PMU_EVENT_BATT_FREE,      // battery freed
    PMU_EVENT_BATT_PRESENT,   // have battery? 
    PMU_EVENT_BATT_REMOVED,   // ups, battery removed!
    PMU_EVENT_BATT_TEMP_LOW,  // Help! battery can be damaged!
    PMU_EVENT_BATT_TEMP_HIGH, // o_Ou
    PMU_EVENT_BATT_NOTFOUND,  // no battery
    PMU_EVENT_BATT_PC,        // battery load percent
    PMU_EVENT_REPORT,         // see pmu/axp202* monitor

    PMU_EVENT_POWER,          // usb connected
    PMU_EVENT_NOPOWER,        // no usb
    PMU_EVENT_PEK_SHORT,      // used by system to sleep/wake
    PMU_EVENT_PEK_LONG,       // used by system to launch ordered powerdown
    PMU_EVENT_TIMER_TIMEOUT,  // @TODO some kind of timer in the AXP202
    PMU_EVENT_TEMPERATURE,    // battery temperature

    BMP_EVENT_MOVE,             // movement detected
    BMA_EVENT_STEPCOUNTER,      // step detected
    BMA_EVENT_ACTIVITY,         // activity
    BMA_EVENT_NOMOTION,         // @TODO why interrupt never received? :(
    BMA_EVENT_TEMP,             // device temperature
    BMA_EVENT_DIRECTION,        // pose/orientation
    BMA_EVENT_TILT,             // watch tilt
    BMA_EVENT_DOUBLE_TAP        // tap twice on watch
};
// obtain data from all sensors
void TakeAllSamples();

// Build system loop
void SystemEventsStart();
// Announce the end of boot
void SystemEventBootEnd();
void SystemBMARestitution();

static inline void FreeSpace() {
    uint32_t minHeap = xPortGetMinimumEverFreeHeapSize();
    uint32_t freeHeap = xPortGetFreeHeapSize(); // portable of heap_caps_get_free_size(MALLOC_CAP_8BIT)
    size_t largestHeap = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    uint32_t espHeap = ESP.getFreeHeap();
    UBaseType_t stackMax = uxTaskGetStackHighWaterMark(NULL);
    //uint16_t stackBegin = uxTaskGetStackHighWaterMark2(NULL);
    uint32_t usedHeap = freeHeap-largestHeap;
    float fragmentation=(float(usedHeap)/float(freeHeap))*100.0;
    // himem
    //size_t memcnt=esp_himem_get_phys_size();
    //size_t memfree=esp_himem_get_free_size();

    if ( stackMax > (1024*1024) ) { lSysLog("ESP32: Free stack: %.2f MByte\n", float(stackMax/1024.0/1024.0)); }
    else if ( stackMax > 1024 ) { lSysLog("ESP32: Free stack: %.2f Kb\n", float(stackMax/1024.0)); }
    else { lSysLog("ESP32: Free stack: %u Byte\n",stackMax); }

    if ( espHeap > (1024*1024) ) { lSysLog("ESP32: Internal Free heap: %.2f MByte\n", float(espHeap/1024.0/1024.0)); }
    else if ( espHeap > 1024 ) { lSysLog("ESP32: Internal Free heap: %.2f Kb\n", float(espHeap/1024.0)); }
    else { lSysLog("ESP32: Internal Free heap: %u Byte\n",espHeap); }


    if ( freeHeap > (1024*1024) ) { lSysLog("ESP32: Free heap: %.2f MByte\n", float(freeHeap/1024.0/1024.0)); }
    else if ( freeHeap > 1024 ) { lSysLog("ESP32: Free heap: %.2f Kb\n", float(freeHeap/1024.0)); }
    else { lSysLog("ESP32: Free heap: %u Byte\n",freeHeap); }

    //lSysLog("ESP32: Stack window size: %u Bytes Range 0x%x~0x%x\n",stackMax,stackBegin,stackMax);

    if ( usedHeap > (1024*1024) ) { lSysLog("ESP32: Heap hole: %.2f MByte\n", float(usedHeap/1024.0/1024.0)); }
    else if ( usedHeap > 1024 ) { lSysLog("ESP32: Heap hole: %.2f Kb\n", float(usedHeap/1024.0)); }
    else { lSysLog("ESP32: Heap hole: %u Byte\n",usedHeap); }
    
    if ( largestHeap > (1024*1024) ) { lSysLog("ESP32: Most allocable data: %.2f MByte (fragmentation: %.1f%%%%)\n",float((largestHeap/1024.0)/1024.0),fragmentation); }
    else if ( largestHeap > 1024 ) { lSysLog("ESP32: Most allocable data: %.2f Kb (fragmentation: %.1f%%%%)\n", float(largestHeap/1024.0),fragmentation); }
    else { lSysLog("ESP32: Most allocable data: %lu Byte (fragmentation: %.1f%%%%)\n",largestHeap,fragmentation); }

    //lSysLog("ESP32: Himem has %dKiB of memory, %dKiB of which is free.\n", (int)memcnt/1024, (int)memfree/1024);

    bool done = heap_caps_check_integrity_all(true);
    lSysLog("ESP32: Heap integrity: %s\n",(done?"good":"WARNING: CORRUPTED!!!"));

    UBaseType_t tasksNumber = uxTaskGetNumberOfTasks();
    lSysLog("ESP32: Tasks: %u\n",tasksNumber);
    /*
    uint32_t timeIDLE = ulTaskGetIdleRunTimeCounter();
    lSysLog("ESP32: Tasks: %u IDLE: %u\n",tasksNumber,timeIDLE);
    */
    /*
    char * pcBuffer = (char*)ps_malloc(40*50);
    vTaskGetRunTimeStats(pcBuffer);
    lLog(pcBuffer);
    free(pcBuffer);
    */
    /*
    size_t memcnt=esp_himem_get_phys_size();
    size_t memfree=esp_himem_get_free_size();
    lLog("ESP32: Himem has %dKiB of memory, %dKiB of which is free. Testing the free memory...\n", (int)memcnt/1024, (int)memfree/1024);
    */
// uxTaskGetSnapshotAll()

}

/*
 * Low power related functions
 */
void DoSleep();
void SaveDataBeforeShutdown();
void BootReason();

#endif
