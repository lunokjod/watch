# lilyGo TWatch 2020 lunokIoTWatch firmware

* Note: under heavy development (not ready for everyday use)
* On first launch must set WiFi credentials using QR (WiFI only at this time) using android device: https://play.google.com/store/apps/details?id=com.espressif.provsoftap

## Basic usage:

* Use button to wake/sleep the watch
* When screen goes on, the watchface appears
* Long button push for poweroff

## What does:

* WiFi time sync once every 24h (using network time protocol NTP)
* WiFi weather sync every 30 minutes, needs online account (set your openWeather key in "openWeatherKey.txt" before build) more info: https://openweathermap.org/
* Wake up every 60 seconds to monitor the user pose (future sleep monitor)
* Wake up every activity/stepcounter (future fitness monitor)

## Hardware support:
 * BMA423 support (including during sleep)
 * AXP202 support (including sleep)
 * haptic and vibration support
 * ST7789V support (lilygo TFT_eSPI)
 * Partial RTC support (system sync)
 * MAX98357A (mp3 playback support)
 * WiFi (via provisioning)

### Pending to implement:
* PDM Mic (voice assistant)
* RTC (alarm)
* IR (jokes with IR devices mostly x'D)
* Bluetooth ( https://www.espruino.com/Gadgetbridge or something like )
* espNow (locate other family watches)
* Touch on sleep
* Custom activities (hidden app availiable for testing)
* feedbacked apps (what do you need?)

## Common problems:
### Build failed: "NO WEATHER KEY FILE FOUND, ABORT BUILD"
 * If you don't have a openweather account get it one and generate a user key
 * Put the key on "openWeatherKey.txt" (yes, camelCase)
 * Try to build again
### Clock don't show nothing on screen
 * is a V3 device?
 * if are other model, edit platformio.ini and change "build_flags = -DLILYGO_WATCH_2020_V3" to fit with
## Warning:

**Advice about usability:** This software is under heavy development and erlier development phase... many parts can be disfunctional, broken or buggy

**Advise about privacy:** This software (as device does) can monitor your activities (for fitness applications purposes) and isn't shared outside

**Advise about warranty:** Absolutely no warrant about nothing, use at your own risk

## Contact us: https://t.me/lunowatch