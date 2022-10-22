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

## Contact us: https://t.me/lunowatch