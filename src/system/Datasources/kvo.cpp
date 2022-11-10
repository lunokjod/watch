#include "kvo.hpp"
#include <functional>
#include "../SystemEvents.hpp"

#include <esp_event.h>
#include <esp_event_base.h>
#include "../../app/LogView.hpp"
// Event loops
extern esp_event_loop_handle_t systemEventloopHandler;

static void StaticKVOCallback(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    EventKVO *self = reinterpret_cast<EventKVO*>(handler_args);
    if( nullptr ==  self ) {
        lEvLog("KVO: Unable to call to empty KVO!\n");
        return;
    }
    self->callback();
}

EventKVO::EventKVO(KVOCallback callback, int SystemEvent) : callback(callback), event(SystemEvent) {
    esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, event, StaticKVOCallback, (void *)this, &instance);
    lEvLog("KVO: 0x%p attaching to event: %d\n",this,event);
}

EventKVO::~EventKVO() {
    esp_event_handler_instance_unregister_with(systemEventloopHandler,SYSTEM_EVENTS, event,instance);
    lEvLog("KVO: 0x%p stop watching event: %d\n",this,event);
}
