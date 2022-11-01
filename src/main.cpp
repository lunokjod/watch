/*
 * Hi!!! any developer, issuer, ideas, tips are welcome!!!
 */
#include <Arduino.h> // more portable than esp-idf, but more limited
#include <LilyGoWatch.h> // manufacturer library
#include <ArduinoNvs.h> // persistent values

#include "lunokiot_config.hpp" // all watch shit and hardcoded values (generated!! don't waste your time)

#include "system/SystemEvents.hpp" // the "heart"
#include "UI/UI.hpp" // UI optional stuff for user interaction

#include "UI/BootSplash.hpp" // convenient hardcoded, quick and dirty splash screen

#include "system/Application.hpp" // UI App definitions

#include "app/Watchface.hpp" // main app


TTGOClass *ttgo; // ttgo library shit ;)

RTC_DATA_ATTR uint32_t bootCount = 0; // @TODO useless...don't work, why?

#ifdef LUNOKIOT_DEBUG
unsigned long currentBootTime = 0; // debug-freak info
#endif

void setup() {
  // for monitoring uptime
  unsigned long setupBeginTime = millis();

  // do you want debug?
  Serial.begin(115200);
  #ifdef LUNOKIOT_DEBUG
      Serial.setDebugOutput(true);
      esp_log_level_set("*", ESP_LOG_VERBOSE);
  #endif

  // announe myself with build information
  Serial.printf("lunokIoT: 'компаньон' #%d//%s// (boot number %u)\n", LUNOKIOT_BUILD_NUMBER, LUNOKIOT_KEY, bootCount);
  bootCount++;

#ifdef LUNOKIOT_DEBUG
  Serial.println("lunokIoT: Init lilyGo TWatch2020 hardware...");
#endif
  // lilygo Twatch library dependencies
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  NVS.begin(); // need NVS to get the current configuration
  uint8_t rotation = NVS.getInt("ScreenRot");
  Serial.printf("lunokIoT: User screen rotation: %d\n", rotation);
  ttgo->tft->setRotation(rotation); // user selected rotation (0 by default)

#ifdef LUNOKIOT_DEBUG
  Serial.println("lunokIoT: System initializing...");
#endif
  SplashAnnounce(); // simple eyecandy meanwhile boot

#ifdef LILYGO_WATCH_2020_V3
  // haptic announce (@TODO user choice "silent boot")
#ifndef LUNOKIOT_SILENT_BOOT
  ttgo->motor_begin();
  delay(10); // need to shake works
  ttgo->shake();
#endif
#endif


  // begin system communications with applications and itself (lunokIoT system bus)
  SystemEventsStart();

  AXPIntHandler();  // interrupt handlers
  BMPIntHandler();
  //RTCIntHandler();

  NetworkHandler(); // already provisioned? start the network timed tasks loop

  UIStart();  // Start the interface with the user via the screen and buttons!!! (born to serve xD)

#ifdef LUNOKIOT_DEBUG
  unsigned long setupEndTime = millis();
  currentBootTime = setupEndTime-setupBeginTime;
  Serial.printf("lunokIoT: Boot time: %lu ms\n", currentBootTime);
#endif
  SplashAnnounceEnd();
  // Announce system boot end and begin the magic!!!
  SystemEventBootEnd(); // notify to system end boot procedure (SystemEvents must launch watchface here)


}
void loop() { vTaskDelete( NULL ); } // don't need arduinofw loop, can free it if you want

// https://github.com/espressif/esp-idf/tree/cab6b6d1824e1a1bf4d965b747d75cb6a77f5151/examples/bluetooth/bluedroid/classic_bt/bt_hid_mouse_device



/* IR TEST
#include <IRremoteESP8266.h>
#include <IRsend.h>

IRsend irLed(TWATCH_2020_IR_PIN);
void shit() {
  const uint16_t rawData[] = { 0, 65535, 0, 65535 };
  irLed.begin();
  while(true) {
    irLed.sendRaw(rawData, 67, 38);  // Send a raw data capture at 38kHz.
    delay(2000);
  }
}
*/