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

#ifndef ___LUNOKIOT__WIFI__HANDLING___
#define ___LUNOKIOT__WIFI__HANDLING___
#include <Arduino.h>

#include <freertos/semphr.h>
#include <WiFiMulti.h>
#include <Ticker.h>
#include <functional>
#include <list>

const uint8_t WIFICORE = 0;

// define a task for wifi
class LoTWiFiTask {
    public:
        virtual void Launch()=0; // force launch now
        virtual bool Check()=0; // return if mus be Launched
        virtual ~LoTWiFiTask() {};
        virtual const char * Name() { return "NOT SET"; }
};

class LoTWiFiCredentials {
    public:
        char * SSID=nullptr;
        char * Password=nullptr;
};

// define wifi hanlder
class LoTWiFi {
    private:
        std::list<LoTWiFiCredentials*> credentials = {};
        const float GraceTimeSeconds=5;
        const unsigned long ConnectionTimeoutMs = 20*1000;
        const float HeartBeatTime = 5*60; // time in seconds to check pending tasks
        SemaphoreHandle_t taskLock = xSemaphoreCreateMutex();
        std::list<LoTWiFiTask *> tasks = {};
        bool enabled=false;
        bool running=false;
        void InstallTimer();
        WiFiMulti wifiMulti;
    public:
        bool Provisioned();
        void AddConnection(const char *SSID, const char *password); 
        size_t GetConnections();
        char * GetSSID(size_t offset=0);
        char * GetPWD(size_t offset=0);
        const float GraceTime() { return GraceTimeSeconds; }
        Ticker NetworkHeartbeatTicker;
        // allow/disallow wifi tasks
        void Enable();
        void Disable();
        bool IsEnabled();
        bool IsDisabled() { return (!IsEnabled()); }
        bool InUse();
        bool RadioInUse();
        // must be called from core 0 to be functional
        void PerformTasks();
        LoTWiFi();
        ~LoTWiFi();
        bool AddTask(LoTWiFiTask * newTask);
        bool RemoveTask(LoTWiFiTask * newTask);
};

#endif
