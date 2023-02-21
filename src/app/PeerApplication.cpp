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
#include <esp_now.h>
#include <WiFi.h>
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo library shit ;)
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "PeerApplication.hpp"
#include "../static/img_chat_32.xbm"
#include "../static/img_hi_32.xbm"
#include "../static/img_hi_48.xbm"
#include "../static/img_ask_48.xbm"
#include "../static/img_walk_48.xbm"
#include "../static/img_flag_48.xbm"
#include "../static/img_fast_48.xbm"
#include "../static/img_slow_48.xbm"
#include "../static/img_home_48.xbm"
#include "../static/img_next_48.xbm"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"

const char *LUNOKIOT_ESPNOW_EMOJI = "lIoTEmoji";

typedef struct {
    int16_t width;
    int16_t height;
    unsigned char * data;
} PeerIconData;

PeerIconData emojiArray[] = {
    { img_hi_48_width, img_hi_48_height, img_hi_48_bits },
    { img_ask_48_width, img_ask_48_height, img_ask_48_bits },
    { img_walk_48_width, img_walk_48_height, img_walk_48_bits },
    { img_flag_48_width, img_flag_48_height, img_flag_48_bits },
    { img_fast_48_width, img_fast_48_height, img_fast_48_bits },
    { img_slow_48_width, img_slow_48_height, img_slow_48_bits },
    { img_home_48_width, img_home_48_height, img_home_48_bits },
    { img_next_48_width, img_next_48_height, img_next_48_bits },
};

uint8_t broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
esp_now_peer_info_t peerInfo;

uint8_t espNowMAC[6];                  // 6 octets are the WIFI address

bool newNotification=false;
unsigned long showIconUntil=0;
uint8_t lastPeerMAC[6];                  // 6 octets are their WIFI address
size_t lastPeerEmojiID=0;

typedef struct {
    char command[10];
    uint8_t emojiID;
} lIoTESPNowDataPackage;

void PeerApplication::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    //lAppLog("ESPNOW: Last Packet Send Status: %s\n",((status == ESP_NOW_SEND_SUCCESS)?"Success":"FAILED"));
}

void PeerApplication::OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    lLog("Bytes received: %d byte from: '%02x:%02x:%02x:%02x:%02x:%02x'\n",len,
    mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    lIoTESPNowDataPackage *receiveData = (lIoTESPNowDataPackage*)incomingData;
    if ( 0 == strncmp(LUNOKIOT_ESPNOW_EMOJI,receiveData->command,strlen(LUNOKIOT_ESPNOW_EMOJI))) {
        memcpy(lastPeerMAC,mac,6);
#ifdef LILYGO_WATCH_2020_V3
        ttgo->shake();
#endif
        lastPeerEmojiID=receiveData->emojiID;
        newNotification=true;
    }
    /*
    if ( 0 == strncmp(PEER_HELLO,dataBuffer,strlen(PEER_HELLO))) {
        memcpy(lastPeerMAC,mac,6);
        //lAppLog("Peer says hello!\n");
        ttgo->shake();
        newNotification=true;
        showIconUntil=millis()+3000;
    }*/
    /*
    else if ( 0 == strncmp(PEER_TIMESTAMP,dataBuffer,strlen(PEER_TIMESTAMP))) {
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
    }*/
}

