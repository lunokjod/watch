//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//    Watchface Author: @hpsaturn
//    License: GPLv3
//----------------------------------------------------------------

#include "WatchfaceSquare.hpp"

extern float PMUBattDischarge;

extern const PROGMEM char openWeatherMapApiKey[];

extern const PROGMEM uint8_t openweatherPEM_start[] asm("_binary_asset_openweathermap_org_pem_start");
extern const PROGMEM uint8_t openweatherPEM_end[] asm("_binary_asset_openweathermap_org_pem_end");
const PROGMEM char urlOpenWeatherFormatString[] = "http://api.openweathermap.org/data/2.5/weather?q=%s,%s&units=metric&appid=%s";

// network timed tasks
NetworkTaskDescriptor *WatchfaceSquare::weatherTask = nullptr;
NetworkTaskDescriptor *WatchfaceSquare::geoIPTask = nullptr;

String WatchfaceSquare::httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

bool WatchfaceSquare::GetSecureNetworkWeather() {
    if (( nullptr == weatherCity )||( nullptr == weatherCountry )) {
        lNetLog("OpenWeather: Unable to get location due no City or Country setted\n");
        return false;
    }
    char *url = (char *)ps_malloc(200); // usually ~130
    sprintf(url,urlOpenWeatherFormatString,weatherCity,weatherCountry,openWeatherMapApiKey);
    lNetLog("GetSecureNetworkWeather: URL: %s Len: %d\n",url,strlen(url));

    jsonBuffer = WatchfaceSquare::httpGETRequest(url);
    if ( nullptr != weatherReceivedData ) {
        free(weatherReceivedData);
        weatherReceivedData=nullptr;
    }
    if ( jsonBuffer.length() > 0 ) {
        weatherReceivedData=(char*)ps_malloc(jsonBuffer.length()+1);
        sprintf(weatherReceivedData,"%s",jsonBuffer.c_str());
        return true;
    }
    return false;
}

bool WatchfaceSquare::ParseWeatherData() {

    if (nullptr == weatherReceivedData) {
        lNetLog("Watchface: ERROR: OpenWeather JSON parsing: Empty string ''\n");
        weatherId = -1;
        return false;
    }
    JSONVar myObject = JSON.parse(weatherReceivedData);
    if (JSON.typeof(myObject) == "undefined")
    {
        lNetLog("Watchface: ERROR: OpenWeather JSON parsing: malformed JSON\n");
#ifdef LUNOKIOT_DEBUG_NETWORK
        Serial.printf("%s\n", weatherReceivedData);
#endif
        return false;
    }
    if (false == myObject.hasOwnProperty("weather"))
    {
        lNetLog("Watchface: ERROR: OpenWeather JSON parsing: property 'weather' not found\n");
#ifdef LUNOKIOT_DEBUG_NETWORK
        Serial.printf("%s\n", weatherReceivedData);
#endif
        weatherId = -1;
        return false;
    }
    JSONVar weatherBranch = myObject["weather"][0];

    if (false == weatherBranch.hasOwnProperty("description"))
    {
        lNetLog("Watchface: ERROR: OpenWeather JSON parsing: property '[weather][0][description]' not found\n");
#ifdef LUNOKIOT_DEBUG_NETWORK
        Serial.printf("%s\n", weatherReceivedData);
#endif
        weatherId = -1;
        return false;
    }
    weatherId = weatherBranch["id"];
    const char *weatherMainConst = weatherBranch["main"];

    if (nullptr != weatherMain)
    {
        free(weatherMain);
        weatherMain = nullptr;
    }
    weatherMain = (char *)ps_malloc(strlen(weatherMainConst) + 1);
    strcpy(weatherMain, weatherMainConst);

    const char *weatherDescriptionConst = weatherBranch["description"];
    if (nullptr != weatherDescription)
    {
        free(weatherDescription);
        weatherDescription = nullptr;
    }
    weatherDescription = (char *)ps_malloc(strlen(weatherDescriptionConst) + 1);
    strcpy(weatherDescription, weatherDescriptionConst);

    const char *weatherIconConst = weatherBranch["icon"];
    if (nullptr != weatherIcon)
    {
        free(weatherIcon);
        weatherIcon = nullptr;
    }
    weatherIcon = (char *)ps_malloc(strlen(weatherIconConst) + 1);
    strcpy(weatherIcon, weatherIconConst);

    if (false == myObject.hasOwnProperty("main"))
    {
        lNetLog("Watchface: ERROR: OpenWeather JSON parsing: property 'main' not found\n");
#ifdef LUNOKIOT_DEBUG
        Serial.printf("%s\n", weatherReceivedData);
#endif
        return false;
    }
    JSONVar mainBranch = myObject["main"];
    weatherTemp = mainBranch["temp"];

    String jsonString = myObject.stringify(myObject);
    SqlJSONLog("oweather",jsonString.c_str());

    return true;
}

