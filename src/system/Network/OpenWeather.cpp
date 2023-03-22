#include <Arduino.h>
#include <ArduinoNvs.h>
#include <LilyGoWatch.h>
extern TTGOClass *ttgo;
#include <Arduino_JSON.h>
#include <HTTPClient.h>

#include "../Network.hpp"
#include "../../app/LogView.hpp"

#include "../Datasources/database.hpp"
#include "OpenWeather.hpp"

bool ntpSyncDone=false;
bool weatherSyncDone = false;

// NTP sync data
const char *ntpServer = "pool.ntp.org";
// const long  gmtOffset_sec   = 3600;
// const int   daylightOffset_sec = 0; // winter
// const int   daylightOffset_sec = 3600; // summer

// network timed tasks
NetworkTaskDescriptor * NetworkNTPTask = nullptr;
NetworkTaskDescriptor * NetworkWeatherTask = nullptr;
NetworkTaskDescriptor * NetworkGeoIPTask = nullptr;


int weatherId = -1;
char *weatherMain = nullptr;
char *weatherDescription = nullptr;
char *weatherIcon = nullptr;
double weatherTemp = -1000;

// Extra Weather variables
double weatherFeelsTemp = -1000;
double weatherTempMin = -1000;
double weatherTempMax = -1000;
double weatherHumidity = 0;
double weatherPressure = 0;
double weatherWindSpeed = 0;

extern const PROGMEM uint8_t openweatherPEM_start[] asm("_binary_asset_openweathermap_org_pem_start");
extern const PROGMEM uint8_t openweatherPEM_end[] asm("_binary_asset_openweathermap_org_pem_end");
extern const PROGMEM char openWeatherMapApiKey[];

char *geoIPReceivedData = nullptr;
char *weatherReceivedData = nullptr;

char *weatherCity = nullptr;
char *weatherCountry = nullptr;
const PROGMEM char urlOpenWeatherFormatString[] = "http://api.openweathermap.org/data/2.5/weather?q=%s,%s&units=metric&appid=%s";


#include <WiFi.h>
#include <HTTPClient.h>

