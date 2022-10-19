#include <Arduino.h>
#include <LilyGoWatch.h>
#include <ArduinoNvs.h>

#include "lunokiot_config.hpp"

#include "system/SystemEvents.hpp"

#include "UI/UI.hpp"
#include "UI/BootSplash.hpp"

#include "system/Application.hpp"
#include "app/Watchface.hpp"
#include "app/Provisioning.hpp"
#include "app/IATest.hpp"
#include "app/ActivityRecorder.hpp"

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
  // announe product name
  Serial.printf("lunokIoT: 'компаньон' #%d//%s// (boot number %u)\n", LUNOKIOT_BUILD_NUMBER, LUNOKIOT_KEY, bootCount);
  ++bootCount;
  Serial.println("lunokIoT: Init lilyGo TWatch2020 hardware...");
  // lilygo Twatch library dependencies
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  Serial.println("@TODO motor disabled!!!");
  //ttgo->motor_begin();
  ttgo->shake();
  ttgo->tft->setRotation(0);  //  default correct position, the ttgo code sux

  Serial.println("lunokIoT: System initializing...");
  DrawSplash();
  ttgo->openBL();

  NVS.begin();
  // begin system communications with itself (lunokIoT system)
  SystemEventsStart();
  AXPIntHandler();
  BMPIntHandler();
  //RTCIntHandler();
  // first, connect with the user via the screen and buttons!!! (born to serve xD)
  UIStart();

  bool netDone = NetworkHandler();


  unsigned long setupEndTime = millis();
  currentBootTime = setupEndTime-setupBeginTime;
  Serial.printf("lunokIoT: Boot time: %lums\n", currentBootTime);
  SystemEventBootEnd();

  if ( netDone ) {
    LaunchApplication(new WatchfaceApplication());
//    LaunchApplication(new ActivityRecorderApplication());
  }
}

void loop() { vTaskDelete( NULL ); }