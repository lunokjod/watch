#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../lunokiot_config.hpp"

#include <ArduinoNvs.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>

#include "../system/SystemEvents.hpp"
#include <wifi_provisioning/manager.h>

#include "Watchface.hpp"

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

#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_BLE
#include <wifi_provisioning/scheme_ble.h>
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_BLE */

#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP
#include <wifi_provisioning/scheme_softap.h>
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP */

#include "Provisioning.hpp"

#include "Shutdown.hpp"


#include "../static/img_provisioning_160.xbm"
#include "../static/img_provisioning_48.xbm"
#include "../static/img_trash_48.xbm"

bool provisioned = false;
bool provisioningStarted = false;

/* Do we want a proof-of-possession (ignored if Security 0 is selected):
    *      - this should be a string with length > 0
    *      - NULL if not used
    */
//const char *pop = "abcd1234";
char *pop=nullptr;
char service_name[32];
char *service_key = nullptr;

TFT_eSprite * currentQRRendered = nullptr;
QRCode currentQR;
uint8_t *currentQRData;

void GenerateQRCode() {
    /*
    char payload[150] = {0};
    snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\"" \
            ",\"transport\":\"%s\"}",
            PROV_QR_VERSION, name, transport);
    */
    if ( nullptr!= currentQRRendered ) {
        TFT_eSprite * btnPtr = currentQRRendered;
        currentQRRendered = nullptr;
        delete btnPtr;
    }
    //@TODO  must be use canvasWidget instead of tft sprites!! 
    currentQRRendered = new TFT_eSprite(ttgo->tft);
    currentQRRendered->setColorDepth(16);
    currentQRRendered->createSprite(QRSize, QRSize);
    currentQRRendered->fillSprite(TFT_BLACK);


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
    Serial.printf("Provisioning: QR: Generated with: '%s' (size: %d)\n",wifiCredentials,strlen(wifiCredentials));
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
    uint16_t iconColor = ttgo->tft->color24to16(0x71648b);
    //(ttgo->tft->color565(64,64,64);
    // border
    currentQRRendered->fillCircle((currentQRRendered->width()/2),(currentQRRendered->height()/2),radius, iconColor); // BRG
    // background
    currentQRRendered->fillCircle((currentQRRendered->width()/2),(currentQRRendered->height()/2),radius-5, ttgo->tft->color565(255,255,255)); // BRG
    // icon
    currentQRRendered->drawXBitmap((currentQRRendered->width()/2)-(img_provisioning_48_width/2),(currentQRRendered->height()/2)-(img_provisioning_48_height/2), img_provisioning_48_bits, img_provisioning_48_width, img_provisioning_48_height,iconColor); // BRG

   Serial.printf("Provisioning: QR: Version: %d Side: %d Pixelscale: %d\n", QRVERSION, SIDELEN, PIXELSIZE);

}

