#include <Arduino.h>
#include "LogView.hpp" // log capabilities
#include "Shutdown.hpp" // call shutdown

#include <WiFi.h>
#include "../UI/AppTemplate.hpp"
#include "OTAUpdate.hpp"
#include "esp_wifi.h"
#include <esp_https_ota.h>

#include "../static/img_update_48.xbm"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

char * latestBuildFoundString = nullptr;
extern const uint8_t githubPEM_start[] asm("_binary_asset_raw_githubusercontent_com_pem_start");
extern const uint8_t githubPEM_end[] asm("_binary_asset_raw_githubusercontent_com_pem_end");
extern bool wifiOverride;

int OTAbytesPerSecond=0;

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        lAppLog("HTTP_EVENT_ERROR\n");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        lAppLog("HTTP_EVENT_ON_CONNECTED\n");
        break;
    case HTTP_EVENT_HEADER_SENT:
        lAppLog("HTTP_EVENT_HEADER_SENT\n");
        break;
    case HTTP_EVENT_ON_HEADER:
        lAppLog("HTTP_EVENT_ON_HEADER, key=%s, value=%s\n", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        //lAppLog("HTTP_EVENT_ON_DATA, len=%d\n", evt->data_len);
        OTAbytesPerSecond=evt->data_len;
        break;
    case HTTP_EVENT_ON_FINISH:
        lAppLog("HTTP_EVENT_ON_FINISH\n");
        break;
    case HTTP_EVENT_DISCONNECTED:
        lAppLog("HTTP_EVENT_DISCONNECTED\n");
        break;
    }
    return ESP_OK;
}

