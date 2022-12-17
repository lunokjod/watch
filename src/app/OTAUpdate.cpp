#include <Arduino.h>
#include "LogView.hpp" // log capabilities
#include "Shutdown.hpp" // call shutdown

#include <WiFi.h>
#include "../UI/AppTemplate.hpp"
#include "OTAUpdate.hpp"
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

char * latestBuildFoundString = nullptr;
extern const uint8_t githubPEM_start[] asm("_binary_asset_raw_githubusercontent_com_pem_start");
extern const uint8_t githubPEM_end[] asm("_binary_asset_raw_githubusercontent_com_pem_end");
extern bool wifiOverride;

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
            OTADownloadTimeLapse=millis()+300;
        } else {
            OTADownloadSample+=evt->data_len;
        }
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
    OTAStep=OTASTEP_IDLE;
    canvas->fillSprite(ThCol(background)); // use theme colors

    OTADownloadSpeed = new GraphWidget(40,180,0,18*1024,TFT_BLUE,Drawable::MASK_COLOR,TFT_YELLOW);

    //progressBar = new GaugeWidget(30, 30, 180);
    progressBar = new GaugeWidget(55, 55, 130);
//uint32_t wgaugeColor=ThCol(button), uint32_t markColor=ThCol(middle), uint32_t needleColor=ThCol(highlight), uint32_t incrementColor=ThCol(max));

    updateBtn = new ButtonImageXBMWidget(120-50,120-50, 100, 100, [&,this](){
        updateBtn->SetEnabled(false);
        if ( nullptr == latestBuildFoundString ) {
            lAppLog("OTA: WiFi working?\n");
            updateBtn->SetEnabled(true);
            OTAStep=OTASTEP_NO_WIFI;
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
        wifiOverride=true;
        WiFi.begin();
        esp_wifi_set_ps(WIFI_PS_NONE); // disable wifi powersave
        long timeout = millis()+(20*1000);
        while ( timeout > millis() ) {
            wl_status_t currStat = WiFi.status();
            if ( WL_CONNECTED == currStat) { break; }
        }
        if ( millis() >= timeout ) {
            lAppLog("OTA: Timeout trying to start WiFi!\n");
            updateBtn->SetEnabled(true);
            OTAStep=OTASTEP_NO_WIFI;
            return;
        }
        lAppLog("OTA: WiFi becomes Online\n");
        OTAStep=OTASTEP_ONLINE;
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
        OTAStep=OTASTEP_CHECKING;
        lAppLog("OTA: Getting new firmware from: '%s'...\n",firmwareURL);
        esp_https_ota_config_t ota_config = {
            .http_config = &config,
        };
        esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
        if (err != ESP_OK) {
            lAppLog("OTA: ESP HTTPS OTA Begin failed: %s\n",esp_err_to_name(err));
            OTAStep=OTASTEP_HTTPS_ERROR;
            goto ota_end;
        }


        esp_app_desc_t app_desc;
        err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
        if (err != ESP_OK) {
            lAppLog("OTA: esp_https_ota_read_img_desc failed\n");
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
            OTAStep=OTASTEP_IMAGE_DOWNLOAD_ERROR;
        }

ota_end:
        ota_finish_err = esp_https_ota_finish(https_ota_handle);
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
            https_ota_handle=NULL;
            OTAStep=OTASTEP_IMAGE_DOWNLOAD_DONE;
            lAppLog("OTA: ESP_HTTPS_OTA upgrade successful. Rebooting ...\n");
            LaunchApplication(new ShutdownApplication(true,true));
            return;
        } else {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                lAppLog("OTA: Image validation failed, image is corrupted\n");
                OTAStep=OTASTEP_IMAGE_DOWNLOAD_ERROR;
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
        OTAStep=OTASTEP_IDLE;
    },img_update_48_bits, img_update_48_height, img_update_48_width); //,TFT_WHITE,TFT_BLACK,false);
    updateBtn->taskStackSize=LUNOKIOT_APP_STACK_SIZE;

    canvas->setTextFont(0);
    //canvas->setTextColor(ThCol(dark));
    canvas->setTextSize(4);
    canvas->setTextDatum(CC_DATUM);
    bufferForText=(char*)ps_malloc(256);
    Tick();
}

OTAUpdateApplication::~OTAUpdateApplication() {
    if ( NULL != https_ota_handle ) {
        esp_https_ota_abort(https_ota_handle);
        https_ota_handle=NULL;
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
    wifiOverride=false;
}


bool OTAUpdateApplication::Tick() {
    btnBack->SetEnabled( updateBtn->GetEnabled() );
    
    bool backTap = btnBack->Interact(touched,touchX, touchY); 
    
    updateBtn->Interact(touched,touchX, touchY); 


    if ( millis() > nextRefresh ) {
        canvas->fillSprite(ThCol(background)); // use theme colors
        if ( updateBtn->GetEnabled() ) {
            if ( backTap ) {
                btnBack->DirectDraw();
            } else {
                btnBack->DrawTo(canvas);
            }
        }

        //imageSize=360;
        //imageDownloaded=180;
        if ( imageSize > 0 ) {
            
            int16_t newAngle = (imageDownloaded*360)/imageSize;
            if ( progressBar->selectedAngle != newAngle) {
                progressBar->dirty=true; // force redraw of gauge (used as progressbar here)
                progressBar->selectedAngle= newAngle;
                if ( progressBar->selectedAngle > 359 ) {
                    progressBar->selectedAngle=0;
                }
                int percentDone=percentDone =(imageDownloaded*100)/imageSize;
                sprintf(percentAsString,"%d%%", percentDone);
                //lAppLog("PERCENT: '%s'\n", percentAsString);
                sprintf(bufferForText,"%d / %d", imageDownloaded, imageSize);
                
            }

        } else { sprintf(bufferForText,"awaiting data"); }
        if ( OTASTEP_IDLE!=OTAStep ) {
            progressBar->DrawTo(canvas);
        }

        const char * whatAreYouDoing="Please wait...";
        if ( OTASTEP_NO_WIFI==OTAStep ) {
            whatAreYouDoing="No updates seen";
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
            uint32_t remoteVersion = atoi(latestBuildFoundString);
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
            canvas->drawString(percentAsString,canvas->width()/2, canvas->height()/2);

        }
        nextRefresh=millis()+(1000/6);
        return true;
    }
    return false;
}
