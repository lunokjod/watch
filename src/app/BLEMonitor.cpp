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
#include "BLEMonitor.hpp"
#include "../static/img_back_32.xbm"
#include "../static/img_bluetooth_32.xbm"
#include "BLEDevice.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "LogView.hpp"
#include "../system/Network/BLE.hpp"
#include <esp_task_wdt.h>
#include "../system/Datasources/database.hpp"
#include "../lunokIoT.hpp"

extern SemaphoreHandle_t BLEKnowDevicesSemaphore;
//extern bool bleEnabled;
unsigned long BLEMonitorTasknextBLEScan = 0;
TaskHandle_t lunokIoT_BLEMonitorTask = NULL;
bool lunokIoT_BLEMonitorTaskLoop = false;
bool lunokIoT_BLEMonitorTaskLoopEnded = false;
size_t BLEMonitorScanLoops = 0;
bool BLEMonitorEnded=false;
int BLEMonitorApplication::rotateVal=0;
extern esp_power_level_t defaultBLEPowerLevel;

void BLEMonitorTask(void *data) {
    lunokIoT_BLEMonitorTaskLoopEnded = true;
    BLEMonitorEnded=false;
    lNetLog("BLEMonitorTask: BLE scan task starts\n");
    BLEMonitorScanLoops=0;
    while (lunokIoT_BLEMonitorTaskLoop) {
        delay(67);
        // launch when idle
        if (millis() > BLEMonitorTasknextBLEScan) {
            if (LoT().GetBLE()->IsEnabled()) {
                NimBLEScan *pBLEScan = BLEDevice::getScan();
                if (pBLEScan->isScanning()) {
                    pBLEScan->stop();
                    lNetLog("BLEMonitorTask: Scan stopped!\n");
                    delay(1000);
                    continue;
                }
                lNetLog("BLEMonitorTask: Scan %d begin\n", BLEMonitorScanLoops);
                // pBLEScan->clearDuplicateCache();
                // pBLEScan->clearResults();
                pBLEScan->setMaxResults(5);
                BLEScanResults foundDevices = pBLEScan->start(4);
                // lAppLog("BLE: Devices found: %d\n",foundDevices.getCount());
                // pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
                // pBLEScan->clearDuplicateCache();
                BLEMonitorScanLoops++;
            }
            BLEMonitorTasknextBLEScan = millis() + (5 * 1000);
        }
    }
    lNetLog("BLEMonitorTask: BLE scan task stops\n");
    lunokIoT_BLEMonitorTaskLoopEnded = false;
    BLEMonitorEnded=true;
    vTaskDelete(NULL);
}

BLEMonitorApplication::~BLEMonitorApplication()
{
    //NimBLEDevice::setPower(defaultBLEPowerLevel); // default power
    lAppLog("BLEMonitor: Waiting BLE task stop...\n");
    lunokIoT_BLEMonitorTaskLoop = false;
    while (not BLEMonitorEnded) {
        delay(100);
        esp_task_wdt_reset();   
        //lAppLog("BLEMonitor: Waiting stop...\n");
    }
    lAppLog("BLEMonitor: BLE task dies\n");
    // task is dead here, don't need vTaskDelete(lunokIoT_BLEMonitorTask);
    NimBLEScan *pBLEScan = BLEDevice::getScan();
    if (false == pBLEScan->isScanning()) {
        pBLEScan->stop();
    }
    //pBLEScan->clearDuplicateCache();
    //pBLEScan->clearResults();
}

BLEMonitorApplication::BLEMonitorApplication() {
    // max power!
    //NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    lAppLog("Already know devices:\n");
    if ( nullptr != systemDatabase ) { systemDatabase->SendSQL("SELECT COUNT(1) FROM bluetooth;"); }
    /*
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        db_exec(lIoTsystemDatabase,"SELECT COUNT(1) FROM bluetooth;");
        xSemaphoreGive( SqlLogSemaphore ); // free
    }
    */

    lunokIoT_BLEMonitorTaskLoop = true;
    xTaskCreatePinnedToCore(BLEMonitorTask, "bMonTA", LUNOKIOT_APP_STACK_SIZE, NULL, tskIDLE_PRIORITY, &lunokIoT_BLEMonitorTask,1);
    Tick(); // splash
}

