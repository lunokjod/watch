# lilyGo TWatch 2020 lunokIoTWatch firmware

![wartchface0](/doc/watchface0.png)
![wartchface0_day](/doc/day_watchface0.png)
![playground example0](/doc/playground0.png)
![playground example3](/doc/playground3.png)
![set time app](/doc/settime.png)
![battery app](/doc/battery.png)
![ble upload](/doc/ble_upload.png)
![provisioning menu](/doc/mainmenu_provisioning.png)
![provisioning app](/doc/provisioning.png)

* Note: under heavy development (not ready for everyday use)
* On first launch must set WiFi credentials using QR (WiFI only at this time) using android device: https://play.google.com/store/apps/details?id=com.espressif.provsoftap you can bypass disabling -DLUNOKIOT_WIFI_ENABLED if don't want wifi features (NTP+weather+localcloud)

## Basic usage:

* Use button to wake/sleep the watch
* When screen goes on, the watchface appears
* Long button push for poweroff

## What does:

* WiFi time sync once every 24h (using network time protocol NTP)
* WiFi weather sync every 30 minutes, needs online account (set your openWeather key in "openWeatherKey.txt" at same level as platformio.ini before build) more info: https://openweathermap.org/
* Wake up every 60 seconds to monitor the user pose (future sleep monitor)
* Wake up every activity/stepcounter (future fitness monitor)
* Screen capture can be shared via BLE UART (see tools/getScreenshoot.py)

## Full application list:

[See App gallery](src/app/README.md)

## Hardware support:
 * BMA423 support (including during sleep)
 * AXP202 support (including sleep)
 * haptic and vibration support
 * ST7789V support (lilygo TFT_eSPI)
 * Partial RTC support (system sync)
 * MAX98357A (mp3 playback support)
 * WiFi (via provisioning)
 * BLE (work in progress) issues while wifi is in use
 * IR work, but only can send with this hardware (useless by now, I want to do jokes!)

### Pending to implement:
* Add first launch application assistant to set or omit the desired setup parts
  * the mandatory wifi sync on first launc must be optional (Lohtse shar)
  * option for flip screen (Lohtse shar)
* PDM Mic (voice assistant)
* RTC (alarm)
* espNow (locate other family watches)
* Touch on sleep
* Custom activities (hidden app availiable for testing)
* OTA shit
* feedbacked apps (what do you need?)
* LocalCloud, a support app for the watch (companion app or something)
* Web installer: https://espressif.github.io/esptool-js/
* TFT low-power minimal service (no backligt+slow bus clock), only black and white, trying to simulate eINK (playground7)
 * User customizable night hours, during this time, the system don't allow to reply anything inmediatly(delayed send guarantees don't send stupid messages to others by accident)
 * Menú must remember the last used or better: reorder the list with user preferences (app usage counters)
 * viewport
 * animations
 * hive mode (wireless mesh, share info with other lunokIoT, without infrastructure)
## Important build flags
 platform.ini file contains the build flags, the more relevant are:
 * **-DLILYGO_WATCH_2020_V3** change with your version (lilygo library driven) please report issues :)
* **-DLOG_LOCAL_LEVEL=ESP_LOG_VERBOSE** esp-idf debug shit
* **-DLUNOKIOT_DEBUG** generic system debug messages
* **-DLUNOKIOT_DEBUG_UI** UI specific debug messages
* **-DLUNOKIOT_SILENT_BOOT** dont shake and no fanfare sound when start/stop
* **-DLUNOKIOT_WIFI_ENABLED** start the wifi timed connection subsystem
  * **-DLUNOKIOT_UPDATES_ENABLED** check lastbuild on github (blocked development until OTA)
* **-DLUNOKIOT_BLE_ENABLED** start the ble subsystem
* **-DLUNOKIOT_SCREENSHOOT_ENABLED** Allow to do screenshoots (touch 5 seconds to use it)

## Common problems:
### Build failed: "NO WEATHER KEY FILE FOUND, ABORT BUILD"
 * If you don't have a openweather account get it one and generate a user key
 * Put the key on "openWeatherKey.txt" (yes, camelCase)
 * Try to build again
### Clock don't show nothing on screen
 * is a V3 device?
 * if are other model, edit platformio.ini and change "build_flags = -DLILYGO_WATCH_2020_V3" to fit with

## Know problems
 * Lower than 172Kb of free heap makes fail the sprite alloc, getting glitches or missinformation
 * Some kind of leak on (almost) heap between system and watchface... long term use impossible, restarted by heap overflow occurs after many of uses (glitches can be seen before as advice system is running out of memory)
 * BLE added a little bit of unstability (development in early stages...)
 * Touch are unstable meanwhile calibration is running (some seconds after boot)

## Warning:

**Advice about usability:** This software is under heavy development and erlier development phase... many parts can be disfunctional, broken or buggy

**Advise about privacy:** This software (as device does) can monitor your activities (for fitness applications purposes) and isn't shared outside

**Advise about warranty:** Absolutely no warrant about nothing, use at your own risk

## Contact us: https://t.me/lunowatch