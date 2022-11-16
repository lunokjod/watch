#include <Arduino.h>
#include "Battery.hpp"
#include "../lunokiot_config.hpp"
#include "../static/img_battery_empty_160.c"
#include "../static/img_power_64.xbm"

#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo lib

BatteryApplication::~BatteryApplication() {
    if ( nullptr != BattPCEvent ) {
        delete BattPCEvent;
        BattPCEvent = nullptr;
    }
    if ( nullptr != btnBack ) {
        delete btnBack;
        btnBack = nullptr;
    }
}

BatteryApplication::BatteryApplication() {
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        this->dirtyFrame = true;    // first full redraw must be
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,canvas->color24to16(0x353e45),false);    

    // this KVO launch callback every time PMU_EVENT_BATT_PC differs from last (change detection)
    BattPCEvent = new EventKVO([&, this](){
        this->dirtyFrame = true; // mark as dirty for full redraw the next Tick
    },PMU_EVENT_BATT_PC);
    Tick();
}
bool BatteryApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY); // user touch the button?

    if ( this->dirtyFrame ) { // only refresh when dirty
        if (millis() > nextRedraw ) {
            canvas->fillSprite(TFT_BLACK);
            canvas->pushImage(TFT_WIDTH-90,40,img_battery_empty_160.width,img_battery_empty_160.height, (uint16_t *)img_battery_empty_160.pixel_data);
            if ( -1 != batteryPercent ) {
                uint32_t battColor = TFT_DARKGREY;
                if ( vbusPresent ) {
                    battColor = TFT_GREEN;
                } else if ( batteryPercent < 10 ) {
                        battColor = TFT_RED;
                } else if ( batteryPercent < 35 ) { battColor = TFT_YELLOW; }
                else if ( batteryPercent > 70 ) { battColor = TFT_GREEN; }
                const int16_t battHeight = 133;
                int16_t calculatedBattCanvasHeight = (batteryPercent*battHeight/100);
                int16_t restBattCanvasHeight = battHeight - calculatedBattCanvasHeight;
                canvas->fillRect(TFT_WIDTH-80,58+restBattCanvasHeight,69,calculatedBattCanvasHeight,battColor);

                char textBuffer[24] = { 0 };
                sprintf(textBuffer,"%d%%", batteryPercent);
                canvas->setFreeFont(&FreeMonoBold24pt7b);
                canvas->setTextSize(1);
                canvas->setTextDatum(TC_DATUM);
                canvas->setTextColor(TFT_WHITE);
                int16_t battTextHeight =  58+restBattCanvasHeight;
                if ( battTextHeight > 140 ) { battTextHeight = 140; }
                canvas->drawString(textBuffer, 75, battTextHeight);
            }
            if ( ttgo->power->isChargeing() ) {
                canvas->drawXBitmap(TFT_WIDTH-80+17+1,58+33+1,img_power_64_bits, img_power_64_width, img_power_64_height, TFT_BLACK);
                canvas->drawXBitmap(TFT_WIDTH-80+17,58+33,img_power_64_bits, img_power_64_width, img_power_64_height, TFT_WHITE);
            }
            btnBack->DrawTo(canvas);
           nextRedraw=millis()+(1000/1);
        }
        this->dirtyFrame = false;
    }
    return true;
}