#include "Watchface2.hpp"

#include <esp_log.h>
#include <Arduino.h>
#include <LilyGoWatch.h> // thanks for the warnings :/
extern TTGOClass *ttgo;
// #include <libraries/TFT_eSPI/TFT_eSPI.h>
#include <Arduino_JSON.h>
#include <HTTPClient.h>
#include <ArduinoNvs.h>

#include "../app/LogView.hpp" // for lLog functions
#include "../system/SystemEvents.hpp"
#include "MainMenu.hpp"

#include "../static/img_hours_hand.c"
#include "../static/img_minutes_hand.c"
#include "../static/img_seconds_hand.c"

#include "../static/img_wifi_24.xbm"
#include "../static/img_bluetooth_24.xbm"
#include "../static/img_bluetooth_peer_24.xbm"
#include "../static/img_usb_24.xbm"

#include "../static/img_weather_200.c"
#include "../static/img_weather_300.c"
#include "../static/img_weather_500.c"
#include "../static/img_weather_600.c"

#include "../static/img_weather_800.c"

int currentSec = random(0, 60);
int currentMin = random(0, 60);
int currentHour = random(0, 24);
int currentDay = random(1, 30);
int currentMonth = random(0, 11);

extern bool bleEnabled;        //@TODO this is a crap!!!
extern bool bleServiceRunning; //@TODO this is a crap!!!
extern bool blePeer;

// network timed tasks
NetworkTaskDescriptor *Watchface2Application::ntpTask = nullptr;
NetworkTaskDescriptor *Watchface2Application::weatherTask = nullptr;
NetworkTaskDescriptor *Watchface2Application::geoIPTask = nullptr;

// NTP sync data
const char *ntpServer = "pool.ntp.org";
// const long  gmtOffset_sec   = 3600;
// const int   daylightOffset_sec = 0; // winter
// const int   daylightOffset_sec = 3600; // summer

bool ntpSyncDone = false;
bool weatherSyncDone = false;

// manipulate the angle data from UI
// extern int16_t downTouchX;
// extern int16_t downTouchY;
// extern int batteryPercent;

//extern const char *openWeatherMapApiKey;
extern const PROGMEM char openWeatherMapApiKey[];

int weatherId = -1;
char *weatherMain = nullptr;
char *weatherDescription = nullptr;
char *weatherIcon = nullptr;
double weatherTemp = -1000;

extern const PROGMEM uint8_t openweatherPEM_start[] asm("_binary_asset_openweathermap_org_pem_start");
extern const PROGMEM uint8_t openweatherPEM_end[] asm("_binary_asset_openweathermap_org_pem_end");

char *geoIPReceivedData = nullptr;
char *weatherReceivedData = nullptr;

char *weatherCity = nullptr;
char *weatherCountry = nullptr;

