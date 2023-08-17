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
#include <ArduinoNvs.h>

#include <LilyGoWatch.h>
//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include "../system/Network/BLE.hpp"
#include "Provisioning2.hpp"
#include "../system/Application.hpp"

#include "Shutdown.hpp"

#include "../../static/img_back_32.xbm"
#include "../../static/img_provisioning_48.xbm"
#include "../../static/img_trash_48.xbm"
#include "../../static/img_bluetooth_32.xbm"
#include "../../static/img_wifi_32.xbm"

#include "../lunokIoT.hpp"
#include "WiFiProv.h"
#include "WiFi.h"
#include <wifi_provisioning/manager.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include "../system/SystemEvents.hpp"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_system.h"

#include "LogView.hpp"
#include "../system/Network.hpp"

// https://github.com/ricmoo/QRCode
#include <qrcode.h>
//#define QRVERSION 28  //129x129
//#define QRVERSION 22  //105x105
//#define QRVERSION 16  //81x81
//#define QRVERSION 10  //57x57
//#define QRVERSION 8   //49x49
#define QRVERSION 5     //37x37 = 60 bytes for url
//#define QRVERSION 2   //25x25
//#define QRVERSION 1   //21x21
const int16_t QRSize = 240;
#define SIDELEN (4 * QRVERSION + 17)
#define PIXELSIZE QRSize/SIDELEN
#define PIXELSIZE2 QRSize/SIDELEN // with wide black borders
#define PROV_QR_VERSION         "v1"

//extern bool wifiOverride;
//bool provisioned=false;
extern TTGOClass *ttgo;

Provisioning2Application *lastProvisioning2Instance = nullptr;

void Provisioning2_SysProvEvent(arduino_event_t *sys_event);

void Provisioning2DestroyNVS() {
    WiFi.begin();
    esp_err_t erased = wifi_prov_mgr_reset_provisioning();
    WiFi.mode(WIFI_OFF);
    if ( ESP_OK == erased ) {
        lLog("Provisioning: NVS: Provisioning data destroyed\n");
        //NVS.setInt("provisioned",0,false);
        //NVS.setInt("WiFiCredNo",0,false);
        LoT().GetSettings()->SetInt(SystemSettings::SettingKey::WiFiCredentialsNumber,0);
        //provisioned=false;
        LaunchApplication(new ShutdownApplication(true,true));
    }
}

void Provisioning2Deinit() {
    wifi_prov_mgr_stop_provisioning();
    wifi_prov_mgr_deinit();
    WiFi.removeEvent(Provisioning2_SysProvEvent);
    WiFi.disconnect(true,true);
    delay(100);
    WiFi.mode(WIFI_OFF);
    if ( nullptr != lastProvisioning2Instance ) {
        lastProvisioning2Instance->provisioningStarted = false;
    }
}

void Provisioning2_SysProvEvent(arduino_event_t *sys_event) {
    switch (sys_event->event_id) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            //Serial.print("\nConnected IP address : ");
            //Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            lLog("Provisioning: Disconnected. Connecting to the AP again... \n");
            break;
        case ARDUINO_EVENT_PROV_START:
            lLog("Provisioning: Up and ready! use ESP provisining app\n");
            break;
        case ARDUINO_EVENT_PROV_CRED_RECV: { 
            lLog("Provisioning: Received Wi-Fi credentials:\n");
            lLog("SSID: '%s'\n",(const char *) sys_event->event_info.prov_cred_recv.ssid);
            lLog("Password: '%s'\n",(char const *) sys_event->event_info.prov_cred_recv.password);
            const size_t BuffrSize = 64;
            char ssid[BuffrSize+1];
            char passwd[BuffrSize+1];
            snprintf(ssid,BuffrSize,"%s",sys_event->event_info.prov_cred_recv.ssid);
            snprintf(passwd,BuffrSize,"%s",sys_event->event_info.prov_cred_recv.password);
            lNetLog("WiFi: Provisioning: Added '%s' pwd: '%s' offset: %u\n", ssid, passwd, LoT().GetWiFi()->GetConnections() );
            LoT().GetWiFi()->AddConnection(ssid,passwd);
            //NVS.setString("provSSID",buffr,false);
            //NVS.setString("provPWD",buffr,false);
            break;
        }
        case ARDUINO_EVENT_PROV_CRED_FAIL: {
            lNetLog("WiFi: Provisioning: Failed, reason:\n");
            if(sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR) {
                lNetLog("WiFi: AP password incorrect\n");
            } else {
                lNetLog("WiFi: AP not found\n");
            }
            Provisioning2DestroyNVS();
            //NVS.setInt("provisioned",0,false);
            //provisioned = false;
            Provisioning2Deinit();
            break;
        }
        case ARDUINO_EVENT_PROV_CRED_SUCCESS:
            lNetLog("WiFi: Provisioning Successful\n");
            if ( nullptr != lastProvisioning2Instance ) {
                lastProvisioning2Instance->provisioningStarted = false;
                //NVS.setInt("provisioned",1, false);
                //provisioned = true;
                //LaunchApplication(new ShutdownApplication(true,true));
            }
            break;
        case ARDUINO_EVENT_PROV_END:
            lNetLog("WiFi: Provisioning Ends\n");
            LaunchWatchface();
            break;
        default:
            break;
    }
}

