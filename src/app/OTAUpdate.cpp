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

#include <Arduino.h>
#include "LogView.hpp" // log capabilities
#include "Shutdown.hpp" // call shutdown

#include <WiFi.h>
#include "../UI/AppTemplate.hpp"
#include "OTAUpdate.hpp"
#include "../system/SystemEvents.hpp"
#include "esp_wifi.h"
#include <esp_https_ota.h>

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GaugeWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"

#include "../static/img_update_48.xbm"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

#include "../static/img_wifi_24.xbm"

#include "../lunokIoT.hpp"
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo library shit ;)


esp_https_ota_handle_t https_ota_handle = NULL;

char * latestBuildFoundString = nullptr;
#ifdef LUNOKIOT_UPDATES_LOCAL_URL
extern const PROGMEM uint8_t githubPEM_start[] asm("_binary_asset_server_pem_start");
extern const PROGMEM uint8_t githubPEM_end[] asm("_binary_asset_server_pem_end");
#else
extern const PROGMEM uint8_t githubPEM_start[] asm("_binary_asset_raw_githubusercontent_com_pem_start");
extern const PROGMEM uint8_t githubPEM_end[] asm("_binary_asset_raw_githubusercontent_com_pem_end");
#endif

//int OTAbytesPerSecond=0;
#define OTASTEP_IDLE -1
#define OTASTEP_NO_WIFI 0
#define OTASTEP_ALREADYUPDATED 1
#define OTASTEP_CONNECTING 2
#define OTASTEP_ONLINE 3
#define OTASTEP_CHECKING 3
#define OTASTEP_HTTPS_ERROR 4 
#define OTASTEP_IMAGE_ERROR 5 
#define OTASTEP_IMAGE_DOWNLOAD 6 
#define OTASTEP_IMAGE_DOWNLOAD_ERROR 7 
#define OTASTEP_IMAGE_DOWNLOAD_DONE 8

int8_t OTAStep=OTASTEP_IDLE;

GraphWidget * OTADownloadSpeed=nullptr;
long OTADownloadTimeLapse=0;
int32_t OTADownloadSample=0;

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    if ( nullptr == OTADownloadSpeed ) {
        return ESP_OK;
    }
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        lLog("HTTP_EVENT_ERROR\n");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        //lAppLog("HTTP_EVENT_ON_CONNECTED\n");
        break;
    case HTTP_EVENT_HEADER_SENT:
        //lAppLog("HTTP_EVENT_HEADER_SENT\n");
        break;
    case HTTP_EVENT_ON_HEADER:
        //lAppLog("HTTP_EVENT_ON_HEADER, key=%s, value=%s\n", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        //lAppLog("HTTP_EVENT_ON_DATA, len=%d\n", evt->data_len);
        if ( millis() > OTADownloadTimeLapse ) {
            OTADownloadSpeed->PushValue(0); // add space
            OTADownloadSpeed->PushValue(OTADownloadSample);
            OTADownloadSpeed->PushValue(0); // add space
            //lAppLog("OTADownloadSample: %d\n",OTADownloadSample);
            OTADownloadSample=0;
            OTADownloadTimeLapse=millis()+100;
        } else {
            OTADownloadSample+=evt->data_len;
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        lLog("HTTP_EVENT_ON_FINISH\n");
        break;
    case HTTP_EVENT_DISCONNECTED:
        lLog("HTTP_EVENT_DISCONNECTED\n");
        OTAStep=OTASTEP_NO_WIFI;
        break;
    }
    return ESP_OK;
}