bool Watchface2Application::GetSecureNetworkWeather() {
    bool oweatherValue = (bool)NVS.getInt("OWeatherEnabled");
    if (false == oweatherValue) {
        lNetLog("Watchface: Openweather Sync disabled by NVS\n");
        return true;
    }
    char *url = (char *)ps_malloc(180); 
    const char urlFormatString[] = "https://api.openweathermap.org/data/2.5/weather?q=%s,%s&units=metric&APPID=%s";
    sprintf(url,urlFormatString,weatherCity,weatherCountry,openWeatherMapApiKey);
    lNetLog("GetSecureNetworkWeather: URL: %s Len: %d\n",url,strlen(url));
    WiFiClientSecure *client = new WiFiClientSecure;
    if(nullptr == client) {
        lNetLog("GetSecureNetworkWeather: ERROR: Unable to create WiFiClientSecure\n");
        return false;
    }
    client -> setCACert((const PROGMEM char *)openweatherPEM_start);
    {
        HTTPClient weatherClient;
        if (false == weatherClient.begin(*client, url)) {  // HTTPS
            lNetLog("GetSecureNetworkWeather: ERROR: Unable to connect!\n");
            delete client;
            return false;
        }
        int httpResponseCode = weatherClient.GET();
        free(url);
        lNetLog("Watchface: GetSecureNetworkWeather: HTTP response code: %d\n", httpResponseCode);
        if (httpResponseCode < 1) {
            weatherClient.end();
            delete client;
            return false;
        }
        // clean old buffer if needed
        if (nullptr != weatherReceivedData) {
            free(weatherReceivedData);
            weatherReceivedData = nullptr;
        }
        String payload = weatherClient.getString(); // fuck strings
        
        lNetLog("Watchface: GetSecureNetworkWeather: free weatherClient\n");
        weatherClient.end();
        delete client;

        // Generate the received data buffer
        if (nullptr == weatherReceivedData) {
            lNetLog("Allocate: weatherReceivedData size: %d\n",payload.length());
            weatherReceivedData = (char *)ps_malloc(payload.length() + 1);
            strcpy(weatherReceivedData, payload.c_str());
        }
    }
    lNetLog("HTTP: %s\n",weatherReceivedData);
    return true;


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
    return false;
    */
}

bool Watchface2Application::ParseWeatherData() {

    if (nullptr == weatherReceivedData) {
        lNetLog("Watchface: ERROR: OpenWeather JSON parsing: Empty string ''\n");
        weatherId = -1;
        return false;
    }
    JSONVar myObject = JSON.parse(weatherReceivedData);
    if (JSON.typeof(myObject) == "undefined")
    {
        lNetLog("Watchface: ERROR: OpenWeather JSON parsing: malformed JSON");
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
    return true;
}

void Watchface2Application::Handlers()
{
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, Watchface2Application::FreeRTOSEventReceived, this);
#ifdef LUNOKIOT_WIFI_ENABLED
    if (nullptr == ntpTask)
    {
        ntpTask = new NetworkTaskDescriptor();
        ntpTask->name = (char *)"NTP Watchface";
        ntpTask->everyTimeMS = ((1000 * 60) * 60) * 24; // once a day
        ntpTask->payload = (void *)this;
        ntpTask->_lastCheck = millis();
        // if ( false == ntpSyncDone ) {
        ntpTask->_nextTrigger = 0; // launch NOW if no synched never again
        //}
        ntpTask->callback = [&, this]()
        {
            if (false == (bool)NVS.getInt("NTPEnabled"))
            {
                lEvLog("Watchface: NTP Sync disabled");
                return true;
            }
            struct tm timeinfo;
            if (getLocalTime(&timeinfo))
            {
                lEvLog("ESP32: Time: %02d:%02d:%02d %02d-%02d-%04d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon, 1900 + timeinfo.tm_year);
            }

            lNetLog("Watchface: Trying to sync NTP time...\n");
            // delay(100);
            // init and get the time
            int daylight = NVS.getInt("summerTime");
            long timezone = NVS.getInt("timezoneTime");
            lEvLog("Watchface: Summer time: %s\n", (daylight ? "true" : "false"));
            lEvLog("Watchface: GMT: %d\n", timezone);
            configTime(timezone * 3600, daylight * 3600, ntpServer);
            // Set offset time in seconds to adjust for your timezone, for example:
            // GMT +1 = 3600
            // GMT +8 = 28800
            // GMT -1 = -3600
            // GMT 0 = 0
            while (false == getLocalTime(&timeinfo, 100)) {
                delay(100);
            }
            lNetLog("Watchface: NTP Received\n");
            if (getLocalTime(&timeinfo))  {
                lEvLog("ESP32: Time: %02d:%02d:%02d %02d-%02d-%04d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon, 1900 + timeinfo.tm_year);
                ttgo->rtc->syncToRtc();
                RTC_Date d = ttgo->rtc->getDateTime();
                lEvLog("RTC: Read RTC: '%s'\n", ttgo->rtc->formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));
                /*
                if (d.year != (timeinfo.tm_year + 1900) || d.month != timeinfo.tm_mon + 1
                                || d.day !=  timeinfo.tm_mday ||  d.hour != timeinfo.tm_hour
                                || d.minute != timeinfo.tm_min) {
                    lLog("RTC: Write Fail: '%s'\n",ttgo->rtc->formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));
                    ntpSyncDone = false;
                    return false;
                }
                */
                lNetLog("NTP sync done!\n");
                ntpSyncDone = true;
                return true;
            }
            return false;
        };
        ntpTask->enabled = (bool)NVS.getInt("NTPEnabled");
        AddNetworkTask(ntpTask);
    }
    bool oweatherValue = (bool)NVS.getInt("OWeatherEnabled");
    if (nullptr == geoIPTask) {
        geoIPTask = new NetworkTaskDescriptor();
        geoIPTask->name = (char *)"GeoIP Watchface";
        geoIPTask->everyTimeMS = (60 * 1000) * 29;
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
            lNetLog("HTTP: %s\n",payload.c_str());

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
                return false;
            }
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
            lNetLog("GeoIP: Country: '%s' City: '%s' (can be so innacurate)\n",weatherCountry,weatherCity);
            return true;
        };
        geoIPTask->enabled = oweatherValue;
        AddNetworkTask(geoIPTask);
    }

    if (nullptr == weatherTask) {
        weatherTask = new NetworkTaskDescriptor();
        weatherTask->name = (char *)"OpenWeather Watchface";
        weatherTask->everyTimeMS = (60 * 1000) * 29;
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
            lNetLog("Openweather KEY; '%s'\n",openWeatherMapApiKey);
            if (0 == strlen(openWeatherMapApiKey)) {
                lNetLog("Watchface: Openweather Cannot get weather without API KEY https://openweathermap.org/api\n");
                return true;
            }
            bool getDone = Watchface2Application::GetSecureNetworkWeather();
            lLog("@TODO THIS PART IS DISABLED\n");
            return true;

            if (getDone) {
                bool parseDone = Watchface2Application::ParseWeatherData();
                weatherSyncDone = parseDone;
                return parseDone;
            }
            return getDone;
        };
        weatherTask->enabled = oweatherValue;
        weatherTask->desiredStack = LUNOKIOT_PROVISIONING_STACK_SIZE;
        AddNetworkTask(weatherTask);
    }