void WatchfaceSquare::Handlers()
{
    #ifdef LUNOKIOT_WIFI_ENABLED
    esp_err_t done = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, WatchfaceSquare::FreeRTOSEventReceived, this);
    if ( ESP_OK != done ) {
        lAppLog("Unable to register into WiFi freeRTOS event loop\n");
        return;
    }
    bool oweatherValue = (bool)NVS.getInt("OWeatherEnabled");
    if (nullptr == geoIPTask) {
        geoIPTask = new NetworkTaskDescriptor();
        geoIPTask->name = (char *)"GeoIP Watchface";
        geoIPTask->everyTimeMS = ((60 * 1000) * 60)*3; // every 3 hours
        geoIPTask->payload = (void *)this;
        geoIPTask->_lastCheck = millis();
        geoIPTask->_nextTrigger = 0; // launch NOW (as soon as system wants)
        geoIPTask->callback = [&]() {
            bool oweatherValue = (bool)NVS.getInt("OWeatherEnabled");
            if (false == oweatherValue) {
                lNetLog("Watchface: Openweather Sync disabled (geoip is futile)\n");
                return true;
            }
            const char url[] = "http://www.geoplugin.net/json.gp";
            // geoplugin_city
            // geoplugin_countryCode
            HTTPClient geoIPClient;
            geoIPClient.begin(url);
            int httpResponseCode = geoIPClient.GET();
            if (httpResponseCode < 1) {
                lNetLog("Watchface: ERROR: geoIP HTTP response code: %d\n", httpResponseCode);
                geoIPClient.end();
                return false;
            }
            lNetLog("Watchface: geoIP HTTP response code: %d\n", httpResponseCode);
            // clean old buffer if needed
            if (nullptr != geoIPReceivedData) {
                free(geoIPReceivedData);
                geoIPReceivedData = nullptr;
            }
            String payload = geoIPClient.getString(); // fuck strings
            
            lNetLog("Watchface: free geoIPClient\n");
            geoIPClient.end();

            // Generate the received data buffer
            if (nullptr == geoIPReceivedData) {
                lNetLog("Allocate: geoIPReceivedData size: %d\n",payload.length());
                geoIPReceivedData = (char *)ps_malloc(payload.length() + 1);
                strcpy(geoIPReceivedData, payload.c_str());
            }
            lNetLog("Parsing response...\n");
            // process response
            JSONVar myObject = JSON.parse(geoIPReceivedData);
            if (JSON.typeof(myObject) == "undefined") {
                lNetLog("Watchface: ERROR: geoIP JSON parsing: malformed JSON\n");
                lNetLog("HTTP: %s\n",payload.c_str());
                return false;
            
            }
            //delete myObject["geoplugin_credit"];
            myObject["geoplugin_credit"] = undefined; // sorry,so expensive credits :( thanks geoIP!!!! :**
            //delete myObject["geoplugin_currencySymbol_UTF8"];
            myObject["geoplugin_currencySymbol_UTF8"] = undefined; // no utf problems please!
            //delete myObject["geoplugin_currencySymbol"];
            myObject["geoplugin_currencySymbol"] = undefined; // no utf problems please!

            //lLog("JSON:\n%s\n",myObject.stringify(myObject).c_str());
            if (false == myObject.hasOwnProperty("geoplugin_city")) {
                lNetLog("Watchface: ERROR: geoIP JSON parsing: unable to get 'geoplugin_city'\n");
                return false;
            }
            if (false == myObject.hasOwnProperty("geoplugin_countryCode")) {
                lNetLog("Watchface: ERROR: geoIP JSON parsing: unable to get 'geoplugin_countryCode'\n");
                return false;
            }
            JSONVar cityVar = myObject["geoplugin_city"];
            JSONVar countryVar = myObject["geoplugin_countryCode"];
            const char *cityString = cityVar;
            const char *countryString = countryVar;
            if (nullptr != weatherCountry) {
                free(weatherCountry);
                weatherCountry = nullptr;
            }
            if (nullptr != weatherCity) {
                free(weatherCity);
                weatherCity = nullptr;
            }
            if (nullptr == weatherCity) {
                weatherCity = (char *)ps_malloc(strlen(cityString) + 1);
            }
            if (nullptr == weatherCountry) {
                weatherCountry = (char *)ps_malloc(strlen(countryString) + 1);
            }
            strcpy(weatherCountry, countryString);
            strcpy(weatherCity, cityString);
            lNetLog("GeoIP: Country: '%s' City: '%s' (innacurate)\n",weatherCountry,weatherCity);

            String jsonString = myObject.stringify(myObject);
            SqlJSONLog("geoip",jsonString.c_str());

            return true;
        };
        geoIPTask->enabled = oweatherValue;
        geoIPTask->desiredStack=LUNOKIOT_QUERY_STACK_SIZE;
        AddNetworkTask(geoIPTask);
    }

    if (nullptr == weatherTask) {
        weatherTask = new NetworkTaskDescriptor();
        weatherTask->name = (char *)"OpenWeather Watchface";
        weatherTask->everyTimeMS = (60 * 1000) * 30; // every 30 minutes
        weatherTask->payload = (void *)this;
        weatherTask->_lastCheck = millis();
        weatherTask->_nextTrigger = 0; // launch NOW (as soon as system wants)
        weatherTask->callback = [&]() {
            weatherSyncDone = false;
            bool oweatherValue = (bool)NVS.getInt("OWeatherEnabled");
            if (false == oweatherValue) {
                lNetLog("Watchface: Openweather Sync disabled by NVS\n");
                return true;
            }
            lNetLog("Openweather KEY: '%s'\n",openWeatherMapApiKey);
            if (0 == strlen(openWeatherMapApiKey)) {
                lNetLog("Watchface: Openweather Cannot get weather without API KEY https://openweathermap.org/api\n");
                return true;
            }
            bool getDone = WatchfaceSquare::GetSecureNetworkWeather();

            if (getDone) {
                bool parseDone = WatchfaceSquare::ParseWeatherData();
                weatherSyncDone = parseDone;
                return parseDone;
            }
            return getDone;
        };
        weatherTask->enabled = oweatherValue;
        weatherTask->desiredStack = LUNOKIOT_TASK_STACK_SIZE;
        AddNetworkTask(weatherTask);
    }
