//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//
// LunokWatch is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later 
// version.
//
// LunokWatch is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
// details.
//
// You should have received a copy of the GNU General Public License along with 
// LunokWatch. If not, see <https://www.gnu.org/licenses/>. 
//

#include "Weather.hpp"
#include "../../../app/LogView.hpp"
#include <ArduinoNvs.h>
#include <Arduino_JSON.h>
#include <HTTPClient.h>
#include "../../Datasources/database.hpp"

const char urlOpenWeatherFormatString[] = "http://api.openweathermap.org/data/2.5/weather?q=%s,%s&units=metric&appid=%s";

bool weatherSyncDone = false;

char *weatherCity = nullptr;
char *weatherCountry = nullptr;
char *weatherReceivedData = nullptr;
int weatherId = -1;
char *weatherMain = nullptr;
char *weatherDescription = nullptr;
char *weatherIcon = nullptr;
double weatherTemp = -1000;

double weatherFeelsTemp = -1000;
double weatherTempMin = -1000;
double weatherTempMax = -1000;
double weatherHumidity = 0;
double weatherPressure = 0;
double weatherWindSpeed = 0;

String WeatherWifiTask::NetworkHTTPGETRequest(const char* serverName) {
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

bool WeatherWifiTask::NetworkGetSecureWeather() {
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
}

bool WeatherWifiTask::NetworkParseWeatherData() {
    if (nullptr == weatherReceivedData) {
        lNetLog("Watchface: ERROR: OpenWeather JSON parsing: Empty string ''\n");
        weatherId = -1;
        return false;
    }
    JSONVar myObject = JSON.parse(weatherReceivedData);
    if (JSON.typeof(myObject) == "undefined") {
        lNetLog("Watchface: ERROR: OpenWeather JSON parsing: malformed JSON\n");
        return false;
    }
    if (false == myObject.hasOwnProperty("weather")) {
        lNetLog("Watchface: ERROR: OpenWeather JSON parsing: property 'weather' not found\n");
        weatherId = -1;
        return false;
    }
    JSONVar weatherBranch = myObject["weather"][0];

    if (false == weatherBranch.hasOwnProperty("description")) {
        lNetLog("Watchface: ERROR: OpenWeather JSON parsing: property '[weather][0][description]' not found\n");
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
    const char * filebase = "/littlefs/oweather.log";
    FILE *pFile = fopen(filebase, "a");
    const char * asChar = jsonString.c_str();
    fwrite(asChar,sizeof(char),strlen(asChar),pFile);
    fwrite("\n",sizeof(char),strlen("\n"),pFile);
    fclose(pFile);
    //SqlJSONLog("oweather",jsonString.c_str());

    return true;
}
void WeatherWifiTask::Launch() {
    weatherSyncDone = false;
    bool oweatherValue = (bool)NVS.getInt("OWeatherEnabled");
    if (false == oweatherValue) {
        lNetLog("Task: %p '%s' User settings don't agree\n",this,Name());
        return;
    }
    lNetLog("Task: %p '%s' key: '%s'\n",this,Name(),openWeatherMapApiKey);
    if (0 == strlen(openWeatherMapApiKey)) {
        lNetLog("Task: %p '%s' ERROR: Invalid key: '%s'\n",this,Name(),openWeatherMapApiKey);
        return;
    }
    bool getDone = NetworkGetSecureWeather();

    if (getDone) {
        bool parseDone = NetworkParseWeatherData();
        weatherSyncDone = parseDone;
        return;
    }
}

bool WeatherWifiTask::Check() {
    if ( nextShoot > millis() ) { return false; }
    nextShoot=millis()+Every60M;
    if (false == (bool)NVS.getInt("OWeatherEnabled")) { return false; }
    return true;
}