bool ProvisioningApplication::Tick() {
    if ( nullptr == this->GetCanvas() ) { return false; }

    if ( false == provisioningStarted ) {
        BackToWatchface->Interact(touched,touchX,touchY);
        StartProvisioningButton->Interact(touched,touchX,touchY);
        ClearProvisioningButton->Interact(touched,touchX,touchY);
    }

    static unsigned long nextRedraw=0;
    if ( nextRedraw < millis() ) {
        char stepsString[128] = { 0 };
        // new picture!!!
        // BGR, but color24to16 seems swaps fine :)
        this->GetCanvas()->fillSprite(ttgo->tft->color24to16(0x565b71));
        //this->GetCanvas()->fillSprite(ttgo->tft->color24to16(0x0));

        if ( false == provisioningStarted ) {
            this->GetCanvas()->drawXBitmap((TFT_WIDTH/2)-(img_provisioning_160_width/2),(TFT_HEIGHT/2)-(img_provisioning_160_height/2), img_provisioning_160_bits, img_provisioning_160_width, img_provisioning_160_height, ttgo->tft->color24to16(0x242424));
        } else {
            if ( nullptr != currentQRRendered ) {
                this->GetCanvas()->setPivot(TFT_WIDTH/2,(TFT_HEIGHT/2));
                currentQRRendered->pushRotated(this->GetCanvas(),0);

            }
        }
        /*
        if (nullptr != pop) {
            this->GetCanvas()->setTextSize(1);
            this->GetCanvas()->setFreeFont(&FreeMonoBold9pt7b);
            this->GetCanvas()->setTextDatum(BC_DATUM);
            this->GetCanvas()->setTextWrap(false,false);
            int32_t posX = TFT_WIDTH/2;
            int32_t posY = TFT_HEIGHT-12;
            sprintf(stepsString,"PoP: %s",pop);
            this->GetCanvas()->setTextColor(TFT_WHITE);
            this->GetCanvas()->drawString(stepsString, posX, posY);
        }*/

        nextRedraw = millis()+100;
    }

    if ( false == provisioningStarted ) {

        
        BackToWatchface->DrawTo(this->GetCanvas());


        // clear NVS provisioning
        ClearProvisioningButton->DrawTo(this->GetCanvas());
        uint16_t iconColor = ttgo->tft->color24to16(0xffffff);
        int16_t cx = (ClearProvisioningButton->x + (ClearProvisioningButton->GetCanvas()->height()/2)-(img_trash_48_height/2));
        int16_t cy = (ClearProvisioningButton->y + (ClearProvisioningButton->GetCanvas()->width()/2)-(img_trash_48_width/2));
        this->GetCanvas()->drawXBitmap(cx,cy,img_trash_48_bits, img_trash_48_width, img_trash_48_height,iconColor);

        // begin provisioning
        StartProvisioningButton->DrawTo(this->GetCanvas());
        cx = (StartProvisioningButton->x + (StartProvisioningButton->GetCanvas()->width()/2)-(img_provisioning_48_width/2));
        cy = (StartProvisioningButton->y + (StartProvisioningButton->GetCanvas()->height()/2)-(img_provisioning_48_height/2));
        //x = (StartProvisioningButton->x -(img_trash_48_width/2));
        //y = (StartProvisioningButton->y -(img_trash_48_height/2));
        this->GetCanvas()->drawXBitmap(cx,cy,img_provisioning_48_bits, img_provisioning_48_width, img_provisioning_48_height,iconColor);
     }
    return true;
}

ProvisioningApplication::~ProvisioningApplication(){

    if ( nullptr != BackToWatchface ) {
        ButtonWidget * btnPtr = BackToWatchface;
        BackToWatchface = nullptr;
        btnPtr->canvas->deleteSprite();
        delete btnPtr;
    }
    if ( nullptr != StartProvisioningButton ) {
        ButtonWidget * btnPtr = StartProvisioningButton;
        StartProvisioningButton = nullptr;
        btnPtr->canvas->deleteSprite();
        delete btnPtr;
    }
    if ( nullptr != ClearProvisioningButton ) {
        ButtonWidget * btnPtr = ClearProvisioningButton;
        ClearProvisioningButton = nullptr;
        btnPtr->canvas->deleteSprite();
        delete btnPtr;
    }
    if ( nullptr!= currentQRRendered ) {
        TFT_eSprite * btnPtr = currentQRRendered;
        currentQRRendered = nullptr;
        btnPtr->deleteSprite();
        delete btnPtr;
    }
    if ( nullptr != pop ) {
        char * shit = pop;
        pop = nullptr;
        free(shit);
    }
    if ( nullptr != service_key ) {
        char * shit = service_key;
        service_key = nullptr;
        free(shit);
    }

}


/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;
static EventGroupHandle_t wifi_event_group;

/* Event handler for catching system events */
static void event_handler(void* arg, esp_event_base_t event_base,
                          int event_id, void* event_data) {
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                //Serial.println("Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                Serial.printf("Provisioning: Event: Received Wi-Fi credentials SSID: '%s' Password : '%s'\n", wifi_sta_cfg->ssid, wifi_sta_cfg->password);
                break;
            }
            case WIFI_PROV_CRED_FAIL: {
                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                Serial.printf("Provisioning failed!\n\tReason : %s"
                         "\n\tPlease reset to factory and retry provisioning",
                         (*reason == WIFI_PROV_STA_AUTH_ERROR) ?
                         "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
                         provisioned=false;
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                Serial.println("Provisioning successful");
                provisioned=true;
                break;
            case WIFI_PROV_END:
                /* De-initialize manager once provisioning is finished */
                wifi_prov_mgr_deinit();
                break;
            default:
                break;
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        Serial.printf("Connected with IP Address:" IPSTR "\n", IP2STR(&event->ip_info.ip));
        /* Signal main application to continue execution */
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        Serial.println("Disconnected. Connecting to the AP again...");
        esp_wifi_connect();
    }
}