PeerApplication::PeerApplication() {
    canvas->fillSprite(ThCol(background)); // use theme colors
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
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
    peerInfo.priv = (void *)this;
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        lAppLog("ESPNOW: Failed to add peer\n");
    }
    iconPaletteBtn=new ButtonImageXBMWidget(TFT_WIDTH-69,TFT_HEIGHT-69,64,64,[&](void *bah){
        emojiPaletteVisible=(!emojiPaletteVisible);
    },img_chat_32_bits,img_chat_32_height,img_chat_32_width);
    
    sendEmojiBtn=new ButtonImageXBMWidget(68,TFT_HEIGHT-69,64,102,[&,this](void *bah){
        lIoTESPNowDataPackage myCurrentData;
        strncpy(myCurrentData.command,LUNOKIOT_ESPNOW_EMOJI,strlen(LUNOKIOT_ESPNOW_EMOJI));
        myCurrentData.emojiID= (uint8_t)selectedEmojiOffset;
        // = { LUNOKIOT_ESPNOW_EMOJI, (uint8_t)selectedEmojiOffset };
        // Send message via ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t*)&myCurrentData, sizeof(myCurrentData));
        if (result == ESP_OK) {
            //lAppLog("ESPNOW: Sent with success\n");
        } else {
            //lAppLog("ESPNOW: Error sending\n");
        }
        // draw my message
        scrollZone->canvas->scroll(0,-64);
        int16_t sZw = scrollZone->canvas->width();
        int16_t sZh = scrollZone->canvas->height();
        scrollZone->canvas->fillRoundRect(0,sZh-62,sZw-10,62,10,ThCol(darken));
        scrollZone->canvas->fillTriangle(sZw, sZh-10, sZw-12, sZh-10, sZw-12, sZh-20,ThCol(darken));
        scrollZone->canvas->drawRoundRect(0,sZh-62,sZw-10,62,10,ThCol(dark));

        scrollZone->canvas->drawXBitmap(
            25,
            sZh-52,
            emojiArray[selectedEmojiOffset].data,emojiArray[selectedEmojiOffset].width,emojiArray[selectedEmojiOffset].height,
            ThCol(shadow)
        );
        scrollZone->canvas->drawXBitmap(
            23,
            sZh-54,
            emojiArray[selectedEmojiOffset].data,emojiArray[selectedEmojiOffset].width,emojiArray[selectedEmojiOffset].height,
            ThCol(text)
        );
        //char macAsChar[18] = { 0 };
        //sprintf(macAsChar,"%02x%02x:",lastPeerMAC[4],lastPeerMAC[5]);
        scrollZone->canvas->setTextFont(0);
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            char timeChar[18] = { 0 };
            sprintf(timeChar,"%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min);
            scrollZone->canvas->setTextSize(1);
            scrollZone->canvas->setTextDatum(TR_DATUM);
            scrollZone->canvas->setTextColor(ThCol(shadow));
            scrollZone->canvas->drawString(timeChar,sZw-32,(scrollZone->canvas->height()-28));
            scrollZone->canvas->setTextColor(ThCol(text));
            scrollZone->canvas->drawString(timeChar,sZw-30,(scrollZone->canvas->height()-30));
        }
        const char myselfChar[] =":You";
        scrollZone->canvas->setTextSize(2);
        scrollZone->canvas->setTextDatum(BR_DATUM);
        scrollZone->canvas->setTextColor(ThCol(shadow));
        scrollZone->canvas->drawString(myselfChar,sZw-32,(scrollZone->canvas->height()-28));
        scrollZone->canvas->setTextColor(ThCol(text));
        scrollZone->canvas->drawString(myselfChar,sZw-30,(scrollZone->canvas->height()-30));



    },
    emojiArray[selectedEmojiOffset].data,
    emojiArray[selectedEmojiOffset].height,
    emojiArray[selectedEmojiOffset].width);

    colorCanvas = new CanvasWidget(TFT_HEIGHT-100,TFT_WIDTH-60);
    emojiPaletteCanvas = new CanvasWidget(144,144);
    emojiPaletteCanvas->canvas->fillRoundRect(0,0,emojiPaletteCanvas->canvas->width(),emojiPaletteCanvas->canvas->height(),8,TFT_WHITE);

    emojiPaletteCanvas->canvas->drawXBitmap(0,0,img_hi_48_bits,img_hi_48_width,img_hi_48_height,TFT_BLACK);
    emojiPaletteCanvas->canvas->drawXBitmap(48,0,img_ask_48_bits,img_ask_48_width,img_ask_48_height,TFT_BLACK);
    emojiPaletteCanvas->canvas->drawXBitmap(96,0,img_walk_48_bits,img_walk_48_width,img_walk_48_height,TFT_BLACK);

    emojiPaletteCanvas->canvas->drawXBitmap(0,48,img_flag_48_bits,img_flag_48_width,img_flag_48_height,TFT_BLACK);
    emojiPaletteCanvas->canvas->drawXBitmap(48,48,img_fast_48_bits,img_fast_48_width,img_fast_48_height,TFT_BLACK);
    emojiPaletteCanvas->canvas->drawXBitmap(96,48,img_slow_48_bits,img_slow_48_width,img_slow_48_height,TFT_BLACK);

    emojiPaletteCanvas->canvas->drawXBitmap(0,96,img_home_48_bits,img_home_48_width,img_home_48_height,TFT_BLACK);
    emojiPaletteCanvas->canvas->drawXBitmap(48,96,img_next_48_bits,img_next_48_width,img_next_48_height,TFT_BLACK);