#endif
}

#define MINIMUM_BACKLIGHT 10
#define MARGIN_TOP 3
#define MARGIN_LFT 2

char const *DAY[]={"SUN","MON","TUE","WED","THU","FRI","SAT"};

WatchfaceSquare::WatchfaceSquare() {
    lAppLog("WatchfaceSquare On\n");

    topRightButton = new ActiveRect(160, 0, 80, 80, [](void *unused) {
        LaunchApplication(new BatteryApplication());
    });
    if ( nullptr == topRightButton ) {
        lAppLog("Unable to allocate topRightButton\n");
        return;
    }

    bottomRightButton = new ActiveRect(160, 160, 80, 80, [](void *unused) {
        LaunchApplication(new MainMenuApplication());
    });
    if ( nullptr == bottomRightButton ) {
        lAppLog("Unable to allocate bottomRightButton\n");
        return;
    }

    topLeftButton = new ActiveRect(0, 0, 80, 80, [](void *unused) {
        LaunchApplication(new SettingsApplication());
    });

    if ( nullptr == topLeftButton ) {
        lAppLog("Unable to allocate bottomTopButton\n");
        return;
    }

    bottomLeftButton = new ActiveRect(0, 160, 80, 80, [](void *unused) {
        LaunchApplication(new StepsApplication());
    });
    if ( nullptr == bottomLeftButton ) {
        lAppLog("Unable to allocate bottomLeftButton\n");
        return;
    }
    Handlers();
    Tick(); // OR call this if no splash 
}

