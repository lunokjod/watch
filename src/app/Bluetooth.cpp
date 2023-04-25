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
#include "Bluetooth.hpp"
#include <NimBLEDevice.h>
#include "../resources.hpp"
/*
#include "../static/img_back_32.xbm"
#include "../static/img_trash_32.xbm"
*/
#include "../static/img_lock_32.xbm"
#include "../system/Network/BLE.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "LogView.hpp"
#include <esp_task_wdt.h>
#include "../lunokIoT.hpp"

BluetoothApplication::~BluetoothApplication() {
    if ( nullptr != btnGeneratePIN ) { delete btnGeneratePIN; }
    if ( nullptr != btnRemoveBonding ) { delete btnRemoveBonding; }
}
uint32_t BluetoothApplication::SetBLERandomPin() {
    generatedPin=random(0,999999);
    lNetLog("BLE: generated PIN: %06d\n",generatedPin);
    BLEDevice::setSecurityPasskey(generatedPin);
    return generatedPin;
}

BluetoothApplication::BluetoothApplication() {
    
    btnGeneratePIN=new ButtonImageXBMWidget(TFT_WIDTH-69,TFT_HEIGHT-69,64,64,[&,this](void *bah){
        SetBLERandomPin();
    },img_lock_32_bits,img_lock_32_height,img_lock_32_width,ThCol(text),ThCol(button),false);

    btnRemoveBonding=new ButtonImageXBMWidget(5,5,64,64,[&,this](void *bah){
        lAppLog("BLE: Removing all bounded devices...\n");
        NimBLEDevice::deleteAllBonds();
        lAppLog("BLE: Bound devices removed\n");
    },img_trash_32_bits,img_trash_32_height,img_trash_32_width,ThCol(text),ThCol(high));
    generatedPin = BLEDevice::getSecurityPasskey();
    Tick();
}

bool BluetoothApplication::Tick() {
    UINextTimeout = millis() + UITimeout; // no sleep in this screen
    btnRemoveBonding->enabled=LoT().GetBLE()->IsEnabled();
    btnGeneratePIN->enabled=LoT().GetBLE()->IsEnabled();
    btnBack->Interact(touched,touchX, touchY);
    btnGeneratePIN->Interact(touched,touchX, touchY);
    btnRemoveBonding->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        btnBack->DrawTo(canvas);
        btnGeneratePIN->DrawTo(canvas);
        btnRemoveBonding->DrawTo(canvas);

        canvas->setTextFont(0);
        canvas->setTextColor(ThCol(text));
        char pinAsChar[7] = { 0 };
        sprintf(pinAsChar,"%06d",generatedPin);
        canvas->setTextSize(2);
        canvas->setTextDatum(BC_DATUM);
        canvas->drawString("BLE PIN:",TFT_WIDTH/2,TFT_HEIGHT/2);
        canvas->setTextSize(4);
        canvas->setTextDatum(TC_DATUM);
        if ( LoT().GetBLE()->IsEnabled() ) {
            canvas->drawString(pinAsChar,TFT_WIDTH/2,TFT_HEIGHT/2);
        } else {
            canvas->setTextColor(ThCol(text_alt));
            canvas->drawString("DISABLED",TFT_WIDTH/2,TFT_HEIGHT/2);
        }

        nextRedraw=millis()+(1000/10);
        return true;
    }
    return false;
}