//    emojiPaletteCanvas->canvas->drawXBitmap(96,48,img_slow_48_bits,img_slow_48_width,img_slow_48_height,TFT_BLACK);

    iconPaletteArea= new ActiveRect(
        (canvas->width()/2)-(emojiPaletteCanvas->canvas->width()/2),
        (canvas->height()/2)-(emojiPaletteCanvas->canvas->height()/2),
        emojiPaletteCanvas->canvas->height(),emojiPaletteCanvas->canvas->width(),[&](void *bah){
            selectedEmojiOffset++;
            if ( selectedEmojiOffset > (sizeof(emojiArray)/sizeof(emojiArray[0]))-1 ) {
                selectedEmojiOffset = 0;
            }
            lAppLog("selectedEmojiOffset: %d\n",selectedEmojiOffset);
            
            // hack my own buttons x'DD
            sendEmojiBtn->XBMBitmapPtr = emojiArray[selectedEmojiOffset].data;
            sendEmojiBtn->xbmH = emojiArray[selectedEmojiOffset].height;
            sendEmojiBtn->xbmW = emojiArray[selectedEmojiOffset].width;
            emojiPaletteVisible=false;
        });


    scrollZone = new CanvasWidget(TFT_HEIGHT-100,TFT_WIDTH-60);
    scrollZone->canvas->fillSprite(CanvasWidget::MASK_COLOR);
    scrollZone->canvas->setScrollRect(0,0,scrollZone->canvas->width(),scrollZone->canvas->height(),CanvasWidget::MASK_COLOR);
    Tick(); // OR call this if no splash 
}

PeerApplication::~PeerApplication() {
    // please remove/delete/free all to avoid leaks!
    esp_now_unregister_recv_cb();
    esp_now_unregister_send_cb();
    esp_now_deinit();
    WiFi.mode(WIFI_OFF);
    delete colorCanvas;
    delete sendEmojiBtn;
    delete iconPaletteBtn;
    delete scrollZone;
    delete iconPaletteArea;
    delete emojiPaletteCanvas;
}

