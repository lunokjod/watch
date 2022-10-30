#ifndef ___LUNOKIOT__SYSTEM__KVO__BASE___
#define ___LUNOKIOT__SYSTEM__KVO__BASE___

#include <esp_event.h>
#include <esp_event_base.h>

#include <functional>

typedef std::function<void ()> KVOCallback;

class EventKVO {
    public:
        esp_event_handler_instance_t instance;
        KVOCallback callback;
        int event;
        EventKVO(KVOCallback callback, int SystemEvent);
        ~EventKVO();
};

#endif
