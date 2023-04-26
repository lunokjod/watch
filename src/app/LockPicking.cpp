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
#include "LockPicking.hpp"
#include "../UI/UI.hpp"
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "ApplicationBase.hpp"
#include "../UI/widgets/EntryTextWidget.hpp"
#include "../lunokiot_config.hpp"
extern TTGOClass *ttgo; // ttgo lib

extern void TakeBMPSample();

LockPickingGameApplication::~LockPickingGameApplication() {
    delete wristGauge;
}

LockPickingGameApplication::LockPickingGameApplication() {
    wristGauge = new GaugeWidget(10,10,220);
    levelCorrectAngle=random(aMIN,aMAX);
    lAppLog("Selected angle: %d\n",levelCorrectAngle);
    Tick();
}

bool LockPickingGameApplication::Tick() {
    UINextTimeout = millis()+UITimeout; // disable screen timeout
    TemplateApplication::Tick();
    if ( lastAngle != wristGauge->selectedAngle ) {
        wristGauge->dirty=true; // force redraw
        lastAngle=wristGauge->selectedAngle;
        //lAppLog("degX: %.2f wristGauge: %d\n",degX,wristGauge->selectedAngle);
        lAppLog("levelCorrectAngle: %d wristGauge: %d\n",levelCorrectAngle,wristGauge->selectedAngle);
        if ( (levelCorrectAngle-levelDriftAngle <=  wristGauge->selectedAngle ) 
                && (levelCorrectAngle+levelDriftAngle >=  wristGauge->selectedAngle ) ) {
            tft->fillScreen(TFT_GREEN);
            ttgo->motor->onec();
            currentLevel++;
            levelCorrectAngle=random(aMIN,aMAX);
            lAppLog("Selected angle: %d\n",levelCorrectAngle);
            levelDriftAngle--;
            if (levelDriftAngle < 0 ) {
                levelDriftAngle=0;
                playerWin=true; // YOU WIN!!!
                LaunchWatchface();
            }
            delay(200);
        }
        int16_t difference=abs(levelCorrectAngle-wristGauge->selectedAngle);
        uint16_t currentValue = ((255/360.0)*difference);
        lAppLog("DIFFERENCE: %d va: %u\n", difference,currentValue);
        nearby=currentValue;
        if ( millis() > nextHapticBeat ) {
            //if ( nearby > )
            ttgo->motor->onec(255-nearby);
            nextHapticBeat=millis()+1000;
        }
        //wristGauge->DirectDraw();
    }
    if (millis() > nextRedraw ) {
        TakeBMPSample(); // taint system about collect fresh gyroscope data (more than usual) 
        //lLog("degX: %.2f degY: %.2f degZ: %.2f\n",degX,degY,degZ);

        wristGauge->selectedAngle=degX;
        canvas->fillSprite(canvas->color565(255-nearby,255-nearby,255-nearby));

        char buffer[20];
        canvas->setTextSize(2);
        canvas->setFreeFont(&FreeMonoBold24pt7b);
        canvas->setTextDatum(CC_DATUM);
        canvas->setTextWrap(false,false);
        sprintf(buffer,"%d", currentLevel );
        canvas->setTextColor(TFT_WHITE);
        canvas->drawString(buffer, 120, 120);

        canvas->setTextSize(1);
        canvas->setFreeFont(&FreeMonoBold12pt7b);
        sprintf(buffer,"%d", wristGauge->selectedAngle );
        canvas->setTextDatum(TL_DATUM);
        canvas->drawString(buffer, 5, 5);

        canvas->setTextDatum(TR_DATUM);
        sprintf(buffer,"%d", levelDriftAngle );
        canvas->drawString(buffer, TFT_WIDTH-5, 5);


        canvas->setTextDatum(BR_DATUM);
        sprintf(buffer,"%d", levelCorrectAngle );
        canvas->drawString(buffer, TFT_WIDTH-5, TFT_HEIGHT-5);

        wristGauge->DrawTo(canvas);
        TemplateApplication::Tick();
        nextRedraw=millis()+(1000/12); // tune your required refresh
        return true;
    }
    return false;
}
