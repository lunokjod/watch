#ifndef ___lunokIoT___H____
#define ___lunokIoT___H____

#include "lunokiot_config.hpp" // WARNING: generated file, the original from "tools/lunokiot_config.hpp.template"


// LoT is all you need for use it! ;)
#define LoT LunokIoT::Get

// yep... a f*kng singleton, you know C++11 stylish
class LunokIoT {
    public:
        static LunokIoT& Get() {
            static LunokIoT _myInstance;
            return _myInstance;
        }
        void ListSPIFFS();
        bool IsSPIFFSEnabled();
        bool IsNVSEnabled();
        //void ListSPIFFS();
        // delete copy and move constructors and assign operators
        LunokIoT(LunokIoT const&) = delete;             // Copy construct
        LunokIoT(LunokIoT&&) = delete;                  // Move construct
        LunokIoT& operator=(LunokIoT const&) = delete;  // Copy assign
        LunokIoT& operator=(LunokIoT &&) = delete;      // Move assign
    protected:
        LunokIoT();
        ~LunokIoT() {}; // implemented here ¿what kind of singleton.... x'D
    private:
        bool SPIFFSReady=false;
        bool NVSReady=false;
        void InitLogs();
};

#endif
