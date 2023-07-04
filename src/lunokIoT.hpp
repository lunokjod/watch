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

#ifndef ___lunokIoT___H____
#define ___lunokIoT___H____

#include "lunokiot_config.hpp" // WARNING: generated file, the original from "tools/lunokiot_config.hpp.template"
#include "system/Network/BLE.hpp"
#include "system/Network/WiFi.hpp"
#include "system/Network.hpp"
#include "system/Storage/Settings.hpp"
#include "system/Storage/vfs.hpp"

// almost empty defines to clarify the paramethers direction (see declarations below)
#define IN const // clarify to developer if the data is input or output
#define OUT
#define INOUT
// other convenient syntactic-sugar
#define NO_CALLBACK nullptr
#define IGNORE_PARAM void *unused

void ReverseBubbleSort(int arr[], int n);

// LoT is all you need for use it! ;)
#define LoT LunokIoT::Get

// yep... a f*kng singleton, you know C++11 stylish
class LunokIoT {
    public:
        static LunokIoT& Get() {
            static LunokIoT _myInstance;
            return _myInstance;
        }
        void LogRotate();
        long int GetSecondsUntilHour(int hour=0,int minute=0, int second=0);
        void DestroyOldFiles();
        void CleanCache(const char *path="/_cache");
        void ListLittleFS(const char *path="/",uint8_t spaces=1);
        bool IsLittleFSEnabled();
        void BootReason();
        bool IsNetworkInUse();
        bool CpuSpeed(IN uint32_t mhz);

        SystemSettings * GetSettings() {
            if ( nullptr == settings ) {  settings = new SystemSettings(); }
            return settings;
        }
        // network things
        void DestroyWiFi();
        LoTWiFi * CreateWiFi();
        LoTWiFi * GetWiFi() {
            if ( nullptr == wifi ) {  wifi = CreateWiFi(); }
            return wifi;
        }

        void DestroyBLE();
        LoTBLE * CreateBLE();
        LoTBLE * GetBLE() {
            if ( nullptr == ble ) {  ble = CreateBLE(); }
            return ble;
        }
        void DestroySettings();
        
        // delete copy and move constructors and assign operators
        LunokIoT(LunokIoT const&) = delete;             // Copy construct
        LunokIoT(LunokIoT&&) = delete;                  // Move construct
        LunokIoT& operator=(LunokIoT const&) = delete;  // Copy assign
        LunokIoT& operator=(LunokIoT &&) = delete;      // Move assign
        bool LittleFSReady=false;
        size_t selectedWatchFace=0;
    protected:
        LunokIoT();
        ~LunokIoT() {}; // implemented here ¿what kind of singleton.... x'D
    private:
        const float BootWifiPerformSeconds = 10; // time to first wifi tasks launch
        Ticker BootWifiPerform; // used to launch tasks on first boot faster than "one cycle"
        LoTWiFi * wifi=nullptr; // used internally
        LoTBLE * ble=nullptr;
        SystemSettings * settings=nullptr;
        bool normalBoot = false;
        bool fromDeepSleep = false;
        //bool NVSReady=false;
        void InitLogs();
        void InstallRotateLogs();
        Ticker LunokIoTSystemLogRotation;
        Ticker LunokIoTSystemLogRotationByDeepSleep;
};

#endif
