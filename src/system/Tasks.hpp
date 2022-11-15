#ifndef __LUNOKIOT__TASKS__SCHEDULER__
#define __LUNOKIOT__TASKS__SCHEDULER__
#include <Arduino.h>
#include <functional>

class TimedTaskDescriptor {
    public:
        bool enabled=true;
        char *name = nullptr; // task description
        unsigned long everyTimeMS = -1; // lapse in millis
        unsigned long _nextTrigger = -1; // -1 means first launch in next time window or 0 for inmediate
        unsigned long _lastCheck = -1;  // reserved
        std::function<bool ()> callback = nullptr; // lambda power
};

// Install the network loop task scheduler
bool TimedTaskHandler();
// Remove desired task from network pooling
bool RemoveTimedTask(TimedTaskDescriptor *oldTsk);
// Add task to the network loop
bool AddTimedTask(TimedTaskDescriptor *nuTsk);

#endif