#endif
}

Watchface2Application::~Watchface2Application()
{
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, Watchface2Application::FreeRTOSEventReceived);
    // directDraw=false;
    delete bottomRightButton;
    delete hourHandCanvas;
    delete minuteHandCanvas;
    delete innerSphere;
    delete outherSphere;
    delete colorBuffer;
    lAppLog("Watchface is gone\n");
}

Watchface2Application::Watchface2Application()
{
    //@TODO Theme it ... ThCol(aaa)

    middleX = canvas->width() / 2;
    middleY = canvas->height() / 2;
    radius = (canvas->width() + canvas->height()) / 4;

    bottomRightButton = new ActiveRect(160, 160, 80, 80, [&, this]()
                                       { LaunchApplication(new MainMenuApplication()); });

    colorBuffer = new CanvasZWidget(canvas->width(), canvas->height());
    colorBuffer->canvas->fillSprite(TFT_BLACK);
    outherSphere = new CanvasZWidget(canvas->width(), canvas->height());
    outherSphere->canvas->fillSprite(TFT_BLACK);

    hourHandCanvas = new CanvasZWidget(img_hours_hand.height, img_hours_hand.width);
    hourHandCanvas->canvas->pushImage(0, 0, img_hours_hand.width, img_hours_hand.height, (uint16_t *)img_hours_hand.pixel_data);
    hourHandCanvas->canvas->setPivot(12, 81);

    minuteHandCanvas = new CanvasZWidget(img_minutes_hand.height, img_minutes_hand.width);
    minuteHandCanvas->canvas->pushImage(0, 0, img_minutes_hand.width, img_minutes_hand.height, (uint16_t *)img_minutes_hand.pixel_data);
    minuteHandCanvas->canvas->setPivot(12, 107);

    // Buttons
    outherSphere->canvas->fillRect(0, 0, middleX, middleY, tft->color24to16(0x000408));
    outherSphere->canvas->fillRect(middleX, 0, middleX, middleY, tft->color24to16(0x000408));
    outherSphere->canvas->fillRect(0, middleY, middleX, middleY, tft->color24to16(0x000408));
    outherSphere->canvas->fillRect(middleX, middleY, middleX, middleY, tft->color24to16(0x102048));
    outherSphere->canvas->fillCircle(middleX, middleY, radius + (margin * 2), TFT_BLACK);

    // outher circle
    outherSphere->canvas->fillCircle(middleX, middleY, radius - margin, tft->color24to16(0x384058));
    outherSphere->canvas->fillCircle(middleX, middleY, radius - (margin * 2), TFT_BLACK);

    DescribeCircle(middleX, middleY, radius - (margin * 2.5),
                   [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload)
                   {
                       if (3 == (angle % 6))
                       { // dark bites
                           outherSphere->canvas->fillCircle(x, y, 5, TFT_BLACK);
                       }
                       return true;
                   });
    DescribeCircle(middleX, middleY, radius - (margin * 2),
                   [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload)
                   {
                       if (0 == (angle % 30))
                       { // blue 5mins bumps
                           outherSphere->canvas->fillCircle(x, y, 5, tft->color24to16(0x384058));
                       }
                       return true;
                   });
    outherSphere->canvas->fillRect(0, middleY - 40, 40, 80, TFT_BLACK); // left cut

    outherSphere->DrawTo(colorBuffer->canvas, 0, 0, 1.0, false, TFT_BLACK);

    // inner quarter
    innerSphere = new CanvasZWidget(canvas->width() / 2, canvas->height() / 2, 1.0, 8);
    innerSphere->canvas->fillSprite(TFT_BLACK);
    innerSphere->canvas->fillCircle(0, 0, 95, tft->color24to16(0x182858));
    innerSphere->canvas->fillCircle(0, 0, 45, TFT_BLACK);
    innerSphere->DrawTo(colorBuffer->canvas, middleX, middleY, 1.0, false, TFT_BLACK);

    colorBuffer->DrawTo(canvas, 0, 0); // for splash
    // directDraw=true;
    Handlers();
}

