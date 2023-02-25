
#ifndef __LUNOKIOT__WATCHFACE_HANDLERS_APP__
#define __LUNOKIOT__WATCHFACE_HANDLERS_APP__

#include <Arduino.h>
#include "../system/Datasources/database.hpp"
#include "../system/Network.hpp"
#include "../system/Application.hpp"
#include "../UI/activator/ActiveRect.hpp"
#include "../UI/widgets/CanvasZWidget.hpp"

class WatchfaceHandlers {
 private:
  
  String jsonBuffer;
  String httpGETRequest(const char* serverName);

 public:
  const char* AppName() { return "WatchfaceHandlers"; };

  bool wifiEnabled = false;
  bool weatherSyncDone = false;
  int weatherId = -1;
  char* weatherMain = nullptr;
  char* weatherDescription = nullptr;
  char* weatherIcon = nullptr;
  double weatherTemp = -1000;
  double weatherFTemp = -1000;
  double wspeed = 0;

  bool bleEnabled;         //@TODO this is a crap!!!
  bool bleServiceRunning;  //@TODO this is a crap!!!
  bool blePeer;

  static void FreeRTOSEventReceived(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
  bool GetSecureNetworkWeather();
  bool ParseWeatherData();
  void Handlers();

  // network timed tasks
  NetworkTaskDescriptor* ntpTask = nullptr;
  NetworkTaskDescriptor* weatherTask = nullptr;
  NetworkTaskDescriptor* geoIPTask = nullptr;

};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_WFHANDLER)
extern WatchfaceHandlers wfhandler;
extern bool ntpSyncDone;
#endif

#endif
