#ifndef ___LUNOKIOT__SYSTEM__KVO__BASE___
#define ___LUNOKIOT__SYSTEM__KVO__BASE___

#include <functional>

typedef std::function<void ()> KVOCallback;

class EventKVO {
    public:
        KVOCallback callback;
        int event;
        EventKVO(KVOCallback callback, int SystemEvent);
        ~EventKVO();
};

#endif