void Watchface2Application::FreeRTOSEventReceived(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    Watchface2Application *self = reinterpret_cast<Watchface2Application *>(handler_args);
    if (WIFI_EVENT == base) {
        if (WIFI_EVENT_STA_START == id) {
            self->wifiEnabled = true;
        }
        else if (WIFI_EVENT_STA_STOP == id) {
            self->wifiEnabled = false;
        }
    }
}

bool Watchface2Application::Tick()
{
    bottomRightButton->Interact(touched, touchX, touchY);

    if (millis() > nextRefresh)
    {
        // hourHandCanvas->DrawTo(canvas,middleX,middleY);
        // minuteHandCanvas->DrawTo(canvas,middleX,middleY);
        // directDraw=false;
        colorBuffer->DrawTo(canvas, 0, 0);

        // time mark
        DescribeCircle(middleX, middleY, radius - 1,
                       [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload)
                       {
                           int angleDisplaced = (markAngle + 90) % 360;
                           if ((angle > markAngle) && (angle < angleDisplaced))
                           {
                               canvas->fillCircle(x, y, 1, tft->color24to16(0x5890e8));
                           }
                           return true;
                       });

        time_t now;
        struct tm *timeinfo;
        time(&now);
        timeinfo = localtime(&now);
        char *textBuffer = (char *)ps_malloc(255);

        currentSec = timeinfo->tm_sec;
        currentMin = timeinfo->tm_min;
        currentHour = timeinfo->tm_hour;
        currentDay = timeinfo->tm_mday;
        currentMonth = timeinfo->tm_mon;

        // lAppLog("TIMEINFO: %02d:%02d:%02d\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);

        if (weatherSyncDone)
        {
            // weather icon
            canvas->setBitmapColor(ThCol(mark), TFT_BLACK);
            // watchFaceCanvas->canvas->fillRect(120 - (img_weather_200.width/2),52,80,46,TFT_BLACK);
            if (-1 != weatherId)
            {
                if ((200 <= weatherId) && (300 > weatherId))
                {
                    canvas->pushImage(120 - (img_weather_200.width / 2), 52, img_weather_200.width, img_weather_200.height, (uint16_t *)img_weather_200.pixel_data);
                }
                else if ((300 <= weatherId) && (400 > weatherId))
                {
                    canvas->pushImage(120 - (img_weather_300.width / 2), 52, img_weather_300.width, img_weather_300.height, (uint16_t *)img_weather_300.pixel_data);
                }
                else if ((500 <= weatherId) && (600 > weatherId))
                {
                    canvas->pushImage(120 - (img_weather_500.width / 2), 52, img_weather_500.width, img_weather_500.height, (uint16_t *)img_weather_500.pixel_data);
                }
                else if ((600 <= weatherId) && (700 > weatherId))
                {
                    canvas->pushImage(120 - (img_weather_600.width / 2), 52, img_weather_600.width, img_weather_600.height, (uint16_t *)img_weather_600.pixel_data);
                }
                else if ((700 <= weatherId) && (800 > weatherId))
                {
                    //lAppLog("@TODO Watchface: openweather 700 condition code\n");
                    // watchFaceCanvas->canvas->pushImage(120 - (img_weather_800.width/2) ,52,img_weather_800.width,img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
                    canvas->pushImage(120 - (img_weather_800.width / 2), 52, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
                    // watchFaceCanvas->canvas->pushImage(144,52,img_weather_600.width,img_weather_600.height, (uint16_t *)img_weather_600.pixel_data);
                }
                else if ((800 <= weatherId) && (900 > weatherId))
                {
                    canvas->pushImage(120 - (img_weather_800.width / 2), 52, img_weather_800.width, img_weather_800.height, (uint16_t *)img_weather_800.pixel_data);
                }
            }
            // temperature
            if (-1000 != weatherTemp)
            {
                sprintf(textBuffer, "%2.1fc", weatherTemp);
                int16_t posX = 150;
                int16_t posY = 74;
                canvas->setTextFont(0);
                canvas->setTextSize(2);
                canvas->setTextColor(TFT_WHITE);
                canvas->setTextDatum(TL_DATUM);
                canvas->drawString(textBuffer, posX, posY); //, 185, 96);
            }

            if (nullptr != weatherMain)
            {
                canvas->setTextFont(0);
                canvas->setTextSize(2);
                canvas->setTextDatum(CC_DATUM);
                canvas->setTextWrap(false, false);
                canvas->setTextColor(TFT_WHITE);
                canvas->drawString(weatherMain, 120, 40);
            }

            if (nullptr != weatherDescription)
            {
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

        // connectivity notifications
        if (bleEnabled)
        {
            int16_t posX = 36;
            int16_t posY = 171;
            uint32_t dotColor = TFT_DARKGREY; // enabled but service isn't up yet
            if (bleServiceRunning)
            {
                dotColor = ThCol(medium);
            }
            if (blePeer)
            {
                dotColor = ThCol(low);
            }
            canvas->fillCircle(posX, posY, 5, dotColor);
            unsigned char *img = img_bluetooth_24_bits; // bluetooth logo only icon
            if (blePeer)
            {
                img = img_bluetooth_peer_24_bits;
            } // bluetooth with peer icon
            canvas->drawXBitmap(posX + 10, posY - 12, img, img_bluetooth_24_width, img_bluetooth_24_height, ThCol(text));
        }
        if (wifiEnabled)
        {
            int16_t posX = 51;
            int16_t posY = 189;
            canvas->fillCircle(posX, posY, 5, ThCol(low));
            canvas->drawXBitmap(posX + 10, posY - 12, img_wifi_24_bits, img_wifi_24_width, img_wifi_24_height, ThCol(text));
        }

        // seconds hand
        float secAngle = (timeinfo->tm_sec * 6);
        DescribeCircle(middleX, middleY, 110,
                       [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload)
                       {
                           if (int(secAngle) == angle)
                           {
                               canvas->fillCircle(x, y, 5, TFT_BLACK);
                               canvas->drawLine(x, y, cx, cy, ThCol(clock_hands_second));
                               canvas->fillCircle(x, y, 4, ThCol(clock_hands_second));
                               return false;
                           }
                           return true;
                       });

        float minuteAngle = (timeinfo->tm_min * 6);
        // hour hand
        float hourAngle = ((timeinfo->tm_hour) % 12) * 30;
        float correctedHoureAngle = hourAngle + (minuteAngle / 12.0);
        // lAppLog("hourAngle: %f\n",correctedHoureAngle);
        DescribeCircle(middleX, middleY, 90,
                       [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload)
                       {
                           if (int(correctedHoureAngle) == angle)
                           {
                               canvas->fillCircle(x, y, 9, TFT_BLACK);
                               canvas->drawLine(x, y, cx, cy, ThCol(highlight));
                               canvas->fillCircle(x, y, 8, ThCol(highlight));
                               // canvas->drawLine(x,y,cx,cy, tft->color24to16(0xa0a8c0));
                               // canvas->fillCircle(x,y,8,tft->color24to16(0xa0a8c0));
                               return false;
                           }
                           return true;
                       });

        // minutes hand
        float correctedMinuteAngle = minuteAngle + (secAngle / 60.0);
        DescribeCircle(middleX, middleY, 100,
                       [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload)
                       {
                           if (int(correctedMinuteAngle) == angle)
                           {
                               canvas->fillCircle(x, y, 7, TFT_BLACK);
                               canvas->drawLine(x, y, cx, cy, tft->color24to16(0x787ca0));
                               canvas->fillCircle(x, y, 6, tft->color24to16(0x787ca0));
                               return false;
                           }
                           return true;
                       });
        canvas->fillCircle(middleX, middleY, 10, ThCol(clock_hands_second)); // second cap
        canvas->fillCircle(middleX, middleY, 9, TFT_BLACK);
        canvas->fillCircle(middleX, middleY, 8, tft->color24to16(0x787ca0)); // minute cap
        canvas->fillCircle(middleX, middleY, 4, TFT_BLACK);
        canvas->fillCircle(middleX, middleY, 3, ThCol(highlight)); // hour cap

        // time
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextWrap(false, false);
        sprintf(textBuffer, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);

        // canvas->setTextColor(TFT_BLACK);
        // canvas->setTextDatum(CR_DATUM);
        // canvas->drawString(textBuffer, middleX-22,middleY+2);
        // canvas->drawString(textBuffer, middleX-18,middleY-2);

        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(CR_DATUM);
        canvas->drawString(textBuffer, middleX - 20, middleY);

        // date
        sprintf(textBuffer, "%02d/%02d", timeinfo->tm_mday, timeinfo->tm_mon + 1);
        canvas->setTextDatum(BL_DATUM);
        // canvas->setTextColor(TFT_BLACK);
        // canvas->drawString(textBuffer, middleX+22,middleY+2);
        // canvas->drawString(textBuffer, middleX+18,middleY-2);
        canvas->setTextColor(TFT_WHITE);
        canvas->drawString(textBuffer, middleX + 35, middleY - 15);

        // steps
        uint32_t activityColor = TFT_WHITE;
        bool doingSomething = false;
        int16_t posX = 40;
        int16_t posY = 74;
        if (vbusPresent)
        {
            activityColor = TFT_DARKGREY;
        }
        else if (nullptr != currentActivity)
        {
            if (0 == strcmp("BMA423_USER_WALKING", currentActivity))
            {
                activityColor = TFT_GREEN;
                doingSomething = true;
            }
            else if (0 == strcmp("BMA423_USER_RUNNING", currentActivity))
            {
                activityColor = TFT_YELLOW;
                doingSomething = true;
            }
            if (doingSomething)
            {
                canvas->fillCircle(posX - 10, posY + 6, 5, activityColor);
            }
        }
        canvas->setTextColor(activityColor);
        canvas->setTextSize(2);
        canvas->setTextDatum(TL_DATUM);
        sprintf(textBuffer, "%d", stepCount);
        canvas->drawString(textBuffer, posX, posY);

        if (vbusPresent)
        {
            posX = 69;  // 60;
            posY = 203; // 190;
            uint32_t color = ThCol(background);
            if (ttgo->power->isChargeing())
            {
                color = ThCol(low);
            }
            canvas->fillCircle(posX, posY, 5, color);
            canvas->drawXBitmap(posX + 10, posY - 12, img_usb_24_bits, img_usb_24_width, img_usb_24_height, TFT_WHITE);
        }

        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextDatum(TL_DATUM);
        canvas->setTextWrap(false, false);

        // batt pc
        if (batteryPercent > -1)
        {
            uint32_t battColor = TFT_DARKGREY;
            posX = 26;
            posY = 149;
            bool battActivity = false;
            if (vbusPresent)
            {
                battColor = TFT_GREEN;
                battActivity = true;
            }
            else if (batteryPercent < 10)
            {
                battActivity = true;
                battColor = ThCol(high);
            }
            else if (batteryPercent < 35)
            {
                battColor = TFT_YELLOW;
            }
            // else if ( batteryPercent > 70 ) { battColor = TFT_GREEN; }
            canvas->setTextColor(battColor);
            int battFiltered = batteryPercent;
            if (battFiltered > 99)
            {
                battFiltered = 99;
            }
            sprintf(textBuffer, "%2d%%", battFiltered);
            canvas->setTextDatum(CL_DATUM);
            canvas->drawString(textBuffer, posX + 10, posY + 1);
            canvas->fillCircle(posX, posY, 5, battColor); // Alarm: red dot
        }
        else
        {
            uint32_t battColor = TFT_DARKGREY;
            posX = 26;
            posY = 149;
            canvas->setTextFont(0);
            canvas->setTextSize(2);
            canvas->setTextColor(battColor);
            canvas->setTextDatum(CL_DATUM);
            canvas->drawString("No batt", posX + 10, posY + 1);
            canvas->fillCircle(posX, posY, 5, ThCol(high)); // Alarm: red dot
        }
        free(textBuffer);
        const int16_t margin = 15;
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextColor(ThCol(mark));
        canvas->setTextDatum(TC_DATUM);
        canvas->drawString("12", TFT_WIDTH / 2, margin);
        canvas->setTextDatum(CR_DATUM);
        canvas->drawString("3", TFT_WIDTH - margin, TFT_HEIGHT / 2);
        canvas->setTextDatum(BC_DATUM);
        canvas->drawString("6", TFT_WIDTH / 2, TFT_HEIGHT - margin);
        canvas->setTextDatum(CL_DATUM);
        canvas->drawString("9", margin, TFT_HEIGHT / 2);

        nextRefresh = millis() + (1000 / 4);
        return true;
    }
    return false;
}
