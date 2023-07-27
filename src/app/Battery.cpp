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
#include "Battery.hpp"
#include "../lunokiot_config.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"
#include "../resources.hpp"
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo lib

extern float PMUBattDischarge;
extern float batteryRemainingTimeHours;
extern float batteryTotalTimeHours;

BatteryApplication::~BatteryApplication() {
    delete BattPCEvent;
    delete BattChargeEvent;
    delete BattFullEvent;
    delete batteryTempMonitor;
}

BatteryApplication::BatteryApplication() {
    // this KVO launch callback every time PMU_EVENT_BATT_PC differs from last (change detection)
    BattPCEvent = new EventKVO([&, this](){
        this->dirtyFrame = true; // mark as dirty for full redraw the next Tick
    },PMU_EVENT_BATT_PC);
    BattChargeEvent = new EventKVO([&, this](){
        charging=true;
        this->dirtyFrame = true;
    },PMU_EVENT_BATT_CHARGING); // show charging animation
    BattFullEvent = new EventKVO([&, this](){
        charging=false;
        this->dirtyFrame = true;
        //LaunchWatchface(); // battery full, return to watchface
    },PMU_EVENT_BATT_FULL);

    charging = ttgo->power->isChargeing();
    int64_t tmin = int64_t(axpTemp*10)-SCALE;
    int64_t tmax = int64_t(axpTemp*10)+SCALE;
    batteryTempMonitor = new GraphWidget(35,100,tmin,tmax);
    Tick();
}
bool BatteryApplication::Tick() {
    UINextTimeout = millis()+UITimeout; // disable screen timeout
    TemplateApplication::btnBack->Interact(touched, touchX, touchY);
    /*
    if ( vbusPresent ) {
        UINextTimeout = millis()+UITimeout; // if plugged in, don't turn off screen
    }
    */
    //if ( millis() > scaleTempRedraw ) { // reescale graph every...
    if ( false == normalRange ) {
        //lAppLog("Graph reescaled\n");
        int64_t tmin = int64_t(axpTemp*10)-SCALE;
        int64_t tmax = int64_t(axpTemp*10)+SCALE;
        batteryTempMonitor->minValue = tmin;
        batteryTempMonitor->maxValue = tmax;
        //scaleTempRedraw=millis()+(1000*15);
        normalRange=true;
        return false;
    }

    if ( millis() > pollTempRedraw ) { // push data
        normalRange = batteryTempMonitor->PushValue(int64_t(axpTemp*10)); // multiply to not loose precision on graph
        batteryTempMonitor->PushValue(int64_t(axpTemp*10)); // multiply to not loose precision on graph
        batteryTempMonitor->PushValue(int64_t(axpTemp*10)); // multiply to not loose precision on graph
        batteryTempMonitor->PushValue(batteryTempMonitor->minValue); // space
        //batteryTempMonitor->PushValue(batteryTempMonitor->minValue); // space
        //lAppLog("AXP BATT VALUE %lld, min: %lld, max: %lld, charging: '%s'\n",int64_t(axpTemp*10),batteryTempMonitor->minValue,batteryTempMonitor->maxValue,(charging?"yes":"no"));
        pollTempRedraw=millis()+(1000*2);
        return false;
    }
    if (millis() > nextRedraw ) {
        charging = ttgo->power->isChargeing();
        //if ( this->dirtyFrame ) { // only refresh when dirty
        canvas->fillSprite(TFT_BLACK);
        canvas->pushImage(TFT_WIDTH-90,40,img_battery_empty_160.width,img_battery_empty_160.height, (uint16_t *)img_battery_empty_160.pixel_data);
        batteryTempMonitor->DrawTo(canvas,25,canvas->height()-(batteryTempMonitor->canvas->height()+64));
        char buffer[30];

        sprintf(buffer,"%.1f C", axpTemp);
        canvas->setFreeFont(&FreeMonoBold9pt7b);
        canvas->setTextSize(1);
        canvas->setTextDatum(BC_DATUM);
        canvas->setTextColor(TFT_WHITE);
        //int16_t battTextHeight =  58+restBattCanvasHeight;
        //if ( battTextHeight > 140 ) { battTextHeight = 140; }
        canvas->drawString(buffer, 25+(batteryTempMonitor->canvas->width()/2), (canvas->height()-68)-batteryTempMonitor->canvas->height());

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

            sprintf(buffer,"%d%%", batteryPercent);
            canvas->setFreeFont(&FreeMonoBold24pt7b);
            canvas->setTextSize(1);
            canvas->setTextDatum(TC_DATUM);
            canvas->setTextColor(TFT_WHITE);
            //int16_t battTextHeight =  58+restBattCanvasHeight;
            //if ( battTextHeight > 140 ) { battTextHeight = 140; }
            canvas->drawString(buffer, 75, 58);
        }
        if ( false == charging ) {
            if ( PMUBattDischarge > 0.0 ) {
                sprintf(buffer,"Usage: %.2f mAh",PMUBattDischarge);
                canvas->setTextFont(0);
                canvas->setTextSize(2);
                canvas->setTextColor(ThCol(text));
                canvas->setTextDatum(TC_DATUM);
                canvas->drawString(buffer, canvas->width()/2, 10);
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
        //}
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