String jsonBuffer;
String NetworkHTTPGETRequest(const char* serverName) {
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

bool NetworkParseWeatherData() {

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
    if (false == myObject.hasOwnProperty("wind")) {
      lNetLog("Watchface: ERROR: OpenWeather JSON parsing: property 'wind' not found\n");
#ifdef LUNOKIOT_DEBUG
      Serial.printf("%s\n", weatherReceivedData);
#endif
      return false;
    }
    JSONVar mainBranch = myObject["main"];
    weatherTemp = mainBranch["temp"];
    weatherTempMin = mainBranch["temp_min"];
    weatherTempMax = mainBranch["temp_max"];
    weatherHumidity = mainBranch["humidity"];
    weatherPressure = mainBranch["pressure"];
    weatherFeelsTemp = mainBranch["feels_like"];

    JSONVar wind = myObject["wind"];
    weatherWindSpeed = wind["speed"];

    String jsonString = myObject.stringify(myObject);
    SqlJSONLog("oweather",jsonString.c_str());

    return true;
}


bool NetworkGetSecureWeather() {
    if (( nullptr == weatherCity )||( nullptr == weatherCountry )) {
        lNetLog("OpenWeather: Unable to get location due no City or Country setted\n");
        return false;
    }
    char *url = (char *)ps_malloc(200); // usually ~130
    sprintf(url,urlOpenWeatherFormatString,weatherCity,weatherCountry,openWeatherMapApiKey);
    lNetLog("GetSecureNetworkWeather: URL: %s Len: %d\n",url,strlen(url));

    jsonBuffer = NetworkHTTPGETRequest(url);
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
    
    //Serial.println(jsonBuffer);
    //JSONVar myObject = JSON.parse(jsonBuffer);


    /*
    WiFiClientSecure *client = new WiFiClientSecure;
    if(nullptr == client) {
        lNetLog("GetSecureNetworkWeather: ERROR: Unable to create WiFiClientSecure\n");
        free(url);
        return false;
    }

    HTTPClient *weatherClient = new HTTPClient;
    if ( nullptr == weatherClient ) {
        lNetLog("GetSecureNetworkWeather: ERROR: Unable to create HTTPClient\n");
        delete client;
        free(url);
        return false;
    }
    weatherClient->setConnectTimeout(LUNOKIOT_UPDATE_TIMEOUT);
    weatherClient->setTimeout(LUNOKIOT_UPDATE_TIMEOUT);
    weatherClient->setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    client->setCACert((const char*)openweatherPEM_start);
    FreeSpace(); // @TODO problem here
    if (false == weatherClient->begin(*client, url)) {  // HTTPS
        lNetLog("GetSecureNetworkWeather: ERROR: Unable to connect!\n");
        weatherClient->end();
        delete weatherClient;
        delete client;
        free(url);
        return false;
    }
    int httpResponseCode = weatherClient->GET();
    lNetLog("Watchface: GetSecureNetworkWeather: HTTP response code: %d\n", httpResponseCode);
    if (httpResponseCode < 1) {
        weatherClient->end();
        delete weatherClient;
        delete client;
        free(url);
        return false;
    }
    // clean old buffer if needed
    if (nullptr != weatherReceivedData) {
        free(weatherReceivedData);
        weatherReceivedData = nullptr;
    }
    String payload = weatherClient->getString(); // fuck strings
    

    lNetLog("Watchface: GetSecureNetworkWeather: free weatherClient\n");
    weatherClient->end();
    delete weatherClient;
    delete client;
    free(url);
    // Generate the received data buffer
    if (nullptr == weatherReceivedData) {
        lNetLog("Allocate: weatherReceivedData size: %d\n",payload.length());
        weatherReceivedData = (char *)ps_malloc(payload.length() + 1);
        strcpy(weatherReceivedData, payload.c_str());
    }
    lNetLog("HTTP: %s\n",weatherReceivedData);
    return true;
    */





    /*
    HTTPClient *weatherClient = new HTTPClient();
    weatherClient->begin(url,(const PROGMEM char *)openweatherPEM_start);
    int httpResponseCode = weatherClient->GET();
    free(url);
    lNetLog("Watchface: GetSecureNetworkWeather: HTTP response code: %d\n", httpResponseCode);
    if (httpResponseCode < 1) {
        weatherClient->end();
        delete weatherClient;
        return false;
    }
    // clean old buffer if needed
    if (nullptr != weatherReceivedData) {
        free(weatherReceivedData);
        weatherReceivedData = nullptr;
    }
    String payload = weatherClient->getString(); // fuck strings
    
    lNetLog("Watchface: GetSecureNetworkWeather: free weatherClient\n");
    weatherClient->end();
    delete weatherClient;

    // Generate the received data buffer
    if (nullptr == weatherReceivedData) {
        lNetLog("Allocate: weatherReceivedData size: %d\n",payload.length());
        weatherReceivedData = (char *)ps_malloc(payload.length() + 1);
        strcpy(weatherReceivedData, payload.c_str());
    }
    lNetLog("HTTP: %s\n",weatherReceivedData);
    return true;
    */





    /*

    WiFiClientSecure *client = new WiFiClientSecure;
    if (nullptr == client ) {
        lNetLog("GetSecureNetworkWeather: Unable to create WiFiClientSecure\n");
        return false;
    }
    // @NOTE to get the PEM from remote server:
    // $ openssl s_client -connect api.openweathermap.org:443 -showcerts </dev/null | openssl x509 -outform pem > openweathermap_org.pem
    // $ echo "" | openssl s_client -showcerts -connect api.openweathermap.org:443 | sed -n "1,/Root/d; /BEGIN/,/END/p" | openssl x509 -outform PEM >api_openweathermap_org.pem
    client->setCACert((const char *)openweatherPEM_start);
    char *fuckingURLBuffer = (char *)ps_malloc(180); 
    const char urlFormatString[] = "https://api.openweathermap.org/data/2.5/weather?q=%s,%s&units=metric&APPID=%s";
    sprintf(fuckingURLBuffer,urlFormatString,weatherCity,weatherCountry,openWeatherMapApiKey);
    lNetLog("GetSecureNetworkWeather: URL: %s Len: %d\n",fuckingURLBuffer,strlen(fuckingURLBuffer));
    String serverPath = fuckingURLBuffer;
    free(fuckingURLBuffer);
    String payload;
    {
        HTTPClient * https = new HTTPClient();
        https->setConnectTimeout(8 * 1000);
        https->setTimeout(8 * 1000);
        //https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        if (false == https->begin(*client, serverPath)) {
            lNetLog("GetSecureNetworkWeather: Unable to launch HTTPClient\n");
            https->end();
            delete https;
            delete client;
            return false;
        }
        // HTTPS
        // start connection and send HTTP header
        int httpCode = https->GET();
        if (httpCode < 1) {
            https->end();
            delete https;
            delete client;
            lNetLog("GetSecureNetworkWeather: HTTP error: %d\n",httpCode);
            return false; // fake to no call again meanwhile DEBUG @TODO
        }
        if (httpCode != HTTP_CODE_OK ) {
            https->end();
            delete https;
            delete client;
            lNetLog("GetSecureNetworkWeather: HTTP response invalid: %d\n",httpCode);
            return false; // fake to no call again meanwhile DEBUG @TODO
        }
        lNetLog("GetSecureNetworkWeather: Getting data...\n");
        payload = https->getString();
        https->end();
        delete https;
        delete client;
    }


    if (nullptr != weatherReceivedData) {
        free(weatherReceivedData);
        weatherReceivedData = nullptr;
    }

    weatherReceivedData = (char *)ps_malloc(payload.length() + 1);
    strcpy(weatherReceivedData, payload.c_str());
    lNetLog("Received:\n%s\n", weatherReceivedData);
    return true;
    */
    /*
    WiFiClientSecure *client = new WiFiClientSecure;
    if (nullptr != client) {
        client->setCACert((const char *)openweatherPEM_start);
        { // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
            HTTPClient https;
            https.setConnectTimeout(8 * 1000);
            https.setTimeout(8 * 1000);
            https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
            //     https://api.openweathermap.org/data/2.5/weather?q=Barcelona,ES&units=metric&APPID=b228d64ad499d78d86419fe10a131241
            String serverPath = "https://api.openweathermap.org/data/2.5/weather?q=" + String(weatherCity) + "," + String(weatherCountry) + String("&units=metric") + String("&APPID=") + String(openWeatherMapApiKey); // + String("&lang=") + String("ca");
            if (https.begin(*client, serverPath)) {
                // HTTPS
                lNetLog("https get '%s'...\n", serverPath.c_str());
                // start connection and send HTTP header
                int httpCode = https.GET();

                // httpCode will be negative on error
                if (httpCode > 0) {
                    // HTTP header has been send and Server response header has been handled
                    lNetLog("https get code: %d\n", httpCode);
                    // file found at server
                    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                        String payload = https.getString();
                        if (nullptr != weatherReceivedData) {
                            free(weatherReceivedData);
                            weatherReceivedData = nullptr;
                        }
                        weatherReceivedData = (char *)ps_malloc(payload.length() + 1);
                        lLog("BBBBBBBB\n");
                        strcpy(weatherReceivedData, payload.c_str());
                        lNetLog("Received:\n%s\n", weatherReceivedData);
                        // config.alreadySync = true;
                    } else {
                        lNetLog("Received http: %d :(\n",httpCode);
                        https.end();
                        delete client;
                        return false;
                    }
                } else {
                    lNetLog("http get failed, error: %s\n", https.errorToString(httpCode).c_str());
                    https.end();
                    delete client;
                    return false;
                }
                https.end();
            } else {
                lNetLog("Watchface: Weather: http unable to connect\n");
                delete client;
                return false;
            }
        }
        delete client;
        return true;
    }
    lNetLog("Watchface: Weather: Unable to create WiFiClientSecure\n");
    */
    return false;
}


void OpenWeatherTaskHandler() {
    if (nullptr == NetworkNTPTask) {
        NetworkNTPTask = new NetworkTaskDescriptor();
        NetworkNTPTask->name = (char *)"NTP Watchface";
        NetworkNTPTask->everyTimeMS = ((1000 * 60) * 60) * 24; // once a day
        NetworkNTPTask->_lastCheck = millis();
        if ( false == ntpSyncDone ) {
            NetworkNTPTask->_nextTrigger = 0; // launch NOW if no synched never again
        }
        NetworkNTPTask->desiredStack = LUNOKIOT_QUERY_STACK_SIZE;
        NetworkNTPTask->callback = []() {
            if (false == (bool)NVS.getInt("NTPEnabled"))
            {
                lEvLog("Watchface: NTP Sync disabled\n");
                return true;
            }
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                lEvLog("ESP32: Time: %02d:%02d:%02d %02d-%02d-%04d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon, 1900 + timeinfo.tm_year);
            }

            lNetLog("Watchface: Trying to sync NTP time...\n");
            // delay(100);
            // init and get the time
            int daylight = NVS.getInt("summerTime");
            long timezone = NVS.getInt("timezoneTime");
            lEvLog("Watchface: Summer time: '%s', GMT: %+d\n", (daylight?"yes":"no"),timezone);
            configTime(timezone * 3600, daylight * 3600, ntpServer);
            // Set offset time in seconds to adjust for your timezone, for example:
            // GMT +1 = 3600
            // GMT +8 = 28800
            // GMT -1 = -3600
            // GMT 0 = 0
            while (false == getLocalTime(&timeinfo, 100)) {
                // delay(100)
                TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
                xTaskDelayUntil( &nextCheck, (100 / portTICK_PERIOD_MS) ); // wait a ittle bit
            }
            lNetLog("Watchface: NTP Received\n");
            if (getLocalTime(&timeinfo))  {
                lEvLog("ESP32: Time: %02d:%02d:%02d %02d-%02d-%04d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1, 1900 + timeinfo.tm_year);
                ttgo->rtc->syncToRtc();
                RTC_Date d = ttgo->rtc->getDateTime();
                lEvLog("RTC: Sync: '%s'\n", ttgo->rtc->formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));
                lNetLog("NTP sync done!\n");
                //ntpSyncDone = true;
                return true;
            }
            return false;
        };
        NetworkNTPTask->enabled = (bool)NVS.getInt("NTPEnabled");
        AddNetworkTask(NetworkNTPTask);
    }



    bool oweatherValue = (bool)NVS.getInt("OWeatherEnabled");

    if (nullptr == NetworkGeoIPTask) {
        NetworkGeoIPTask = new NetworkTaskDescriptor();
        NetworkGeoIPTask->name = (char *)"GeoIP Watchface";
        NetworkGeoIPTask->everyTimeMS = ((60 * 1000) * 60)*3; // every 3 hours
        //NetworkGeoIPTask->payload = (void *)this;
        NetworkGeoIPTask->_lastCheck = millis();
        NetworkGeoIPTask->_nextTrigger = 0; // launch NOW (as soon as system wants)
        NetworkGeoIPTask->callback = []() {
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
        NetworkGeoIPTask->enabled = oweatherValue;
        NetworkGeoIPTask->desiredStack=LUNOKIOT_QUERY_STACK_SIZE;
        AddNetworkTask(NetworkGeoIPTask);
    }

    if (nullptr == NetworkWeatherTask) {
        NetworkWeatherTask = new NetworkTaskDescriptor();
        NetworkWeatherTask->name = (char *)"OpenWeather Watchface";
        NetworkWeatherTask->everyTimeMS = (60 * 1000) * 30; // every 30 minutes
        //NetworkWeatherTask->payload = (void *)this;
        NetworkWeatherTask->_lastCheck = millis();
        NetworkWeatherTask->_nextTrigger = 0; // launch NOW (as soon as system wants)
        NetworkWeatherTask->callback = [&]() {
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
            bool getDone = NetworkGetSecureWeather();

            if (getDone) {
                bool parseDone = NetworkParseWeatherData();
                weatherSyncDone = parseDone;
                return parseDone;
            }
            return getDone;
        };
        bool oweatherValue = (bool)NVS.getInt("OWeatherEnabled");
        NetworkWeatherTask->enabled = oweatherValue;
        NetworkWeatherTask->desiredStack = LUNOKIOT_TASK_STACK_SIZE;
        AddNetworkTask(NetworkWeatherTask);
    }
}
