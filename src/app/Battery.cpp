#include <Arduino.h>
#include "Battery.hpp"
#include "../lunokiot_config.hpp"
#include "../static/img_battery_empty_160.c"
#include "../static/img_power_64.xbm"

#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo lib

extern float PMUBattDischarge;
extern float batteryRemainingTimeHours;
extern float batteryTotalTimeHours;



BatteryApplication::~BatteryApplication() {
    delete BattPCEvent;
    delete BattChargeEvent;
    delete BattFullEvent;
}

BatteryApplication::BatteryApplication() {
    TemplateApplication();
    // this KVO launch callback every time PMU_EVENT_BATT_PC differs from last (change detection)
    BattPCEvent = new EventKVO([&, this](){
        this->dirtyFrame = true; // mark as dirty for full redraw the next Tick
    },PMU_EVENT_BATT_PC);
    BattChargeEvent = new EventKVO([&, this](){
        this->dirtyFrame = true;
    },PMU_EVENT_BATT_CHARGING); // show charging animation
    BattFullEvent = new EventKVO([&, this](){
        charging=false;
        this->dirtyFrame = true;
        //LaunchWatchface(); // battery full, return to watchface
    },PMU_EVENT_BATT_FULL);

    charging = ttgo->power->isChargeing();
    Tick();
}
bool BatteryApplication::Tick() {
    TemplateApplication::btnBack->Interact(touched, touchX, touchY);
    if ( vbusPresent ) {
        UINextTimeout = millis()+UITimeout; // if plugged in, don't turn off screen
    }

    if (millis() > nextRedraw ) {
        if ( this->dirtyFrame ) { // only refresh when dirty
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
            charging = ttgo->power->isChargeing();
            if ( false == charging ) {
                char buffer[30];
                if ( PMUBattDischarge > 0.0 ) {
                    sprintf(buffer,"Usage: %.2f mA",PMUBattDischarge);
                    canvas->setTextFont(0);
                    canvas->setTextSize(2);
                    canvas->setTextColor(ThCol(text));
                    canvas->setTextDatum(TL_DATUM);
                    canvas->drawString(buffer, 10, 10);
                }
                if ( batteryRemainingTimeHours > 0.0 ) {
                    int hours = fabs(batteryRemainingTimeHours);
                    int minutes = (batteryRemainingTimeHours-hours)*60;
                    sprintf(buffer,"%d hrs %d min",hours, minutes);
                    canvas->setTextFont(0);
                    canvas->setTextSize(2);
                    canvas->setTextColor(TFT_WHITE);
                    canvas->setTextDatum(BL_DATUM);
                    canvas->drawString(buffer, 80, canvas->width()-15);
                }
            }
            this->dirtyFrame = false;
        }
        if ( charging ) {
            int16_t calculatedBattCanvasHeight = (chargingAnimOffset*battHeight/100);
            int16_t restBattCanvasHeight = battHeight - calculatedBattCanvasHeight;
            canvas->fillRect(TFT_WIDTH-80,58+restBattCanvasHeight,69,calculatedBattCanvasHeight,TFT_GREEN);

            canvas->drawXBitmap(TFT_WIDTH-80+17+1,58+33+1,img_power_64_bits, img_power_64_width, img_power_64_height, TFT_BLACK);
            canvas->drawXBitmap(TFT_WIDTH-80+17,58+33,img_power_64_bits, img_power_64_width, img_power_64_height, TFT_WHITE);

            chargingAnimOffset+=10;
            if ( chargingAnimOffset > 100 ) {
                chargingAnimOffset=0;
                this->dirtyFrame=true;
            }
        }
        TemplateApplication::btnBack->DrawTo(canvas);
        nextRedraw=millis()+(1000/3);
        return true;
    }
    return false;
}