OTAUpdateApplication::OTAUpdateApplication() {
    OTAStep=OTASTEP_IDLE;

    canvas->fillSprite(backgroundColor); // use theme colors
    OTADownloadSpeed = new GraphWidget(40,180,0,8*1024,TFT_BLUE,Drawable::MASK_COLOR,TFT_YELLOW);
    progressBar = new GaugeWidget(55, 45, 130);

    updateBtn = new ButtonImageXBMWidget(120-50,120-50, 100, 100, [this](void *bah){
        updateBtn->SetEnabled(false);
        OTAStep=OTASTEP_IDLE;
        backgroundColor=ThCol(background);
        if ( nullptr == latestBuildFoundString ) {
            lAppLog("OTA: WiFi working?\n");
            OTAStep=OTASTEP_NO_WIFI;
            backgroundColor=TFT_RED;
            updateBtn->SetEnabled(true);
            return;
        }
        if ( nullptr == latestBuildFoundString ) {
            updateBtn->SetEnabled(true);
            lAppLog("OTA: need check first\n");
            return;
        }
        uint32_t myVersion = LUNOKIOT_BUILD_NUMBER;
        uint32_t remoteVersion = atoi(latestBuildFoundString);
        if ( remoteVersion <= myVersion ) { // already have the latest version :)
            updateBtn->SetEnabled(true);
            OTAStep=OTASTEP_ALREADYUPDATED;
            return;
        }
        OTAStep=OTASTEP_CONNECTING;
        lAppLog("OTA: Trying to get WiFi...\n");
        LoT().GetWiFi()->Disable();
        WiFi.begin();
        esp_wifi_set_ps(WIFI_PS_NONE); // disable wifi powersave
        unsigned long timeout = millis()+(25*1000);
        while ( timeout > millis() ) {
            wl_status_t currStat = WiFi.status();
            if ( WL_CONNECTED == currStat) { break; }
            delay(100);
        }
        if ( millis() > timeout ) {
            lAppLog("OTA: Timeout trying to start WiFi!\n");
            backgroundColor=TFT_RED;
            updateBtn->SetEnabled(true);
            OTAStep=OTASTEP_NO_WIFI;
            return;
        }
        lAppLog("OTA: WiFi connected\n");
        OTAStep=OTASTEP_ONLINE;
        delay(500); // take your time
        esp_err_t ota_finish_err = ESP_OK;
        // -DLUNOKIOT_UPDATES_ENABLED
        lAppLog("OTA: System update availiable: '%s' running: '%s'\n",latestBuildFoundString,LUNOKIOT_BUILD_STRING);
        // implement https OTA https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-reference/system/esp_https_ota.html
        char *firmwareURL = (char *)ps_malloc(512);

        // device ID's
        const char * myDeviceName = "esp32"; // generic;
        #ifdef LILYGO_WATCH_2020_V3
        myDeviceName = "ttgo-t-watch_2020_v3";
        #endif
        #ifdef LILYGO_WATCH_2020_V2
        myDeviceName = "ttgo-t-watch_2020_v2"; 
        #endif
        #ifdef LILYGO_WATCH_2020_V1
        myDeviceName = "ttgo-t-watch_2020_v1"; 
        #endif

        FreeSpace();
        #ifdef LUNOKIOT_UPDATES_LOCAL_URL
        lAppLog("OTA: Using local URL for updates\n");
        //sprintf(firmwareURL,"%s/ota/lunokWatch_%s_%s.bin",LocalOTAURL,latestBuildFoundString,myDeviceName);

        sprintf(firmwareURL,"%s/ota/lunokWatch_%s_%s.bin",LUNOKIOT_UPDATE_LOCAL_URL_STRING,latestBuildFoundString,myDeviceName);
        #else
        sprintf(firmwareURL,"https://raw.githubusercontent.com/lunokjod/watch/devel/ota/lunokWatch_%s_%s.bin",latestBuildFoundString,myDeviceName);
        #endif
        esp_http_client_config_t config = {
            .url = firmwareURL,
            .cert_pem = (const char *)githubPEM_start,
            .user_agent = "Wget/1.21.2",
            .timeout_ms = 5000,
            .event_handler = _http_event_handler,
            #ifdef LUNOKIOT_UPDATES_LOCAL_URL
            // .transport_type = HTTP_TRANSPORT_OVER_TCP,
            .skip_cert_common_name_check=true,
            #endif
            .keep_alive_enable = true,
        };
        OTAStep=OTASTEP_CHECKING;
        lAppLog("OTA: Getting new firmware from: '%s'...\n",firmwareURL);
        esp_https_ota_config_t ota_config = {
            .http_config = &config,
        };
        esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
        if (err != ESP_OK) {
            lAppLog("OTA: ESP HTTPS OTA Begin failed: %s (%d)\n",esp_err_to_name(err),err);
            backgroundColor=TFT_RED;
            OTAStep=OTASTEP_HTTPS_ERROR;
            goto ota_end;
        }
        esp_app_desc_t app_desc;
        err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
        if (err != ESP_OK) {
            lAppLog("OTA: esp_https_ota_read_img_desc failed\n");
            backgroundColor=TFT_RED;
            OTAStep=OTASTEP_IMAGE_ERROR;
            goto ota_end;
        }

        /*
        err = validate_image_header(&app_desc);
        if (err != ESP_OK) {
            lAppLog("image header verification failed\n");
            goto ota_end;
        }*/
        while (true) {
            err = esp_https_ota_perform(https_ota_handle);
            if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
                break;
            }
            // esp_https_ota_perform returns after every read operation which gives user the ability to
            // monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of image
            // data read so far.
            imageSize=esp_https_ota_get_image_size(https_ota_handle);
            imageDownloaded=esp_https_ota_get_image_len_read(https_ota_handle);
            OTAStep=OTASTEP_IMAGE_DOWNLOAD;
            //lAppLog("OTA: Image bytes read: %d/%d\n",imageDownloaded,imageSize);
        }

        if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
            // the OTA image was not completely received and user can customise the response to this situation.
            lAppLog("OTA: Complete data was not received.\n");
            backgroundColor=TFT_RED;
            OTAStep=OTASTEP_IMAGE_DOWNLOAD_ERROR;
        }
ota_end:
        ota_finish_err = esp_https_ota_finish(https_ota_handle);
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
            //https_ota_handle=NULL;
            OTAStep=OTASTEP_IMAGE_DOWNLOAD_DONE;
            delay(100);
            WiFi.disconnect();
            delay(100);
            WiFi.mode(WIFI_OFF);
            esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
            LoT().GetWiFi()->Enable();
            lAppLog("OTA: ESP_HTTPS_OTA upgrade successful. Rebooting ...\n");
            LaunchApplication(new ShutdownApplication(true,true));
            return;
        } else {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                lAppLog("OTA: Image validation failed\n");
                backgroundColor=TFT_RED;
                OTAStep=OTASTEP_IMAGE_DOWNLOAD_ERROR;
            }
            lAppLog("OTA: ESP_HTTPS_OTA upgrade failed %d\n", ota_finish_err);
            //esp_https_ota_abort(https_ota_handle);
            backgroundColor=TFT_RED;
            //https_ota_handle=NULL;
        }
        delay(100);
        WiFi.disconnect();
        delay(100);
        WiFi.mode(WIFI_OFF);
        esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
        LoT().GetWiFi()->Enable();
        updateBtn->SetEnabled(true);
        free(firmwareURL);
        OTAStep=OTASTEP_IDLE;
        FreeSpace();
    },img_update_48_bits, img_update_48_height, img_update_48_width); //,TFT_WHITE,TFT_BLACK,false);
    updateBtn->taskStackSize=LUNOKIOT_NETWORK_STACK_SIZE;
    
    //updateBtn->SetEnabled(false);
    //if ( nullptr != latestBuildFoundString ) { updateBtn->SetEnabled(true); }

    canvas->setTextFont(0);
    //canvas->setTextColor(ThCol(dark));
    canvas->setTextSize(4);
    canvas->setTextDatum(CC_DATUM);
    bufferForText=(char*)ps_malloc(256);
    if ( nullptr == latestBuildFoundString ) { LaunchWatchface(true); }

    #ifdef LILYGO_WATCH_2020_V3
        ttgo->shake();
    #endif
    imageSize=0;
    imageDownloaded=0;

    Tick();
}

