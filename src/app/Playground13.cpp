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
#include <WiFi.h>
#include "LogView.hpp" // log capabilities
#include "Playground13.hpp"
#include <esp_now.h>

extern uint8_t espNowMAC[6];                  // 6 octets are the WIFI address
extern uint8_t broadcastAddress[];
//extern uint8_t broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
extern esp_now_peer_info_t peerInfo;

enum {
    ESPNOW_TEXT_MESSAGE,
    ESPNOW_COORDINATES,
};

typedef struct {
    size_t packageType=ESPNOW_TEXT_MESSAGE;
    char text[255];
} ESPNowTextMessage;

typedef struct {
    size_t packageType=ESPNOW_COORDINATES;
    int16_t x;
    int16_t y;
} ESPNowCoordinates;


PlaygroundApplication13::PlaygroundApplication13() {
    // Init here any you need
    lAppLog("Hello from PlaygroundApplication13!\n");

    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        lAppLog("Error initializing ESP-NOW\n");
    }
    // Once ESPNow is successfully Init, we will register for Send CB to
    esp_now_register_send_cb(PlaygroundApplication13::OnDataSent);
    esp_now_register_recv_cb(PlaygroundApplication13::OnDataRecv);

    esp_read_mac(espNowMAC,ESP_MAC_WIFI_STA);    // get from esp-idf :-*

    lAppLog("ESPNOW: My MAC address is: '%s'\n",WiFi.macAddress().c_str());
 
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    peerInfo.priv = (void *)this;
    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        lAppLog("ESPNOW: Failed to add peer\n");
    }
    delay(100);
    ESPNowTextMessage sayHello;
    strcpy(sayHello.text,"Hello!!!!");
    esp_now_send(broadcastAddress, (const uint8_t*)&sayHello, sizeof(sayHello));

    Tick(); // OR call this if no splash 
}

PlaygroundApplication13::~PlaygroundApplication13() {
    esp_now_unregister_recv_cb();
    esp_now_unregister_send_cb();
    esp_now_deinit();
    WiFi.mode(WIFI_OFF);
}

void PlaygroundApplication13::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    lLog("ESPNOW: Last Packet Send Status: %s\n",((status == ESP_NOW_SEND_SUCCESS)?"Success":"FAILED"));
}

void PlaygroundApplication13::OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    lLog("ESPNOW: Received: %d bytes from: '%02x:%02x:%02x:%02x:%02x:%02x'\n",len,
    mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    size_t * askMessage = (size_t *)incomingData;
    if ( ESPNOW_TEXT_MESSAGE == *askMessage) {
        ESPNowTextMessage *myMessage=(ESPNowTextMessage *)incomingData;
        lLog("MESSAGE RECEIVED: '%s'\n", myMessage->text);
    } else if ( ESPNOW_COORDINATES == *askMessage) {
        ESPNowCoordinates *myMessage=(ESPNowCoordinates *)incomingData;
        lLog("COORDINATES RECEIVED: X: %d Y: %d\n", myMessage->x, myMessage->y);
    } else {
        lLog("UNKOWN MESSAGE RECEIVED TYPE: %d\n", *askMessage);
    }
}

bool PlaygroundApplication13::Tick() {
    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(ThCol(background)); // use theme colors
        TemplateApplication::Tick();
        if ( touched ) {
            ESPNowCoordinates myCoords;
            myCoords.x=touchX;
            myCoords.y=touchY;
            esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t*)&myCoords, sizeof(myCoords));
        }
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
