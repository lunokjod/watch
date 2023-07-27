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

#ifndef __LUNOKIOT__SETTINGS_STORAGE__H__
#define __LUNOKIOT__SETTINGS_STORAGE__H__

class SystemSettings {
    public:
        static const char KeysAsString[][20];
        typedef enum {
            ScreenRotation,
            Theme,
            LittleFSFormat,
            DoubleTap,
            LampGesture,
            Daylight,
            TimeZone,
            WiFi,
            NTPWiFi,
            OpenWeather,
            BLE,
            NTPBLE,
            LastRotateLog,
            WiFiCredentialsNumber
        } SettingKey;
        SystemSettings();
        ~SystemSettings();
        bool IsNVSEnabled() { return NVSReady; }
        int64_t GetInt(SettingKey what);
        bool SetInt(SettingKey what, uint8_t value);
        bool SetInt(SettingKey what, int32_t value);

    private:
        bool NVSReady = false;
};

#endif
