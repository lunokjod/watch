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

#include <Arduino.h>
//#include <LilyGoWatch.h>
//#include "../lunokIoT.hpp"
#include "Network.hpp"

//#include <functional>
//#include <list>

//#include "lunokiot_config.hpp"
//#include "SystemEvents.hpp"

//#include <ArduinoNvs.h> // persistent values
//#include <WiFi.h>
//#include "UI/UI.hpp" // for rgb unions
//#include <HTTPClient.h>


//#include "Datasources/kvo.hpp"
//#include "../app/LogView.hpp"
//#include <esp_task_wdt.h>
//#include "Datasources/database.hpp"
#include "Network/WiFi.hpp"
#include "Network/BLE.hpp"

// https://stackoverflow.com/questions/44951078/when-how-is-a-ble-gatt-notify-indicate-is-send-on-physical-layer
extern char * latestBuildFoundString;
//#include "../app/OTAUpdate.hpp"
//#include <Ticker.h>
extern bool provisioned;
//extern TFT_eSprite *screenShootCanvas;
//extern TFT_eSprite *screenShootCanvas;    // the last screenshoot
//extern bool screenShootInProgress;        // Notify Watchface about send screenshoot
//size_t imageUploadSize=0;
//size_t realImageSize=0;
//rgb565 myLastColor = { .u16 = (uint16_t)TFT_TRANSPARENT }; // improbable color
//uint16_t lastRLECount = 0;
#ifdef LUNOKIOT_UPDATES_LOCAL_URL
// Use alternative keys
extern const PROGMEM uint8_t githubPEM_start[] asm("_binary_asset_server_pem_start");
extern const PROGMEM uint8_t githubPEM_end[] asm("_binary_asset_server_pem_end");
#else
extern const PROGMEM uint8_t githubPEM_start[] asm("_binary_asset_raw_githubusercontent_com_pem_start");
extern const PROGMEM uint8_t githubPEM_end[] asm("_binary_asset_raw_githubusercontent_com_pem_end");
#endif
extern const PROGMEM uint8_t openweatherPEM_start[] asm("_binary_asset_openweathermap_org_pem_start");
extern const PROGMEM uint8_t openweatherPEM_end[] asm("_binary_asset_openweathermap_org_pem_end");
extern const PROGMEM char openWeatherMapApiKey[];

//@TODO convert this to  system event
extern bool ntpSyncDone;
extern bool weatherSyncDone;

/*
float ReverseFloat( const float inFloat )
{
   float retVal;
   char *floatToConvert = ( char* ) & inFloat;
   char *returnFloat = ( char* ) & retVal;

   // swap the bytes into a temporary buffer
   returnFloat[0] = floatToConvert[3];
   returnFloat[1] = floatToConvert[2];
   returnFloat[2] = floatToConvert[1];
   returnFloat[3] = floatToConvert[0];

   return retVal;
}
*/


/*
//@TODO DEBUG TASK
class FakeWiFiTask: public LoTWiFiTask {
    private:
        bool allow=true;
    public:
        void Launch() override {
            lLog("FakeWiFiTask %p LAUNCH EVENT\n");
            allow=false;
        }
        bool Check() override {
            lLog("FakeWiFiTask %p CHECK EVENT\n");
            return allow;
        }
        ~FakeWiFiTask() override {};
        const char * Name() override { return "Fake task!"; }
};*/

extern int weatherId;
extern char *weatherMain;
extern char *weatherDescription;
extern char *weatherIcon;
extern double weatherTemp;
// Extra Weather variables
extern double weatherFeelsTemp;
extern double weatherTempMin;
extern double weatherTempMax;
extern double weatherHumidity;
extern double weatherPressure;
extern double weatherWindSpeed;


extern char *geoIPReceivedData;
extern char *weatherReceivedData;

extern char *weatherCity;
extern char *weatherCountry;


//extern PCF8563_Class * rtc;