bool PeerApplication::Tick() {
    UINextTimeout = millis()+UITimeout; // disable screen timeout during this app run

    // put your interacts here:
    //mywidget->Interact(touched,touchX,touchY);
    
    /*
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
        nextTimestampSend=millis()+(1000*15);
    }*/
    if ( millis() > nextRefresh ) {
        canvas->fillSprite(ThCol(background)); // use theme colors

        canvas->setTextFont(0);
        canvas->setTextColor(ThCol(text_alt));
        canvas->setTextSize(1);
        canvas->setTextDatum(TC_DATUM);
        canvas->drawString(WiFi.macAddress().c_str(),TFT_WIDTH/2,10);

        sendEmojiBtn->Interact(touched,touchX,touchY);
        iconPaletteBtn->Interact(touched,touchX,touchY);
        sendEmojiBtn->DrawTo(canvas);

        iconPaletteBtn->DrawTo(canvas);
        
        if ( newNotification ) {
            scrollZone->canvas->scroll(0,-64);
            uint16_t deviceColor = lastPeerMAC[4];
            deviceColor = deviceColor << 8;
            deviceColor = deviceColor +lastPeerMAC[5];
            deviceColor = canvas->alphaBlend(192,deviceColor,TFT_WHITE); // more lighten

            //text bubble
            int16_t sZw = scrollZone->canvas->width();
            int16_t sZh = scrollZone->canvas->height();
            scrollZone->canvas->fillRoundRect(10,sZh-62,sZw-10,62,10,ThCol(background_alt));
            scrollZone->canvas->fillTriangle(0, sZh-10, 12, sZh-10, 12, sZh-20, ThCol(background_alt));
            scrollZone->canvas->drawRoundRect(10,sZh-62,sZw-10,62,10,ThCol(dark));
            //scrollZone->canvas->drawTriangle(0, sZh-5, 10, sZh, 10, sZh-10, ThCol(dark));
            //lAppLog("DEVICE COLOR: %04x\n", deviceColor);

            scrollZone->canvas->drawXBitmap(
                scrollZone->canvas->width()-(emojiArray[lastPeerEmojiID].width+25),
                scrollZone->canvas->height()-52,
                emojiArray[lastPeerEmojiID].data,emojiArray[lastPeerEmojiID].width,emojiArray[lastPeerEmojiID].height,
                ThCol(shadow)
            );
            scrollZone->canvas->drawXBitmap(
                scrollZone->canvas->width()-(emojiArray[lastPeerEmojiID].width+23),
                scrollZone->canvas->height()-54,
                emojiArray[lastPeerEmojiID].data,emojiArray[lastPeerEmojiID].width,emojiArray[lastPeerEmojiID].height,
                deviceColor
            );
            char macAsChar[18] = { 0 };
            sprintf(macAsChar,"%02x%02x:",lastPeerMAC[4],lastPeerMAC[5]);
            scrollZone->canvas->setTextFont(0);
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                char timeChar[18] = { 0 };
                sprintf(timeChar,"%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min);
                scrollZone->canvas->setTextSize(1);
                scrollZone->canvas->setTextDatum(TL_DATUM);
                scrollZone->canvas->setTextColor(ThCol(shadow));
                scrollZone->canvas->drawString(timeChar,32,(scrollZone->canvas->height()-28));
                scrollZone->canvas->setTextColor(deviceColor);
                scrollZone->canvas->drawString(timeChar,30,(scrollZone->canvas->height()-30));
            }
            scrollZone->canvas->setTextSize(2);
            scrollZone->canvas->setTextDatum(BL_DATUM);
            scrollZone->canvas->setTextColor(ThCol(shadow));
            scrollZone->canvas->drawString(macAsChar,32,(scrollZone->canvas->height()-28));
            scrollZone->canvas->setTextColor(deviceColor);
            scrollZone->canvas->drawString(macAsChar,30,(scrollZone->canvas->height()-30));

            newNotification=false;
        }

        colorCanvas->canvas->fillSprite(CanvasWidget::MASK_COLOR);
        scrollZone->DrawTo(colorCanvas->canvas); // this crap is for GBR RGB
        colorCanvas->DrawTo(canvas,30,25);

        if ( emojiPaletteVisible ) {
            iconPaletteArea->Interact(touched,touchX,touchY);
            emojiPaletteCanvas->DrawTo(canvas,
            (canvas->width()/2)-(emojiPaletteCanvas->canvas->width()/2),
            (canvas->height()/2)-(emojiPaletteCanvas->canvas->height()/2));
        }
        TemplateApplication::Tick(); // calls to TemplateApplication::Tick()

        nextRefresh=millis()+(1000/16);
        return true;
    }
    return false;
}