void WatchfaceSquare::FreeRTOSEventReceived(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    WatchfaceSquare *self = reinterpret_cast<WatchfaceSquare*>(handler_args);
    if (WIFI_EVENT == base) {
        if (WIFI_EVENT_STA_START == id) {
            self->wifiEnabled = true;
        }
        else if (WIFI_EVENT_STA_STOP == id) {
            self->wifiEnabled = false;
        }
    }
}

WatchfaceSquare::~WatchfaceSquare() {
    // please remove/delete/free all to avoid leaks!
    //delete mywidget;
    delete topRightButton;
    delete bottomRightButton;
    delete topLeftButton;
    delete bottomLeftButton;
}

bool WatchfaceSquare::Tick() {

    topRightButton->Interact(touched, touchX, touchY);
    bottomRightButton->Interact(touched, touchX, touchY);
    topLeftButton->Interact(touched, touchX, touchY);
    bottomLeftButton->Interact(touched, touchX, touchY);
    // low the brightness if system changes it
    if ( MINIMUM_BACKLIGHT != ttgo->bl->getLevel() ) { ttgo->setBrightness(MINIMUM_BACKLIGHT); }

    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(TFT_BLACK); // use theme colors

        time_t now;
        struct tm * tmpTime;
        struct tm timeinfo;
        time(&now);
        tmpTime = localtime(&now);
        memcpy(&timeinfo,tmpTime, sizeof(struct tm));
        char buffer[64] = { 0 };
        canvas->fillSprite(TFT_BLACK);
        // update day and month        
        canvas->setTextSize(4);
        canvas->setFreeFont(&TomThumb);
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(TL_DATUM);
        sprintf(buffer,"%02d.%02d", timeinfo.tm_mday, timeinfo.tm_mon+1);
        canvas->drawString(buffer, MARGIN_LFT, MARGIN_TOP);
        // update day of week
        int weekday=timeinfo.tm_wday;
        canvas->setTextColor(TFT_CYAN);
        canvas->setTextDatum(TC_DATUM);
        sprintf(buffer,"%s",DAY[weekday]);
        canvas->drawString(buffer, TFT_WIDTH/2+15, MARGIN_TOP);
        // update hours
        canvas->setTextSize(3);
        canvas->setFreeFont(&FreeMonoBold18pt7b);
        canvas->setTextColor(TFT_CYAN);
        canvas->setTextDatum(TR_DATUM);
        sprintf(buffer,"%02d", timeinfo.tm_hour);
        canvas->drawString(buffer, TFT_WIDTH+7, 30);
        // update minutes
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(CR_DATUM);
        sprintf(buffer,"%02d",timeinfo.tm_min);
        canvas->drawString(buffer, TFT_WIDTH+7, TFT_HEIGHT/2+17);
        // update battery 
        canvas->setFreeFont(&TomThumb);
        canvas->setTextSize(4);
        if (batteryPercent > -1) {
            uint32_t battColor = TFT_DARKGREY;
            bool battActivity = false;
            if (vbusPresent) battColor = TFT_GREEN;
            else if (batteryPercent < 10) battColor = ThCol(high);
            else if (batteryPercent < 35) battColor = TFT_YELLOW;
            else battColor = TFT_WHITE;
            canvas->setTextColor(battColor);
            int battFiltered = batteryPercent;
            if (battFiltered > 99) battFiltered = 99;
            sprintf(buffer, "%2d", battFiltered);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString(buffer, TFT_WIDTH-5, MARGIN_TOP);
            if(battFiltered == 0) battFiltered=1;
            canvas->fillRect(TFT_WIDTH-16,MARGIN_TOP,12,(int)(20/(100/battFiltered)),battColor);
        }
        else {
            canvas->setTextColor(TFT_DARKGREY);
            canvas->setTextDatum(TR_DATUM);
            canvas->drawString("NB", TFT_WIDTH-10, MARGIN_TOP);
        }

        if (weatherSyncDone) {
          char *textBuffer = (char *)ps_malloc(255);
          // weather icon
          canvas->setBitmapColor(ThCol(mark), TFT_BLACK);
          // watchFaceCanvas->canvas->fillRect(120 - (img_weather_200.width/2),52,80,46,TFT_BLACK);
          if (-1 != weatherId) {
            if ((200 <= weatherId) && (300 > weatherId)) {
              canvas->pushImage(120 - (img_weather_200.width / 2), 52, img_weather_200.width, img_weather_200.height, (uint16_t *)img_weather_200.pixel_data);
            } else if ((300 <= weatherId) && (400 > weatherId)) {
              canvas->pushImage(120 - (img_weather_300.width / 2), 52, img_weather_300.width, img_weather_300.height, (uint16_t *)img_weather_300.pixel_data);
            } else if ((500 <= weatherId) && (600 > weatherId)) {
              canvas->pushImage(120 - (img_weather_500.width / 2), 52, img_weather_500.width, img_weather_500.height, (uint16_t *)img_weather_500.pixel_data);
            } else if ((600 <= weatherId) && (700 > weatherId)) {
              canvas->pushImage(120 - (img_weather_600.width / 2), 52, img_weather_600.width, img_weather_600.height, (uint16_t *)img_weather_600.pixel_data);
            } else if ((700 <= weatherId) && (800 > weatherId)) {
              //lAppLog("@TODO Watchface: openweather 700 condition code\n");
              // watchFaceCanvas->canvas->pushImage(120 - (img_weather_800.width/2) ,52,img_weather_800.width,img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
              canvas->pushImage(120 - (img_weather_800.width / 2), 52, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
              // watchFaceCanvas->canvas->pushImage(144,52,img_weather_600.width,img_weather_600.height, (uint16_t *)img_weather_600.pixel_data);
            } else if ((800 <= weatherId) && (900 > weatherId)) {
              canvas->pushImage(120 - (img_weather_800.width / 2), 52, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
            }
          }
          // temperature
          if (-1000 != weatherTemp) {
            sprintf(textBuffer, "%2.1f C", weatherTemp);
            int16_t posX = 150;
            int16_t posY = 74;
            canvas->setTextFont(0);
            canvas->setTextSize(2);
            canvas->setTextColor(TFT_WHITE);
            canvas->setTextDatum(TL_DATUM);
            canvas->drawString(textBuffer, posX, posY);  //, 185, 96);
          }

          if (nullptr != weatherMain) {
            canvas->setTextFont(0);
            canvas->setTextSize(2);
            canvas->setTextDatum(CC_DATUM);
            canvas->setTextWrap(false, false);
            canvas->setTextColor(TFT_WHITE);
            canvas->drawString(weatherMain, 120, 40);
          }

          if (nullptr != weatherDescription) {
            canvas->setTextFont(0);
            canvas->setTextSize(1);
            canvas->setTextDatum(CC_DATUM);
            canvas->setTextWrap(false, false);
            canvas->setTextColor(TFT_BLACK);
            canvas->drawString(weatherDescription, 122, 62);
            canvas->setTextColor(TFT_WHITE);
            canvas->drawString(weatherDescription, 120, 60);
          }
        }

        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI

        return true;
    }
    return false;
}
