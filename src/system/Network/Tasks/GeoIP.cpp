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

#include "GeoIP.hpp"
#include "../../../app/LogView.hpp"
#include <ArduinoNvs.h>
#include <Arduino_JSON.h>
#include <HTTPClient.h>
#include "../../Datasources/database.hpp"

extern char *weatherCity;
extern char *weatherCountry;
char *geoIPReceivedData = nullptr;

void GeoIPWifiTask::Launch() {
    // @TODO use NVS their own setting
    if (false == (bool)NVS.getInt("OWeatherEnabled")) {
        lNetLog("Task: %p '%s' User settings don't agree\n",this,Name());
        return;
    }
    HTTPClient geoIPClient;
    geoIPClient.begin(url);
    int httpResponseCode = geoIPClient.GET();
    if (httpResponseCode < 1) {
        lNetLog("Task: %p '%s' ERROR: http response: %d\n",this,Name(),httpResponseCode);
        geoIPClient.end();
        return;
    }
    lNetLog("Task: %p '%s' http response code: '%d'\n",this,Name(),httpResponseCode);
    // clean old buffer if needed
    if (nullptr != geoIPReceivedData) {
        free(geoIPReceivedData);
        geoIPReceivedData = nullptr;
    }
    String payload = geoIPClient.getString(); // fuck strings
    
    lNetLog("Task: %p '%s' free http client\n",this,Name());
    geoIPClient.end();

    // Generate the received data buffer
    lNetLog("Task: %p '%s' Allocate data size %d\n",this,Name(),payload.length());
    geoIPReceivedData = (char *)ps_malloc(payload.length() + 1);
    strcpy(geoIPReceivedData, payload.c_str());

    lNetLog("Task: %p '%s' Parsing response...\n",this,Name());
    // process response
    JSONVar myObject = JSON.parse(geoIPReceivedData);
    if (JSON.typeof(myObject) == "undefined") {
        lNetLog("Task: %p '%s' Unable to parse JSON\n",this,Name());
        lNetLog("Task: %p '%s' http raw: '%s'\n",this,Name(),payload.c_str());
        return;
    
    }
    //delete myObject["geoplugin_credit"];
    myObject["geoplugin_credit"] = undefined; // sorry,so expensive credits :( thanks geoIP!!!! :**
    //delete myObject["geoplugin_currencySymbol_UTF8"];
    myObject["geoplugin_currencySymbol_UTF8"] = undefined; // no utf problems please!
    //delete myObject["geoplugin_currencySymbol"];
    myObject["geoplugin_currencySymbol"] = undefined; // no utf problems please!



    //lLog("JSON:\n%s\n",myObject.stringify(myObject).c_str());
    if (false == myObject.hasOwnProperty("geoplugin_city")) {
        lNetLog("Task: %p '%s' Unable to parse JSON 'geoplugin_city'\n",this,Name());
        return;
    }
    if (false == myObject.hasOwnProperty("geoplugin_countryCode")) {
        lNetLog("Task: %p '%s' Unable to parse JSON 'geoplugin_countryCode'\n",this,Name());
        return;
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
    lNetLog("Task: %p '%s' Country '%s' City: '%s'\n",this,Name(),weatherCountry,weatherCity);

    String jsonString = myObject.stringify(myObject);
    const char * filebase = "/littlefs/geoip.log";
    FILE *pFile = fopen(filebase, "a");
    const char * asChar = jsonString.c_str();
    fwrite(asChar,sizeof(char),strlen(asChar),pFile);
    fwrite("\n",sizeof(char),strlen("\n"),pFile);
    fclose(pFile);
    //SqlJSONLog("geoip",jsonString.c_str());
    return;
}

bool GeoIPWifiTask::Check() {
    if ( nextShoot > millis() ) { return false; }
    nextShoot=millis()+Every3H;
    if (false == (bool)NVS.getInt("OWeatherEnabled")) { return false; }
    return true;
}
