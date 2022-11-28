#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Bluetooth.hpp"
#include <NimBLEDevice.h>
#include "../static/img_back_32.xbm"
#include "../static/img_lock_32.xbm"
#include "../static/img_trash_32.xbm"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "LogView.hpp"

extern bool bleEnabled;
BluetoothApplication::~BluetoothApplication() {
    if ( nullptr != btnBack ) { delete btnBack; }
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
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchWatchface();
    },img_back_32_bits,img_back_32_height,img_back_32_width,ThCol(text),ThCol(button),false);
    
    btnGeneratePIN=new ButtonImageXBMWidget(TFT_WIDTH-69,TFT_HEIGHT-69,64,64,[&,this](){
        SetBLERandomPin();
    },img_lock_32_bits,img_lock_32_height,img_lock_32_width,ThCol(text),ThCol(button),false);

    btnRemoveBonding=new ButtonImageXBMWidget(5,5,64,64,[&,this](){
        lAppLog("BLE: Bound devices removed\n");
        NimBLEDevice::deleteAllBonds();
    },img_trash_32_bits,img_trash_32_height,img_trash_32_width,ThCol(text),ThCol(high));
    generatedPin = BLEDevice::getSecurityPasskey();
    Tick();
}

bool BluetoothApplication::Tick() {
    UINextTimeout = millis() + UITimeout; // no sleep in this screen
    btnRemoveBonding->enabled=bleEnabled;
    btnGeneratePIN->enabled=bleEnabled;
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
        if ( bleEnabled ) {
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
