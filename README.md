# lilyGo TWatch 2020 v3 firmware

* Note: under heavy development (not ready for everyday use)
* On first launch must set WiFi credentials using QR (WiFI only at this time) using android device: https://play.google.com/store/apps/details?id=com.espressif.provsoftap

## Basic usage:

* Use button to wake/sleep the watch
* When screen goes on, the watchface appears

## What does:
* WiFi time sync once every 24h (using network time protocol NTP)
* WiFi weather sync every 30 minutes, needs online account (set your openWeather key in "openWeatherKey.txt" before build) more info: https://openweathermap.org/

## Common problems:
* Build failed: "-----------> NO WEATHER KEY FILE FOUND, ABORT BUILD"
 * If you don't have a openweaher account get it one and generate a user key
 * Put the key on "openWeatherKey.txt" (yes, camelCase)
 * Try to build again

## Warning:

**Advice about usability:** This software is under heavy development and erlier development phase... many parts can be disfunctional, broken or buggy

**Advise about privacy:** This software (as device does) can monitor your activities (for fitness applications purposes) and isn't shared outside

**Advise about warranty:** Absolutely no warrant about nothing, use at your own risk

## Contact us: https://t.me/lunowatch