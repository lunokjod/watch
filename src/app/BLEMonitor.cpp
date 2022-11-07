#include <Arduino.h>
#include <LilyGoWatch.h>
#include "BLEMonitor.hpp"
#include "../static/img_back_32.xbm"
#include "../static/img_bluetooth_32.xbm"
#include "Watchface.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "LogView.hpp"
#include <esp_task_wdt.h>

unsigned long BLEMonitorTasknextBLEScan=0;
TaskHandle_t lunokIoT_BLEMonitorTask = NULL;
bool lunokIoT_BLEMonitorTaskLoop = false;
bool lunokIoT_BLEMonitorTaskLoopEnd = false;
void BLEMonitorTask(void * data) {
    lunokIoT_BLEMonitorTaskLoopEnd=true;
    lAppLog("BLEMonitor: BLE scan task starts\n");
    while(lunokIoT_BLEMonitorTaskLoop) {
        delay(1000);
        // launch when idle
        if (  millis() > BLEMonitorTasknextBLEScan ) {
            NimBLEScan * pBLEScan = BLEDevice::getScan();
            if ( false == pBLEScan->isScanning() ) {
                lAppLog("BLE: Scan...\n");
                BLEScanResults foundDevices = pBLEScan->start(5,true);
                //lLog("BLE: Devices found: %d\n",foundDevices.getCount());
                //pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
            }
            BLEMonitorTasknextBLEScan=millis()+(5*1000);
        }
    }
    lAppLog("BLEMonitor: BLE scan task stops\n");
    lunokIoT_BLEMonitorTaskLoopEnd=false;
    vTaskDelete(NULL);
}

BLEMonitorApplication::~BLEMonitorApplication() {
    lunokIoT_BLEMonitorTaskLoop=false;
    while(lunokIoT_BLEMonitorTaskLoopEnd) {
        delay(2);
        esp_task_wdt_reset();
    }
    //task is dead here, don't need vTaskDelete(lunokIoT_BLEMonitorTask);
    NimBLEScan * pBLEScan = BLEDevice::getScan();
    if ( false == pBLEScan->isScanning() ) { pBLEScan->stop(); }
    if ( nullptr != btnBack ) { delete btnBack; }
}

BLEMonitorApplication::BLEMonitorApplication() {
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,ttgo->tft->color24to16(0x353e45),false);
    lunokIoT_BLEMonitorTaskLoop=true;
    xTaskCreate(BLEMonitorTask, "", LUNOKIOT_TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), &lunokIoT_BLEMonitorTask);
}

bool BLEMonitorApplication::Tick() {
    UINextTimeout = millis()+UITimeout; // disable screen timeout on this app
    btnBack->Interact(touched,touchX, touchY);

    rotateVal+=1;
    if (millis() > nextRedraw ) {
        canvas->fillSprite(canvas->color24to16(0x212121));

        btnBack->DrawTo(canvas);
        canvas->fillCircle(120,120,30,TFT_BLACK);
        canvas->fillCircle(120,120,28,TFT_WHITE);
        canvas->fillCircle(120,120,24,TFT_BLUE);
        canvas->drawXBitmap((TFT_WIDTH-img_bluetooth_32_width)/2,(TFT_HEIGHT-img_bluetooth_32_height)/2,img_bluetooth_32_bits,img_bluetooth_32_width,img_bluetooth_32_height,TFT_WHITE);

        if ( BLEKnowDevices.size() > 0 ) {
            int elementDegrees = 360/BLEKnowDevices.size();
            if (elementDegrees > 359 ) { elementDegrees = 0; } // 0~359 overflow
            canvas->setTextFont(0);
            canvas->setTextWrap(false,false);
            DescribeCircle(120,120,90,[&,this](int x,int y, int cx, int cy, int angle, int step, void* payload) {
                int currentElement = 0;
                for (auto const& dev : BLEKnowDevices) {
                    esp_task_wdt_reset();
                    //if ( 1 == ( currentElement % 32 ) ) { delay(3); }
                    //if ( abs(dev->rssi) > 92 ) { currentElement++; continue; }
                    int16_t seconds = (millis()-dev->lastSeen)/1000;
                    if ( seconds > 60 ) { currentElement++; continue; } // ignore old devices
                    int searchAngle = (currentElement*elementDegrees);
                    searchAngle=((searchAngle+rotateVal)%360);
                    if ( angle == searchAngle ) {
                        uint8_t alpha = seconds*2;
                        //lLog("DEV RSSI: %d\n",dev->rssi);
                        if ( seconds > 255 ) { alpha=255; }
                        if ( seconds < 3) { // draw signal
                            canvas->fillCircle(x,y,27,canvas->color24to16(0x1825a9));
                            canvas->fillCircle(x,y,24,canvas->color24to16(0x212121));

                            canvas->fillCircle(x,y,20,canvas->color24to16(0x555f68));
                            canvas->fillCircle(x,y,17,canvas->color24to16(0x212121));
                        }
                        canvas->fillCircle(x,y,16,TFT_BLACK);
                        canvas->fillCircle(x,y,14,TFT_WHITE);
                        uint32_t finalColor = canvas->alphaBlend(alpha,canvas->color24to16(0x212121),TFT_BLUE);
                        canvas->fillCircle(x,y,12,finalColor);
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
                    }
                    currentElement++;
                }
                return true;
            },nullptr);
        }
        nextRedraw=millis()+(1000/10);
        return true;
    }
    return false;
}