OTAUpdateApplication::OTAUpdateApplication() {
    canvas->fillSprite(ThCol(background)); // use theme colors
    progressBar = new GaugeWidget(20, 20, 200);
//uint32_t wgaugeColor=ThCol(button), uint32_t markColor=ThCol(middle), uint32_t needleColor=ThCol(highlight), uint32_t incrementColor=ThCol(max));

    updateBtn = new ButtonImageXBMWidget(120-40,120-40, 80, 80, [&,this](){
        updateBtn->SetEnabled(false);
        if ( nullptr == latestBuildFoundString ) {
            lAppLog("OTA: WiFi working?\n");
            updateBtn->SetEnabled(true);
            return;
        }
        uint32_t myVersion = LUNOKIOT_BUILD_NUMBER;
        uint32_t remoteVersion = atoi(latestBuildFoundString);
        if ( remoteVersion <= myVersion ) { // already have the latest version :)
            updateBtn->SetEnabled(true);
            return;
        }
        lAppLog("OTA: Trying to get Wifi...\n");
        wifiOverride=true;
        WiFi.begin();
        esp_wifi_set_ps(WIFI_PS_NONE); // disable wifi powersave
        long timeout = millis()+(20*1000);
        while ( timeout > millis() ) {
            wl_status_t currStat = WiFi.status();
            if ( WL_CONNECTED == currStat) { break; }
        }
        if ( millis() >= timeout ) {
            lAppLog("OTA: Timeout trying to start wifi!\n");
            updateBtn->SetEnabled(true);
            return;
        }
        lAppLog("OTA: WiFi becomes Online\n");
        delay(1000);
        esp_err_t ota_finish_err = ESP_OK;
        // -DLUNOKIOT_UPDATES_ENABLED
        lAppLog("OTA: System update availiable: '%s' running: '%s'\n",latestBuildFoundString,LUNOKIOT_BUILD_STRING);
        // implement https OTA https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-reference/system/esp_https_ota.html
        char firmwareURL[256];
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

        sprintf(firmwareURL,"https://raw.githubusercontent.com/lunokjod/watch/devel/ota/lunokWatch_%s_%s.bin",latestBuildFoundString,myDeviceName);
        esp_http_client_config_t config = {
            .url = firmwareURL,
            .cert_pem = (const char *)githubPEM_start,
            .user_agent = "Wget/1.21.2",
            .timeout_ms = 5000,
            .event_handler = _http_event_handler,
            .keep_alive_enable = true,
        };
        
        lAppLog("OTA: Getting new firmware from: '%s'...\n",firmwareURL);
        esp_https_ota_config_t ota_config = {
            .http_config = &config,
        };
        esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
        if (err != ESP_OK) {
            lAppLog("OTA: ESP HTTPS OTA Begin failed: %s\n",esp_err_to_name(err));
            goto ota_end;
        }


        esp_app_desc_t app_desc;
        err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
        if (err != ESP_OK) {
            lAppLog("OTA: esp_https_ota_read_img_desc failed\n");
            goto ota_end;
        }

        /*
        err = validate_image_header(&app_desc);
        if (err != ESP_OK) {
            lAppLog("image header verification failed\n");
            goto ota_end;
        }*/
        while (1) {
            err = esp_https_ota_perform(https_ota_handle);
            if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
                break;
            }
            // esp_https_ota_perform returns after every read operation which gives user the ability to
            // monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of image
            // data read so far.
            imageSize=esp_https_ota_get_image_size(https_ota_handle);
            imageDownloaded=esp_https_ota_get_image_len_read(https_ota_handle);

            //lAppLog("OTA: Image bytes read: %d/%d\n",imageDownloaded,imageSize);
        }

        if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
            // the OTA image was not completely received and user can customise the response to this situation.
            lAppLog("OTA: Complete data was not received.\n");
        }

ota_end:
        ota_finish_err = esp_https_ota_finish(https_ota_handle);
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
            https_ota_handle=NULL;
            lAppLog("OTA: ESP_HTTPS_OTA upgrade successful. Rebooting ...\n");
            LaunchApplication(new ShutdownApplication(true,true));
            return;
        } else {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                lAppLog("OTA: Image validation failed, image is corrupted\n");
            }
            lAppLog("OTA: ESP_HTTPS_OTA upgrade failed %d\n", ota_finish_err);
            esp_https_ota_abort(https_ota_handle);
            https_ota_handle=NULL;
        }
        delay(100);
        WiFi.disconnect();
        delay(100);
        WiFi.mode(WIFI_OFF);
        esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
        wifiOverride=false;
        updateBtn->SetEnabled(true);
    },img_update_48_bits, img_update_48_height, img_update_48_width);
    updateBtn->taskStackSize=LUNOKIOT_APP_STACK_SIZE;
    Tick();
}

OTAUpdateApplication::~OTAUpdateApplication() {
    if ( NULL != https_ota_handle ) {
        esp_https_ota_abort(https_ota_handle);
        https_ota_handle=NULL;
        WiFi.disconnect();
        delay(100);
        WiFi.mode(WIFI_OFF);
        esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
        wifiOverride=false;
    }
    delete updateBtn;
    delete progressBar;
}

bool OTAUpdateApplication::Tick() {
    btnBack->SetEnabled( updateBtn->GetEnabled() );
    bool interacted = TemplateApplication::Tick(); // calls to TemplateApplication::Tick()
    if ( interacted ) { return true; } // is back button
    
    updateBtn->Interact(touched,touchX, touchY); 


    if ( millis() > nextRefresh ) {
        canvas->fillSprite(ThCol(background)); // use theme colors
        TemplateApplication::Tick(); // redraw back button
        if ( imageSize > 0 ) {
            if ( 0 == (millis()%500) ) {
                progressBar->selectedAngle= (imageDownloaded*360)/imageSize;
                if ( progressBar->selectedAngle > 359 ) {
                    progressBar->selectedAngle=0;
                }
                progressBar->dirty=true; // force redraw of gauge (used as progressbar here)
                progressBar->DrawTo(canvas);
                int percentDone =(imageDownloaded*100)/imageSize;
                lAppLog("PERCENT: %d\n", percentDone);
            }
        }

        if ( updateBtn->GetEnabled() ) {
            updateBtn->DrawTo(canvas);
        }
        nextRefresh=millis()+(1000/8);
        return true;
    }
    return false;
}
