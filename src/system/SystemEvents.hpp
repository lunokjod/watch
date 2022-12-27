#ifndef __LUNOKIOT__SYSTEM_EVENTS__
#define __LUNOKIOT__SYSTEM_EVENTS__

#include <Arduino.h>
#include <esp_event_base.h>
#include <list>
#include <functional>

#include "../app/LogView.hpp"
#include <freertos/portable.h>
#include <freertos/task.h>
/*
 * Event loop for the system
 */
ESP_EVENT_DECLARE_BASE(SYSTEM_EVENTS);

// Events for system loop
enum {
    SYSTEM_EVENT_READY,       // boot end
    SYSTEM_EVENT_TICK,        // timed, meanwhile the screen is on
    SYSTEM_EVENT_STOP,        // when the system is power down
    SYSTEM_EVENT_WAKE,        //  received when the system is wake up
    SYSTEM_EVENT_LIGHTSLEEP,  // received when the system is begin sleep
    SYSTEM_EVENT_TIMER,       // ESP32 timed wakeup
    SYSTEM_EVENT_UPDATE,      // Update comes!!

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
    PMU_EVENT_REPORT,         // see pmu/axp202*

    PMU_EVENT_POWER,          // usb connected
    PMU_EVENT_NOPOWER,        // no usb
    PMU_EVENT_PEK_SHORT,      // reserved by system to sleep/wake
    PMU_EVENT_PEK_LONG,       // reserved by system to launch ordered powerdown
    PMU_EVENT_TIMER_TIMEOUT,   // @TODO some kind of timer in the AXP202
    PMU_EVENT_TEMPERATURE,    // battery temperature

    BMP_EVENT_MOVE,             // movement detected
    BMA_EVENT_STEPCOUNTER,      // step detected
    BMA_EVENT_ACTIVITY,         // activity
    BMA_EVENT_NOMOTION,         // @TODO why interrupt never received? :(
    BMA_EVENT_TEMP,             // device temperature
    BMA_EVENT_DIRECTION         // pose/orientation
};

// Force get samples from sensors
void TakeSamples();

// Build system loop
void SystemEventsStart();
// Announce the end of boot
void SystemEventBootEnd();

static inline void FreeSpace() {
    uint32_t minHeap = xPortGetMinimumEverFreeHeapSize();
    uint32_t freeHeap = xPortGetFreeHeapSize(); // portable of heap_caps_get_free_size(MALLOC_CAP_8BIT)
    size_t largestHeap = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    UBaseType_t stackMax = uxTaskGetStackHighWaterMark(NULL);
    //uint16_t stackBegin = uxTaskGetStackHighWaterMark2(NULL);

    if ( stackMax > (1024*1024) ) { lSysLog("ESP32: Free stack: %.3f MByte\n", float(stackMax/1024.0/1024.0)); }
    else if ( stackMax > 1024 ) { lSysLog("ESP32: Free stack: %.3f KByte\n", float(stackMax/1024.0)); }
    else { lSysLog("ESP32: Free stack: %u Byte\n",stackMax); }

    if ( freeHeap > (1024*1024) ) { lSysLog("ESP32: Free HEAP: %.3f MByte\n", float(freeHeap/1024.0/1024.0)); }
    else if ( freeHeap > 1024 ) { lSysLog("ESP32: Free HEAP: %.3f KByte\n", float(freeHeap/1024.0)); }
    else { lSysLog("ESP32: Free HEAP: %u Byte\n",freeHeap); }

    //lSysLog("ESP32: Stack window size: %u Bytes Range 0x%x~0x%x\n",stackMax,stackBegin,stackMax);

    if ( largestHeap > (1024*1024) ) { lSysLog("ESP32: Most allocable data (fragmentation): %.3f MByte\n", float(largestHeap/1024.0/1024.0)); }
    else if ( largestHeap > 1024 ) { lSysLog("ESP32: Most allocable data: (fragmentation) %.3f KByte\n", float(largestHeap/1024.0)); }
    else { lSysLog("ESP32: Most allocable data: (fragmentation) %u Byte\n",largestHeap); }

    bool done = heap_caps_check_integrity_all(true);
    lSysLog("ESP32: Heap integrity: %s\n",(done?"good":"WARNING: CORRUPTED!!!"));

    UBaseType_t tasksNumber = uxTaskGetNumberOfTasks();
    lSysLog("ESP32: Tasks: %u\n",tasksNumber);

    //lLog("ESP32: Free heap: %d KB\n", ESP.getFreeHeap() / 1024);
    //lLog("ESP32: Free PSRAM: %d KB\n", ESP.getFreePsram() / 1024);
/*
    size_t memcnt=esp_himem_get_phys_size();
    size_t memfree=esp_himem_get_free_size();
    lLog("ESP32: Himem has %dKiB of memory, %dKiB of which is free. Testing the free memory...\n", (int)memcnt/1024, (int)memfree/1024);



heap_caps_get_total_size
heap_caps_get_free_size
heap_caps_get_minimum_free_size
xPortGetMinimumEverFreeHeapSize
heap_caps_get_minimum_free_size()
 uxTaskGetSnapshotAll()
*/

}

/*
 * Low power related functions
 */
void DoSleep();
void SaveDataBeforeShutdown();
void BootReason();

#endif