static void wifi_init_sta(void)
{
    /* Start Wi-Fi in station mode */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "lunokIoT_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

/* Handler for the optional provisioning endpoint registered by the application.
 * The data format can be chosen by applications. Here, we are using plain ascii text.
 * Applications can choose to use other formats like protobuf, JSON, XML, etc.
 */
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                          uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf) {
        Serial.printf("Received data: %.*s\n", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        Serial.println("System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

void GeneratePOPAndWifiKey() {

    const size_t maxLenPOP=8;
    const char letters[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    char * popBack = nullptr;
    // generate new one
    if ( nullptr == pop ) { popBack = (char *)ps_malloc(maxLenPOP+1); }
    
    for(size_t currentCharPOP=0; currentCharPOP <maxLenPOP;currentCharPOP++) {
        popBack[currentCharPOP] = letters[random(0, strlen(letters)-1)];
    }
    popBack[maxLenPOP] = '\0'; // EOL
    pop = popBack;


    char * service_keyBack = nullptr;
    // generate new one
    if ( nullptr == service_key ) { service_keyBack = (char *)ps_malloc(maxLenPOP+1); }
    
    for(size_t currentCharPOP=0; currentCharPOP <maxLenPOP;currentCharPOP++) {
        service_keyBack[currentCharPOP] = letters[random(0, strlen(letters)-1)];
    }
    service_keyBack[maxLenPOP] = '\0'; // EOL
    service_key = service_keyBack;

}
void BeginProvisioning(void) {

    if ( true == provisioningStarted ) {
        Serial.println("Provisioning: Service already running, discarded");
        return;
    }
    provisioningStarted = true;
    Serial.println("Provisioning: Starting provisioning WiFi AP");
    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    GeneratePOPAndWifiKey();
    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());


    /* Initialize the event loop */
    //NO NEEDED!! ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();
    Serial.println("Provisioning: Registering event hooks...");

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();
#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP
    esp_netif_create_default_wifi_ap();
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
        /* What is the Provisioning Scheme that we want ?
         * wifi_prov_scheme_softap or wifi_prov_scheme_ble */
#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_BLE
        .scheme = wifi_prov_scheme_ble,
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_BLE */
#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP
        .scheme = wifi_prov_scheme_softap,
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP */

        /* Any default scheme specific event handler that you would
         * like to choose. Since our example application requires
         * neither BT nor BLE, we can choose to release the associated
         * memory once provisioning is complete, or not needed
         * (in case when device is already provisioned). Choosing
         * appropriate scheme specific event handler allows the manager
         * to take care of this automatically. This can be set to
         * WIFI_PROV_EVENT_HANDLER_NONE when using wifi_prov_scheme_softap*/
#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_BLE
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_BLE */
#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP */
    };

    /* Initialize provisioning manager with the
     * configuration parameters set above */
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    /* Let's find out if the device is provisioned */
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    //bool provisioned = NVS.getInt("provisioned");
    NVS.setInt("provisioned",provisioned, false);
    /* If device is not yet provisioned start provisioning service */
    if (!provisioned) {
        //Serial.println("Starting provisioning");

        /* What is the Device Service Name that we want
         * This translates to :
         *     - Wi-Fi SSID when scheme is wifi_prov_scheme_softap
         *     - device name when scheme is wifi_prov_scheme_ble
         */
        get_device_service_name(service_name, sizeof(service_name));

        /* What is the security level that we want (0 or 1):
         *      - WIFI_PROV_SECURITY_0 is simply plain text communication.
         *      - WIFI_PROV_SECURITY_1 is secure communication which consists of secure handshake
         *          using X25519 key exchange and proof of possession (pop) and AES-CTR
         *          for encryption/decryption of messages.
         */
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

        /* What is the service key (could be NULL)
         * This translates to :
         *     - Wi-Fi password when scheme is wifi_prov_scheme_softap
         *     - simply ignored when scheme is wifi_prov_scheme_ble
         */

#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_BLE
        /* This step is only useful when scheme is wifi_prov_scheme_ble. This will
         * set a custom 128 bit UUID which will be included in the BLE advertisement
         * and will correspond to the primary GATT service that provides provisioning
         * endpoints as GATT characteristics. Each GATT characteristic will be
         * formed using the primary service UUID as base, with different auto assigned
         * 12th and 13th bytes (assume counting starts from 0th byte). The client side
         * applications must identify the endpoints by reading the User Characteristic
         * Description descriptor (0x2901) for each characteristic, which contains the
         * endpoint name of the characteristic */
        uint8_t custom_service_uuid[] = {
            /* LSB <---------------------------------------
             * ---------------------------------------> MSB */
            0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
            0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
        };
        wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_BLE */

        /* An optional endpoint that applications can create if they expect to
         * get some additional custom data during provisioning workflow.
         * The endpoint name can be anything of your choice.
         * This call must be made before starting the provisioning.
         */
        //Serial.println("@WARNING @TODO @TEST custom provisioning endpoint implemented");
        //wifi_prov_mgr_endpoint_create("custom-data");


        // generate QR image...
        GenerateQRCode();

        /* Start provisioning service */
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key));
        
        /* The handler for the optional endpoint created above.
         * This call must be made after starting the provisioning, and only if the endpoint
         * has already been created above.
         */
        //wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);


        /* Uncomment the following to wait for the provisioning to finish and then release
         * the resources of the manager. Since in this case de-initialization is triggered
         * by the default event loop handler, we don't need to call the following */
        Serial.println("Provisioning: Waiting for connection...");
        wifi_prov_mgr_wait();
        Serial.println("Provisioning: Stopping privisioning service...");
        wifi_prov_mgr_deinit();
        provisioningStarted=false;
        Serial.println("Provisioning: New WiFi credentials obtained!");
    } else {
        //Serial.println("Provisioning: Already provisioned, starting Wi-Fi STA");

        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        wifi_prov_mgr_deinit();
        /* Start Wi-Fi station */
        //wifi_init_sta();
        Serial.println("Provisioning: Already provisioned");
    }
    Serial.println("Provisioning: Unregistering event hooks...");
    esp_event_handler_unregister(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler);
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler);

    // done, resume normal operations
    if (provisioned) {
        LaunchApplication(new WatchfaceApplication());
    } else {
        Serial.println("Provisioning: procedure finished, but no valid WiFi provisioning received?");
    }
    /* Wait for Wi-Fi connection */
    //xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, false, true, portMAX_DELAY);

    /* Start main application now */
    /*
    while (1) {
        Serial.println("Hello World!");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    */
    provisioningStarted=false;
}


