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


EventUIKVO::EventUIKVO(KVOCallback callback, int UIEvent) : callback(callback), event(UIEvent) {
    esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, event, StaticKVOCallback, (void *)this, &instance);
    lEvLog("UIKVO: 0x%p attaching to event: %d\n",this,event);
}

EventUIKVO::~EventUIKVO() {
    esp_event_handler_instance_unregister_with(uiEventloopHandle,UI_EVENTS, event,instance);
    lEvLog("UIKVO: 0x%p stop watching event: %d\n",this,event);
}