void Provisioning2Application::GenerateQRCode() {
    /*
    char payload[150] = {0};
    snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\"" \
            ",\"transport\":\"%s\"}",
            PROV_QR_VERSION, name, transport);
    */
    if ( nullptr!= currentQRRendered ) {
        currentQRRendered->deleteSprite();
        delete currentQRRendered;
        currentQRRendered = nullptr;
    }
    //@TODO  must be use canvasWidget instead of tft sprites!! 
    currentQRRendered = new TFT_eSprite(tft);
    currentQRRendered->setColorDepth(16);
    currentQRRendered->createSprite(QRSize, QRSize);
    currentQRRendered->fillSprite(TFT_BLACK);

    if ( nullptr != currentQRData ) {
        free(currentQRData);
        currentQRData = nullptr;
    }
    currentQRData = (uint8_t*)ps_calloc(qrcode_getBufferSize(QRVERSION), sizeof(uint8_t));
    char wifiCredentials[250] = { 0 };
    // WIFI SHARE
    //Serial.printf("SERVICE NAME: '%s' PASSWORD: '%s'\n", service_name, service_key);
    // FOR AP sprintf(wifiCredentials,"WIFI:T:WPA;S:%s;P:%s;;", service_name, service_key );
    //sprintf(wifiCredentials,"WIFI:T:WPA;S:%s;P:%s;;", service_name, service_key );

    // PROVISIONING SHARE
    snprintf(wifiCredentials, sizeof(wifiCredentials), "{\"ver\":\"%s\",\"name\":\"%s\"" \
                ",\"pop\":\"%s\",\"transport\":\"%s\",\"password\":\"%s\"}",
                PROV_QR_VERSION, service_name, pop, "softap", service_key);
    lLog("Provisioning: QR: Generated with: '%s' (size: %d)\n",wifiCredentials,strlen(wifiCredentials));
    qrcode_initText(&currentQR, currentQRData, QRVERSION, ECC_LOW, wifiCredentials);
// ECC_QUARTILE
    for (uint8_t y=0; y < currentQR.size; y++) {
        // Each horizontal module
        for (uint8_t x=0; x < currentQR.size; x++) {
            bool positive = qrcode_getModule(&currentQR, x, y);
            uint32_t color = TFT_BLACK;
            if ( positive ) { color = TFT_WHITE; }
            //currentScreen->drawPixel(x,y,color);
            //currentScreen->fillRect((x*PIXELSIZE2)+20,(y*PIXELSIZE2)+20,PIXELSIZE2,PIXELSIZE2,color);
            currentQRRendered->fillRect((x*PIXELSIZE),(y*PIXELSIZE),PIXELSIZE+1,PIXELSIZE+1,color);
            //rx+=2;
        }
    }

    const int32_t radius = 28; 
    uint16_t iconColor = canvas->color24to16(0x0a0f1e);
    // border
    currentQRRendered->fillCircle((currentQRRendered->width()/2),(currentQRRendered->height()/2),radius, TFT_WHITE);
    // background
    currentQRRendered->fillCircle((currentQRRendered->width()/2),(currentQRRendered->height()/2),radius-5, TFT_BLACK);
    // icon
    //currentQRRendered->drawXBitmap((currentQRRendered->width()/2)-(img_provisioning_48_width/2),(currentQRRendered->height()/2)-(img_provisioning_48_height/2), img_provisioning_48_bits, img_provisioning_48_width, img_provisioning_48_height,iconColor); // BRG

   lLog("Provisioning: QR: Version: %d Side: %d Pixelscale: %d\n", QRVERSION, SIDELEN, PIXELSIZE);

}

Provisioning2Application::~Provisioning2Application() {
    lastProvisioning2Instance = nullptr;
    if ( provisioningStarted ) {
        lLog("Provisioning: Stopping the provisioning...\n");
        Provisioning2Deinit();
    }
    if (nullptr != pop ) { free(pop); pop=nullptr; }
    if (nullptr != service_key ) { free(service_key); service_key=nullptr; }
    if ( nullptr!= currentQRRendered ) {
        currentQRRendered->deleteSprite();
        delete currentQRRendered;
        currentQRRendered = nullptr;
    }
    if ( nullptr != currentQRData ) {
        free(currentQRData);
        currentQRData = nullptr;
    }
    LoT().GetWiFi()->Disable();
}