ProvisioningApplication::ProvisioningApplication() {
    LunokIoTApplication();

    BackToWatchface = new ButtonWidget(5,155,70,80,[&,this]() {
        Serial.println("Provisioning: User has rejected the provisioning");
        Serial.println("Provisioning: @TODO launch watchface here!");
        //LaunchApplication(new WatchfaceApplication());
    }, ttgo->tft->color565(54,44,39)); // BRG

    ClearProvisioningButton = new ButtonWidget(5,5,70,70,[&,this]() {
        provisioned=false;
        wifi_prov_mgr_reset_provisioning();
        Serial.println("Provisioning: NVS: Provisioning data destroyed");
        SaveDataBeforeShutdown();
        Serial.println("ESP32: -- Restart --");
        Serial.flush();
        delay(300);
        ESP.restart();
        LaunchApplication(new ShutdownApplication());
    }, ttgo->tft->color565(0,255,0)); // BRG
    //ClearProvisioningButton->enabled=provisioned;
    StartProvisioningButton = new ButtonWidget(90,5,230,140,[&, this]() {
        Serial.printf("Provisioning: %p: Starting provisioning procedure...\n",this);
        BeginProvisioning();
    }, ttgo->tft->color565(255,0,80)); // BRG
    StartProvisioningButton->taskStackSize=LUNOKIOT_TASK_PROVISIONINGSTACK_SIZE;
}
