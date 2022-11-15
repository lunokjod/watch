#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "PeerApplication.hpp"
#include "../static/img_chat_32.xbm"
#include "../static/img_mainmenu_chat.xbm"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"

uint8_t broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
esp_now_peer_info_t peerInfo;

uint8_t espNowMAC[6];                  // 6 octets are the WIFI address
unsigned long showIconUntil=0;

typedef struct  {
    const char * command;
} lESPNowMessage;

lESPNowMessage PeerAppMessages[] = {
    "lIoT_Hello",
    "lIoT_TS"
};

#define PEER_HELLO PeerAppMessages[0].command
#define PEER_TIMESTAMP PeerAppMessages[1].command

void PeerApplication::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    //lAppLog("ESPNOW: Last Packet Send Status: %s\n",((status == ESP_NOW_SEND_SUCCESS)?"Success":"FAILED"));
}

void PeerApplication::OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    char *dataBuffer =  (char *)ps_malloc(len+1);
    memcpy(dataBuffer,incomingData,len);
    lAppLog("Bytes received: %d from: '%02x:%02x:%02x:%02x:%02x:%02x'\n",len,
    mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    if ( 0 == strncmp(PEER_HELLO,dataBuffer,strlen(PEER_HELLO))) {
        lAppLog("Peer says hello!\n");
        ttgo->shake();
        showIconUntil=millis()+3000;
    } else if ( 0 == strncmp(PEER_TIMESTAMP,dataBuffer,strlen(PEER_TIMESTAMP))) {
        const uint8_t *msgPtr=(const uint8_t *)incomingData+strlen(PEER_TIMESTAMP); // offset of timestamp value
        unsigned long * receivedTimestamp= (unsigned long *)msgPtr;
        msgPtr+=sizeof(unsigned long); // offset of original sender mac
        const uint8_t * fromMACAddress=msgPtr;
        bool thatsMe = true;
        for(int i=0;i<6;i++) {
            //lAppLog("DEBUG: %d from: %02x mine: %02x\n",i,fromMACAddress[i],espNowMAC[i]);
            if ( fromMACAddress[i] != espNowMAC[i] ) { thatsMe=false; break; }
        }
        if ( thatsMe ) {
            lAppLog("ESPNOW: Ping with: '%02x:%02x:%02x:%02x:%02x:%02x' is %d ms\n",
            mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],
            millis()-(*receivedTimestamp));
        } else {
            //lAppLog("ESPNOW received TIMESTAMP, return to sender\n");
            esp_now_send(broadcastAddress, incomingData, len);
        }
    }

    free(dataBuffer);
}

PeerApplication::PeerApplication() {
    canvas->fillSprite(ThCol(background)); // use theme colors
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        lAppLog("Error initializing ESP-NOW\n");
    }
    // Once ESPNow is successfully Init, we will register for Send CB to
    esp_now_register_send_cb(PeerApplication::OnDataSent);
    esp_now_register_recv_cb(PeerApplication::OnDataRecv);

    esp_read_mac(espNowMAC,ESP_MAC_WIFI_STA);    // get from esp-idf :-*

    lAppLog("ESPNOW: My MAC address is: '%s'\n",WiFi.macAddress().c_str());
 
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        lAppLog("ESPNOW: Failed to add peer\n");
    }

    sendHelloBtn=new ButtonImageXBMWidget(20,40,60,200,[](){
        // Send message via ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t*)PEER_HELLO, strlen(PEER_HELLO)+1);
        if (result == ESP_OK) {
            //lAppLog("ESPNOW: Sent with success\n");
        } else {
            //lAppLog("ESPNOW: Error sending\n");
        }
    },img_chat_32_bits,img_chat_32_height,img_chat_32_width);
    Tick(); // OR call this if no splash 
}

PeerApplication::~PeerApplication() {
    // please remove/delete/free all to avoid leaks!
    delete sendHelloBtn;

    esp_now_unregister_recv_cb();
    esp_now_unregister_send_cb();
    esp_now_deinit();
    WiFi.mode(WIFI_OFF);
}

bool PeerApplication::Tick() {

    // put your interacts here:
    //mywidget->Interact(touched,touchX,touchY);

    if ( millis() > nextTimestampSend ) {
        const size_t timestampLenght = strlen(PEER_TIMESTAMP)+sizeof(unsigned long)+ESP_NOW_ETH_ALEN;
        char timestamp[timestampLenght];
        // the command
        memcpy(timestamp,PEER_TIMESTAMP,strlen(PEER_TIMESTAMP));
        // my own mac to recognize the return
        memcpy(timestamp+strlen(PEER_TIMESTAMP)+sizeof(unsigned long),espNowMAC,sizeof(espNowMAC));
        // my timestamp
        unsigned long now = millis();
        //lAppLog("Sending timestamp: %d\n",now);
        memcpy(timestamp+strlen(PEER_TIMESTAMP),&now,sizeof(now));

        esp_now_send(broadcastAddress, (const uint8_t*)timestamp, timestampLenght);
        nextTimestampSend=millis()+(1000*5);
    }
    if ( millis() > nextRefresh ) {
        canvas->fillSprite(ThCol(background)); // use theme colors

        canvas->setTextFont(0);
        canvas->setTextColor(ThCol(text));
        canvas->setTextSize(2);
        canvas->setTextDatum(TC_DATUM);
        canvas->drawString(WiFi.macAddress().c_str(),TFT_WIDTH/2,10);

        sendHelloBtn->Interact(touched,touchX,touchY);
        sendHelloBtn->DrawTo(canvas);
        if ( 0 != showIconUntil ) {
            if ( millis() > showIconUntil ) {
                showIconUntil = 0;
            } else {
                canvas->drawXBitmap(
                    (TFT_WIDTH/2)-(img_mainmenu_chat_width/2),
                    (TFT_HEIGHT/2)-(img_mainmenu_chat_height/2),
                    img_mainmenu_chat_bits,img_mainmenu_chat_width,img_mainmenu_chat_height,
                    ThCol(text)
                );
                // img_mainmenu_chat
            }
        }


        TemplateApplication::Tick(); // calls to TemplateApplication::Tick()

        // draw your interface widgets here!!!
        //mywidget->DrawTo(canvas);

        nextRefresh=millis()+(1000/12); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
