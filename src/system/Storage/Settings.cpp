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
#include <ArduinoNvs.h>
#include "../../app/LogView.hpp"

#include "Settings.hpp"

const char SystemSettings::KeysAsString[][20] = {"ScreenRot", "lWTheme","littleFSReady","doubleTap",
                                "lampGesture","summerTime","timezoneTime",
                                "WifiEnabled","NTPEnabled","OWeatherEnabled",
                                "BLEEnabled","NTPBLEEnabled","LastRotateLog","WiFiCredNo" };

SystemSettings::SystemSettings() {
    NVSReady = NVS.begin(); // need NVS to get the current settings
    lSysLog("SystemSettings %s\n",(NVSReady?"ready":"ERROR"));
}

SystemSettings::~SystemSettings() {
    if ( NVSReady ) {
        lSysLog("SystemSettings closing\n");
        NVS.commit();
        NVSReady=false;
        NVS.close();
    }
}

int64_t SystemSettings::GetInt(SettingKey what) { 
    int64_t result = NVS.getInt(KeysAsString[what]);
    lSysLog("SystemSettings %p: Get '%s' as: (signed: %d) (unsigned: %u)\n", this,KeysAsString[what],result,result);
    return result;
}

bool SystemSettings::SetInt(SettingKey what, int32_t value) {
    lLog("SystemSettings: DEBUG INT32\n");
    bool result = NVS.setInt(String(KeysAsString[what]),value,false);
    if (false == result) {
        lSysLog("SystemSettings %p: ERROR: Unable to set key '%s'\n", this,KeysAsString[what]);
    }
    return result;
}

bool SystemSettings::SetInt(SettingKey what, uint8_t value) {
    lLog("SystemSettings: DEBUG UINT8\n");
    bool result = NVS.setInt(String(KeysAsString[what]),value,false);
    if (false == result) {
        lSysLog("SystemSettings %p: ERROR: Unable to set key '%s'\n", this,KeysAsString[what]);
    }
    return result;
}