bool BLEMonitorApplication::Tick() {
    UINextTimeout = millis() + UITimeout; // no sleep here
    if (millis() > nextRedraw) {
        rotateVal += 1;
        canvas->fillSprite(canvas->color24to16(0x212121));
        canvas->fillCircle(120, 120, 30, TFT_BLACK);
        canvas->fillCircle(120, 120, 28, TFT_WHITE);
        canvas->fillCircle(120, 120, 24, TFT_BLUE);
        uint16_t iconColor = TFT_WHITE;
        if ( false == LoT().GetBLE()->IsEnabled() ) { iconColor = TFT_DARKGREY; }
        canvas->drawXBitmap((TFT_WIDTH - img_bluetooth_32_width) / 2, (TFT_HEIGHT - img_bluetooth_32_height) / 2, img_bluetooth_32_bits, img_bluetooth_32_width, img_bluetooth_32_height, iconColor);
        if (BLEKnowDevices.size() > 0) {
            int elementDegrees = 360 / BLEKnowDevices.size();
            if (elementDegrees > 359)
            {
                elementDegrees = 0;
            } // 0~359 overflow
            canvas->setTextFont(0);
            canvas->setTextWrap(false, false);
            bool BTDeviceTouched=false;
            DescribeCircle(
                120, 120, 90, [&, this](int x, int y, int cx, int cy, int angle, int step, void *payload)
                {
                int currentElement = 0;
                if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS) == pdTRUE )  {
                    lBLEDevice * BTDeviceSelected = nullptr;
                    for (auto const& dev : BLEKnowDevices) {
                        esp_task_wdt_reset();
                        //if ( abs(dev->rssi) > 92 ) { currentElement++; continue; }
                        int16_t seconds = (millis()-dev->lastSeen)/1000;
                        if ( seconds > 80 ) { currentElement++; continue; } // ignore old devices
                        int searchAngle = (currentElement*elementDegrees);
                        searchAngle=((searchAngle+rotateVal)%360);
                        if ( angle == searchAngle ) {
                            int32_t radius=30;
                            uint8_t alpha = seconds*4;
                            if ( seconds > 255 ) { alpha=255; }
                            //lLog("DEV RSSI: %d\n",dev->rssi);
                            if ( seconds < 3) { // draw signal
                                radius+=2;
                                canvas->fillCircle(x,y,radius,canvas->color24to16(0x1825a9));
                                canvas->fillCircle(x,y,radius-4,canvas->color24to16(0x212121));

                                canvas->fillCircle(x,y,radius-8,canvas->color24to16(0x1825a9));
                                canvas->fillCircle(x,y,radius-12,canvas->color24to16(0x212121));
                            } else if ( seconds > 77 ) { radius-=12;
                            } else if ( seconds > 69 ) { radius-=10;
                            } else if ( seconds > 62 ) { radius-=8;
                            } else if ( seconds > 53 ) { radius-=5;
                            } else if ( seconds > 40 ) { radius-=3; }
                            canvas->fillCircle(x,y,radius-14,TFT_BLACK);
                            canvas->fillCircle(x,y,radius-16,TFT_WHITE);
                            uint16_t finalColor = canvas->alphaBlend(alpha,canvas->color24to16(0x212121),TFT_BLUE);
                            canvas->fillCircle(x,y,radius-18,finalColor);
                            if ( seconds < 5) { // draw signal
                                canvas->setTextColor(TFT_BLACK);
                                canvas->setTextDatum(BC_DATUM);
                                canvas->setTextSize(2);
                                canvas->drawString(dev->devName, x-2,y-2);
                                canvas->drawString(dev->devName, x+2,y+2);
                                canvas->setTextDatum(TC_DATUM);
                                canvas->setTextSize(1);
                                canvas->drawString(dev->addr.toString().c_str(), x+2,y+2);
                                canvas->drawString(dev->addr.toString().c_str(), x-2,y-2);

                                canvas->setTextColor(TFT_YELLOW);
                                canvas->setTextDatum(BC_DATUM);
                                canvas->setTextSize(2);
                                canvas->drawString(dev->devName, x,y);
                                canvas->setTextDatum(TC_DATUM);
                                canvas->setTextSize(1);
                                canvas->drawString(dev->addr.toString().c_str(), x,y);
                            }
                            if ( ( touched ) && ( false == BTDeviceTouched ) ) {
                                //lAppLog("InRadius x: %d y: %d\n",x,y);
                                if ( ActiveRect::InRadius(touchX, touchY,x,y,radius) ) {
                                    lAppLog("TOUCHED BT: %s\n",dev->addr.toString().c_str());
                                    BTDeviceTouched=true;
                                    BTDeviceSelected=dev;
                                    canvas->fillCircle(x,y,160,finalColor);
                                    char *macBuff= (char*)ps_malloc(19); // app must free this
                                    strncpy(macBuff,dev->addr.toString().c_str(),18);
                                    xSemaphoreGive( BLEKnowDevicesSemaphore );
                                    LaunchApplication(new BLEDeviceMonitorApplication(finalColor,macBuff));
                                    return true;
                                }
                            }
                        }
                        currentElement++;
                    }
                    xSemaphoreGive( BLEKnowDevicesSemaphore );
                }
                return true; },
                nullptr);
        }
        TemplateApplication::Tick();
        nextRedraw = millis() + (1000 / 18);
        return true;
    }
    return false;
}