void Provisioning2Application::GenerateCredentials() {
    const size_t maxLenPOP=8;
    const char letters[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // generate new one
    if (nullptr != pop ) { free(pop); pop=nullptr; }
    pop = (char *)ps_malloc(maxLenPOP+1);
    for(size_t currentCharPOP=0; currentCharPOP <maxLenPOP;currentCharPOP++) {
        pop[currentCharPOP] = letters[random(0, strlen(letters)-1)];
    }
    pop[maxLenPOP] = '\0'; // EOL
    //pop = popBack;


    // generate new one
    if (nullptr != service_key ) { free(service_key); service_key=nullptr; }
    service_key = (char *)ps_malloc(maxLenPOP+1);
    
    for(size_t currentCharPOP=0; currentCharPOP <maxLenPOP;currentCharPOP++) {
        service_key[currentCharPOP] = letters[random(0, strlen(letters)-1)];
    }
    service_key[maxLenPOP] = '\0'; // EOL
}

Provisioning2Application::Provisioning2Application() {
    LoT().GetWiFi()->Disable();
    lastProvisioning2Instance = this;
    WiFi.begin();
    // get my name
    uint8_t eth_mac[6];
    const char *ssid_prefix = "lunokIoT_";
    esp_wifi_get_mac(WIFI_IF_AP, eth_mac);
    WiFi.mode(WIFI_OFF);

    snprintf(service_name, 32, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
    clearProvBtn=new ButtonImageXBMWidget(5,5,70,70,[&,this](void *unused){
        Provisioning2DestroyNVS();
    },img_trash_48_bits,img_trash_48_height,img_trash_48_width,TFT_WHITE,TFT_RED);

    wifiOrBLE = new SwitchWidget(130,10,[&,this](void *unused){
        useBluetooth = wifiOrBLE->switchEnabled;
    });
    wifiOrBLE->switchEnabled = useBluetooth;
    wifiOrBLE->enabled = false;
    
    startProvBtn=new ButtonImageXBMWidget(90,80,120,130,[&,this](void *unused){
        // https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiProv/examples/WiFiProv

        if ( provisioningStarted ) {
            lAppLog("Provisioning: %p: Rejected (already runing)\n",this);
            return;
        }
        if ( LoT().GetBLE()->IsEnabled() ) {
            lNetLog("Network: BLE must be disabled to maximize WiFi effort\n");
            LoT().GetBLE()->Disable();
        }
        lAppLog("Provisioning: %p: Starting provisioning procedure...\n",this);
        GenerateCredentials();
        GenerateQRCode();
        provisioningStarted = true;
        lAppLog("Provisioning: SSID: '%s' PASSWORD: '%s' POP: '%s'\n", service_name, service_key, pop);
        if ( useBluetooth ) {
            lAppLog("Provisioning: Using Bluetooth...\n");
            // https://github.com/espressif/esp-idf/blob/master/examples/provisioning/wifi_prov_mgr/main/app_main.c#L240
            WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1,pop,service_name,service_key);
        } else {
            // SoftAP provisioning
            //WiFi.begin();
            lAppLog("Provisioning: Using WiFi...\n");
            WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1,pop,service_name,service_key);
        }
        WiFi.onEvent(Provisioning2_SysProvEvent);

    },img_provisioning_48_bits,img_provisioning_48_height,img_provisioning_48_width,TFT_WHITE,canvas->color24to16(0x2347bc));
    startProvBtn->taskStackSize=LUNOKIOT_PROVISIONING_STACK_SIZE;
    Tick();
}
bool Provisioning2Application::Tick() {
    UINextTimeout = millis()+UITimeout; // don't allow sleep by timeout on this app

    if ( provisioningStarted ) {
        if ( nullptr != currentQRRendered ) {
            if ( false == qrVisible ) {
                currentQRRendered->pushRotated(canvas,0);
                qrVisible = true;
                return true;
            }
        }
        if ( touched ) {
            TTGOClass *ttgo = TTGOClass::getWatch();
            uint8_t bright = (255/tft->width())*touchX;
            //lAppLog("BRIGHT: %u\n",bright);
            ttgo->setBrightness(bright);
        }
        return false;
    }
    if ( qrVisible ) {
        //ttgo->setBrightness(255);
        qrVisible = false;
    }
    //backBtn->enabled = provisioned;
    //clearProvBtn->enabled = provisioned;
    btnBack->Interact(touched,touchX,touchY);
    clearProvBtn->Interact(touched,touchX,touchY);
    startProvBtn->Interact(touched,touchX,touchY);
    wifiOrBLE->Interact(touched,touchX,touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        btnBack->DrawTo(canvas);
        clearProvBtn->DrawTo(canvas);
        startProvBtn->DrawTo(canvas);
        wifiOrBLE->DrawTo(canvas);
        uint32_t wifiColor = TFT_WHITE;
        uint32_t bleColor = TFT_DARKGREY;
        if ( useBluetooth ) {
            wifiColor = TFT_DARKGREY; 
            bleColor = TFT_WHITE; 
        }
        canvas->drawXBitmap(200,26,img_bluetooth_32_bits, img_bluetooth_32_width, img_bluetooth_32_height, bleColor);
        canvas->drawXBitmap(90,30,img_wifi_32_bits, img_wifi_32_width, img_wifi_32_height, wifiColor);

        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}