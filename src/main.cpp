#include <Arduino.h>
#include <LilyGoWatch.h>
#include <ArduinoNvs.h>

#include "lunokiot_config.hpp"

#include "system/SystemEvents.hpp"

#include "UI/UI.hpp"
#include "UI/BootSplash.hpp"

#include "system/Application.hpp"
#include "app/Watchface.hpp"

TTGOClass *ttgo;

RTC_DATA_ATTR uint32_t bootCount = 0;
unsigned long currentBootTime = 0;

void setup() {
  // for monitoring uptime
  unsigned long setupBeginTime = millis();

  // do you want debug?
  Serial.begin(115200);
  #ifdef LUNOKIOT_DEBUG
      Serial.setDebugOutput(true);
      esp_log_level_set("*", ESP_LOG_VERBOSE);
  #endif

  // announe myself
  Serial.printf("lunokIoT: 'компаньон' #%d//%s// (boot number %u)\n", LUNOKIOT_BUILD_NUMBER, LUNOKIOT_KEY, bootCount);
  bootCount++;
  Serial.println("lunokIoT: Init lilyGo TWatch2020 hardware...");
  // lilygo Twatch library dependencies
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  Serial.println("@TODO motor disabled!!!");
  //ttgo->motor_begin();
  ttgo->shake();
  ttgo->tft->setRotation(0);  //  default correct position, the ttgo code sux

  Serial.println("lunokIoT: System initializing...");
  DrawSplash(); // simple eyecandy

  NVS.begin(); // need NVS to get the current configuration

  // begin system communications with itself (lunokIoT system)
  SystemEventsStart();
  AXPIntHandler();  // interrupt handlers
  BMPIntHandler();
  //RTCIntHandler();

  // Start the interface with the user via the screen and buttons!!! (born to serve xD)
  UIStart();

  bool netDone = NetworkHandler(); // already provisioned?


  unsigned long setupEndTime = millis();
  currentBootTime = setupEndTime-setupBeginTime;
  Serial.printf("lunokIoT: Boot time: %lums\n", currentBootTime);

  SystemEventBootEnd(); // notify system end boot procedure

  if ( netDone ) {
    LaunchApplication(new WatchfaceApplication());
  }
}

void loop() { vTaskDelete( NULL ); } // don't need arduinofw loop