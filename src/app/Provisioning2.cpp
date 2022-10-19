#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Provisioning2.hpp"
#include "../system/Application.hpp"

#include "Watchface.hpp"
#include "Shutdown.hpp"

#include "../static/img_back_32.xbm"
#include "../static/img_provisioning_48.xbm"
#include "../static/img_trash_48.xbm"

#include "WiFiProv.h"
#include "WiFi.h"
#include <wifi_provisioning/manager.h>

void SysProvEvent(arduino_event_t *sys_event)
{
    switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("\nConnected IP address : ");
        Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("\nDisconnected. Connecting to the AP again... ");
        break;
    case ARDUINO_EVENT_PROV_START:
        Serial.println("\nProvisioning started\nGive Credentials of your access point using \" Android app \"");
        break;
    case ARDUINO_EVENT_PROV_CRED_RECV: { 
        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_FAIL: { 
        Serial.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
        if(sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR) 
            Serial.println("\nWi-Fi AP password incorrect");
        else
            Serial.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");        
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
        Serial.println("\nProvisioning Successful");
        break;
    case ARDUINO_EVENT_PROV_END:
        Serial.println("\nProvisioning Ends");
        break;
    default:
        break;
    }
}

Provisioning2Application::~Provisioning2Application() {
    
}
Provisioning2Application::Provisioning2Application() {
    backBtn=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,canvas->color24to16(0x353e45),false);

    clearProvBtn=new ButtonImageXBMWidget(5,5,70,70,[&,this](){
        wifi_prov_mgr_reset_provisioning();
        Serial.println("Provisioning: NVS: Provisioning data destroyed");
        LaunchApplication(new ShutdownApplication(true));
    },img_trash_48_bits,img_trash_48_height,img_trash_48_width,TFT_WHITE,TFT_RED);

    startProvBtn=new ButtonImageXBMWidget(90,5,230,140,[&,this](){
        // https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiProv/examples/WiFiProv
        Serial.printf("Provisioning: %p: @TODO Starting provisioning procedure...\n",this);

        WiFi.onEvent(SysProvEvent);

        #if CONFIG_IDF_TARGET_ESP32 && CONFIG_BLUEDROID_ENABLED
            WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, "abcd1234", "Prov_123");
        #else
            WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, "abcd1234", "Prov_123");
        #endif


    },img_provisioning_48_bits,img_provisioning_48_height,img_provisioning_48_width,TFT_WHITE,ttgo->tft->color24to16(0x2347bc));
    startProvBtn->taskStackSize=LUNOKIOT_TASK_PROVISIONINGSTACK_SIZE;
}
bool Provisioning2Application::Tick() {
    backBtn->Interact(touched,touchX,touchY);
    clearProvBtn->Interact(touched,touchX,touchY);
    startProvBtn->Interact(touched,touchX,touchY);

    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));
        backBtn->DrawTo(canvas);
        clearProvBtn->DrawTo(canvas);
        startProvBtn->DrawTo(canvas);
        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}