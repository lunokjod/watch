/*
 * Hi!!! any developer, issuer, ideas, tips are welcome!!!
 */
#include <Arduino.h> // more portable than esp-idf, but more limited
#include <LilyGoWatch.h> // manufacturer library
#include <ArduinoNvs.h> // persistent values
#include "lunokiot_config.hpp" // all watch shit and hardcoded values (generated!! don't waste your time)

#include "system/SystemEvents.hpp" // the "heart"
#include "UI/UI.hpp" // UI optional stuff for user interaction
#include "system/Tasks.hpp"
#include "system/Network.hpp"

#include "UI/BootSplash.hpp" // convenient hardcoded, quick and dirty splash screen

#include "system/Application.hpp" // UI App definitions

#include "app/Steps.hpp" // Step manager

#include "app/LogView.hpp" // adds lLog(...)

TTGOClass *ttgo; // ttgo library shit ;)
TFT_eSPI * tft;

#ifdef LUNOKIOT_DEBUG
unsigned long currentBootTime = 0; // debug-freak info
#endif
extern bool ntpSyncDone;
extern const char *ntpServer;


void setup() { // you know that!!! (see ArduinoFW)

  // get current milis monitoring uptime (usually 0)
  unsigned long setupBeginTime = millis();

  // do you want debug?
  #ifdef LUNOKIOT_SERIAL
    Serial.begin(LUNOKIOT_SERIAL_SPEED);
    Serial.setDebugOutput(true);
    esp_log_level_set("*", ESP_LOG_VERBOSE);
  #endif

  // announce myself with build information
  Serial.printf("lunokIoT: 'компаньон' #%d//%s//\n", LUNOKIOT_BUILD_NUMBER, LUNOKIOT_KEY);

  // lilygo Twatch library dependencies
  ttgo = TTGOClass::getWatch();
  ttgo->begin(); //lLog functions become possible beyond here (TFT init)

  tft = ttgo->tft; // dont use ttgo-> lib (future porting)

  NVS.begin(); // need NVS to get the current configuration

  uint8_t rotation = NVS.getInt("ScreenRot"); // get screen rotation user select from NVS
  lUILog("User screen rotation: %d\n", rotation);
  ttgo->tft->setRotation(rotation); // user selected rotation (0 by default)


  size_t themeOffset = NVS.getInt("lWTheme"); // load current theme offset
  currentColorPalette = &AllColorPaletes[themeOffset]; // set the color palette (informative)
  currentThemeScheme = &AllThemes[themeOffset]; // set the GUI colors
  SplashAnnounce(); // simple eyecandy meanwhile boot (themed)

  userTall= NVS.getInt("UserTall");        // get how high is the user
  if ( 0 == userTall ) { userTall = 120; } // almost midget value

  userMaleFemale = NVS.getInt("UserSex");  // maybe is better use "hip type A/B" due to no offend any gender
  if ( true == userMaleFemale ) { // this code isn't trying to define any gender, only their hips proportions...
    // BIOLOGICAL FEMALE PROPS
    stepDistanceCm = userTall * WOMAN_STEP_PROPORTION;
  } else {
    // BIOLOGICAL MALE PROPS
    stepDistanceCm = userTall * MAN_STEP_PROPORTION;
  }

  // first logged message
#ifdef LUNOKIOT_DEBUG
  lEvLog("lunokIoT: System initializing...\n");
#endif

#ifdef LILYGO_WATCH_2020_V3
  // haptic announce (@TODO user choice "silent boot")
#ifndef LUNOKIOT_SILENT_BOOT
  ttgo->motor_begin();
  delay(10); // need for shake works
  ttgo->shake();
#endif
#endif
  // get if are in summertime
  int daylight = NVS.getInt("summerTime");
  // get the GMT timezone
  long timezone = NVS.getInt("timezoneTime");
  // announce values to log
  lEvLog("RTC: Config: Summer time: %s GMT: %d\n",(daylight?"true":"false"),timezone);
  configTime(timezone*3600, daylight*3600, ntpServer); // set ntp server query

  struct tm timeinfo;
  if ( ttgo->rtc->isValid() ) { // woa! RTC seems to guard the correct timedate from last execution :D
    lEvLog("RTC: The timedate seems valid\n");
    // inquiry RTC via i2C
    RTC_Date r = ttgo->rtc->getDateTime();
    lEvLog("RTC: time: %02u:%02u:%02u date: %02u-%02u-%04u\n",r.hour,r.minute,r.second,r.day,r.month,r.year);
    ttgo->rtc->syncToSystem(); // feed ESP32 with correct time
    if (getLocalTime(&timeinfo)) {
      lEvLog("ESP32: Time: %02d:%02d:%02d %02d-%02d-%04d\n",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec,timeinfo.tm_mday,timeinfo.tm_mon,1900+timeinfo.tm_year);
      ntpSyncDone=true; // RTC says "I'm ok", bypass NTP as time font
    }
  }
  // begin system communications with applications and itself (lunokIoT system bus)
  SystemEventsStart();

  AXPIntHandler();  // interrupt handlers
  BMPIntHandler();
  //RTCIntHandler();

  TimedTaskHandler();

  NetworkHandler(); // already provisioned? start the network timed tasks loop

  UIStart();  // Start the interface with the user via the screen and buttons!!! (born to serve xD)

  InstallStepManager();

  SplashAnnounceEnd();
#ifdef LUNOKIOT_DEBUG
  unsigned long setupEndTime = millis();
  currentBootTime = setupEndTime-setupBeginTime;
  lEvLog("lunokIoT: Boot time: %lu ms\n", currentBootTime);
#endif

  // Announce system boot end and begin the magic!!!
  SystemEventBootEnd(); // notify to system end boot procedure (SystemEvents must launch watchface here)


}
void loop() { vTaskDelete( NULL ); } // don't need arduinofw loop, can free it if you want
/*
 * Note: If you want get ArduinoFW compatible code, try to use Applications, have a "setup()" and "loop()" equivalences
 * More info: https://github.com/lunokjod/watch/blob/devel/src/app/README.md#develop-new-application
 */


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