OTAUpdateApplication::~OTAUpdateApplication() {
    if ( NULL != https_ota_handle ) {
        esp_https_ota_abort(https_ota_handle);
        //https_ota_handle=NULL;
    }
    delete updateBtn;
    delete progressBar;
    delete OTADownloadSpeed;
    OTADownloadSpeed=nullptr;
    free(bufferForText);
    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_OFF);
    esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
    OTAStep=OTASTEP_IDLE;
    LoT().GetWiFi()->Enable();
}


bool OTAUpdateApplication::Tick() {
    btnBack->SetEnabled( updateBtn->GetEnabled() );
    
    btnBack->Interact(touched,touchX, touchY); 
    
    updateBtn->Interact(touched,touchX, touchY); 

    UINextTimeout = millis()+UITimeout;  // dont allow screen sleep          

    if ( millis() > nextRefresh ) {
        canvas->fillSprite(backgroundColor); // use theme colors

        wl_status_t whatBoutWifi = WiFi.status();
        if (WL_NO_SHIELD != whatBoutWifi) {
            int16_t posX = TFT_WIDTH-38;
            int16_t posY = 18;
            uint32_t dotColor = ThCol(background);
            if ( WL_CONNECTED == whatBoutWifi ) { dotColor = ThCol(low); }            
            canvas->fillCircle(posX, posY, 5, dotColor);
            canvas->drawXBitmap(posX + 10, posY - 12, img_wifi_24_bits, img_wifi_24_width, img_wifi_24_height, ThCol(text));
        }



        //imageSize=360;
        //imageDownloaded=180;
        if ( ( imageSize > 0 ) && (imageDownloaded > 0 )) {
            
            int16_t newAngle = (imageDownloaded*360)/imageSize;
            if ( progressBar->selectedAngle != newAngle) {
                progressBar->dirty=true; // force redraw of gauge (used as progressbar here)
                progressBar->selectedAngle= newAngle;
                if ( progressBar->selectedAngle > 359 ) {
                    progressBar->selectedAngle=0;
                }
                int percentDone=(imageDownloaded*100)/imageSize;
                sprintf(percentAsString,"%d%%", percentDone);
                //lAppLog("PERCENT: '%s'\n", percentAsString);
                if ( imageSize > (1024*1024) ) { sprintf(bufferForText,"%.2f MB / %.2f MB", float(imageDownloaded/1024.0/1024), float(imageSize/1024.0/1024.0)); }
                else if ( imageSize > 1024 ) { sprintf(bufferForText,"%.2f KB / %.2f KB", float(imageDownloaded/1024.0), float(imageSize/1024.0)); }
                else { sprintf(bufferForText,"%d B / %d B", imageDownloaded, imageSize); }
            }

        } else { sprintf(bufferForText,"awaiting data"); }

        const char * whatAreYouDoing="Please wait...";
        if ( OTASTEP_NO_WIFI==OTAStep ) {
            whatAreYouDoing="No WiFi";
        } else if ( OTASTEP_ALREADYUPDATED==OTAStep ) {
            whatAreYouDoing="Updated!";
        } else if ( OTASTEP_CONNECTING==OTAStep ) {
            whatAreYouDoing="Connecting...";
        } else if ( OTASTEP_ONLINE==OTAStep ) {            
            whatAreYouDoing="Online!";
        } else if ( OTASTEP_CHECKING==OTAStep ) {            
            whatAreYouDoing="Checking...";
        } else if ( OTASTEP_HTTPS_ERROR==OTAStep ) {
            whatAreYouDoing="HTTPS Error!";
        } else if ( OTASTEP_IMAGE_DOWNLOAD==OTAStep ) {
            whatAreYouDoing="Download";
        } else if ( OTASTEP_IMAGE_ERROR==OTAStep ) {
            whatAreYouDoing="Firmware Error!";
        } else if ( OTASTEP_IMAGE_DOWNLOAD_DONE==OTAStep ) {
            whatAreYouDoing="Done!";
        }
        if ( updateBtn->GetEnabled() ) {
            btnBack->DrawTo(canvas);
        } else {
            progressBar->DrawTo(canvas);
        }
        
        if ( ( OTASTEP_IDLE!=OTAStep ) ) { //&& ( OTASTEP_IMAGE_DOWNLOAD!=OTAStep ) ) {
            canvas->setTextSize(2);
            canvas->setTextDatum(TC_DATUM);
            canvas->setTextColor(ThCol(text));
            canvas->drawString(whatAreYouDoing,canvas->width()/2,20); // canvas->height()-20);
        }

        if ( updateBtn->GetEnabled() ) {
            updateBtn->DrawTo(canvas);
            // draw news/changes circle over button
            uint32_t myVersion = LUNOKIOT_BUILD_NUMBER;
            uint32_t remoteVersion = 0;
            if ( nullptr != latestBuildFoundString ) {
                remoteVersion = atoi(latestBuildFoundString);
            }
            
            if ( remoteVersion > myVersion ) {
                canvas->fillCircle(updateBtn->x,updateBtn->y,10,TFT_BLACK);
                canvas->fillCircle(updateBtn->x,updateBtn->y,8,TFT_WHITE);
                canvas->drawCircle(updateBtn->x,updateBtn->y,6,TFT_BLACK);
                canvas->fillCircle(updateBtn->x,updateBtn->y,5,TFT_GREEN);
            }
        } else {
            OTADownloadSpeed->DrawTo(canvas,30,canvas->height()-70);
            canvas->setTextSize(2);
            canvas->setTextDatum(BC_DATUM);
            canvas->setTextColor(ThCol(text));
            canvas->drawString(bufferForText,canvas->width()/2, canvas->height()-5);
            //canvas->drawString(bufferForText,canvas->width()/2, canvas->height()-5);

            canvas->setTextSize(4);
            canvas->setTextDatum(CC_DATUM);
            canvas->drawString(percentAsString,canvas->width()/2, (canvas->height()/2)-10);

        }
        nextRefresh=millis()+(1000/6);
        return true;
    }
    return false;
}
