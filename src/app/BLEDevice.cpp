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
#include <LilyGoWatch.h>
#include "BLEDevice.hpp"
#include "BLEMonitor.hpp"
#include "../system/Network/BLE.hpp"
#include "../resources.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "LogView.hpp"
#include <esp_task_wdt.h>

extern SemaphoreHandle_t BLEKnowDevicesSemaphore;

BLEDeviceMonitorApplication::~BLEDeviceMonitorApplication() {
    if ( nullptr != deviceMAC ) {
        free(deviceMAC);
        deviceMAC=nullptr;
    }
    if ( nullptr != devName ) {
        free(devName);
        devName=nullptr;
    }
    if ( nullptr != pairBtn ) {
        delete pairBtn;
        pairBtn=nullptr;
    }
}

BLEDeviceMonitorApplication::BLEDeviceMonitorApplication(uint16_t baseColor,char *devMAC):backgroundColor(baseColor),deviceMAC(devMAC) {
    // reconfigure back button from template to do other things instead back to watchface
    TemplateApplication::btnBack->tapActivityCallback = [](void *payload) { LaunchApplication(new BLEMonitorApplication()); };
    TemplateApplication::btnBack->xbmColor=TFT_WHITE; // change the color
    TemplateApplication::btnBack->InternalRedraw(); // must redraw the widget
    NimBLEAddress addToinspect = NimBLEAddress(std::string(deviceMAC));
    if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS) == pdTRUE )  {
        lBLEDevice * BTDeviceSelected = nullptr;
        bool found=false;
        for (auto const& dev : BLEKnowDevices) {
            esp_task_wdt_reset();
            if ( addToinspect == dev->addr ) {
                BTDeviceSelected=dev;
                break;
            }
        }
        if ( nullptr != BTDeviceSelected ) {
            lAppLog("MAC Found on %p for: '%s'\n",BTDeviceSelected,deviceMAC);
            if ( nullptr != BTDeviceSelected->devName) {
                size_t len = strlen(BTDeviceSelected->devName);
                if ( len > 0 ) {
                    devName = (char*)ps_malloc(len+1);
                    strncpy(devName,BTDeviceSelected->devName,len);
                }
            }
            firstSeen = BTDeviceSelected->firstSeen;
            lastSeen = BTDeviceSelected->lastSeen;
            seenCount = BTDeviceSelected->seenCount;
            rssi = BTDeviceSelected->rssi;
            txPower = BTDeviceSelected->txPower;
            distance = BTDeviceSelected->distance;
        }
        xSemaphoreGive(BLEKnowDevicesSemaphore);
    }
    pairBtn = new ButtonTextWidget(60,TFT_HEIGHT-52,42,160,[](void* payload){
        lLog("@TODO PAIR WITH DEVICE\n");
    },"Pair");
    pairBtn->SetEnabled(false);
    Tick();
}

bool BLEDeviceMonitorApplication::Tick() {
    UINextTimeout = millis() + UITimeout; // disable screen timeout on this app
    TemplateApplication::btnBack->Interact(touched,touchX,touchY); // force check (speedup response)
    if (millis() > nextRedraw) {
        rotateVal += 1;
        canvas->fillSprite(backgroundColor);
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(TC_DATUM);
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        int16_t x = TFT_WIDTH/2;
        int16_t y = 10;

        if (nullptr != devName ) {
            canvas->drawString(devName, x,y);
        }
        if (nullptr != deviceMAC ) {
            canvas->drawString(deviceMAC, x,y+20);
        }
        char buffer[100];
        sprintf(buffer,"first: %lu", millis()-firstSeen);
        canvas->drawString(buffer, x,y+40);

        sprintf(buffer,"last: %lu", millis()-lastSeen);
        canvas->drawString(buffer, x,y+60);

        sprintf(buffer,"times: %u", seenCount);
        canvas->drawString(buffer, x,y+80);

        sprintf(buffer,"rssi: %d", rssi);
        canvas->drawString(buffer, x,y+100);

        sprintf(buffer,"tx: %d", txPower);
        canvas->drawString(buffer, x,y+120);
        sprintf(buffer,"dist: %g", distance);
        canvas->drawString(buffer, x,y+140);

        pairBtn->Interact(touched,touchX,touchY);
        pairBtn->DrawTo(canvas);
        
        TemplateApplication::Tick();
        nextRedraw = millis() + (1000 / 18);
        return true;
    }
    return false;
}