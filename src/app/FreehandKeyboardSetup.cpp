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
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "FreehandKeyboardSetup.hpp"
#include "../UI/widgets/ButtonTextWidget.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
//#include "../static/img_trash_32.xbm"
#include "../resources.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "FreehandKeyboardTraining.hpp"
#include "FreehandKeyboardQuery.hpp"
#include <ArduinoNvs.h>
#include <LilyGoWatch.h>
#include "../system/Datasources/perceptron.hpp"

#include "../../static/freeHandKeyboardPreTrained.h"


void FreeHandKeyboardSetupApplication::SavePerceptronWithSymbol(char symbol,Perceptron *item) {
    if ( nullptr == item) { return; }

    //Where to save that? (gather details)
    char * ptrToStructPerceptron = (char *)item;
    size_t ptrToStructPerceptronSize=sizeof(Perceptron);
    char * ptrToWeightsPerceptron = (char *)item->weights_;
    size_t ptrToWeightsPerceptronSize=(item->numInputs_)*sizeof(double);
    size_t totalSize=ptrToWeightsPerceptronSize+ptrToStructPerceptronSize;
    if ( ' ' == symbol ) { symbol = '_'; }
    lAppLog("Saving current perceptron %p to '%c' (%zu bytes)...\n",item,symbol,totalSize);

    // intermediate concatenation
    char *bufferdata = (char *)ps_malloc(totalSize); // create buffer "packed file"
    memcpy(bufferdata,ptrToStructPerceptron,ptrToStructPerceptronSize); // put the struct
    memcpy(bufferdata+ptrToStructPerceptronSize,ptrToWeightsPerceptron,ptrToWeightsPerceptronSize); // put the data weights
    

    // Save to NVS blob
    char keyName[15];
    sprintf(keyName,"freeHandSym_%c", symbol);
    bool ok = NVS.setBlob(keyName, (uint8_t*)bufferdata, totalSize,false);
    if (false == ok) { lAppLog("NVS: ERROR: Unable to save '%s'\n",keyName); }
    free(bufferdata); // free buffer shit packed data
}

FreeHandKeyboardSetupApplication::FreeHandKeyboardSetupApplication() {
    btnEraseTrain=new ButtonImageXBMWidget(5,5,54,54,[&,this](void *unused){
        for(size_t c=0;c<FreehandKeyboardLetterTrainableSymbolsCount;c++) {
            char current=FreehandKeyboardLetterTrainableSymbols[c];
            char keyName[15];
            if ( ' ' == current ) { current = '_'; } // changed to
            sprintf(keyName,"freeHandSym_%c", current);
            NVS.erase(keyName,false);
            lAppLog("NVS: Forgot '%c' data\n",current);
        }
    },img_trash_32_bits,img_trash_32_height,img_trash_32_width,ThCol(text),ThCol(high));

    trainButton=new ButtonTextWidget(105,5,54,130,[&,this](void * unused){
        LaunchApplication(new FreehandKeyboardTraining());
    },"Learn");
    DumpButton=new ButtonTextWidget(120,64,54,110,[&,this](void * unused){
        lLog("// === C DUMP: BEGIN ===\n");
        //lLog("symbol, trained times, learn rate, threshold, inputs count, weights blob\n")
        for(size_t c=0;c<FreehandKeyboardLetterTrainableSymbolsCount;c++) {
            char current=FreehandKeyboardLetterTrainableSymbols[c];
            if ( ' ' == current ) { current = '_'; } // changed to
            Perceptron * thaPerceptron = LoadPerceptronFromSymbol(current);
            if ( nullptr == thaPerceptron ) {
                lLog("// Ups! :( Unable to load symbol '%c'\n",FreehandKeyboardLetterTrainableSymbols[c]);
                continue;
            }
            lLog("const uint8_t perceptronBlob_%c[] = \"",current);
            for(size_t n=0;n<thaPerceptron->numInputs_;n++) {
                double entry=thaPerceptron->weights_[n];
                char * ptr;
                ptr=(char *)&entry;
                for(size_t od=0;od<sizeof(double);od++) {
                    lLog("\\%03o",ptr[od]);
                }
            }
            lLog("\";\n");
            lLog("Perceptron perceptron_%c = {%u,(double*)perceptronBlob_%c,%.20llf,%llf,%u};\n",current,thaPerceptron->numInputs_,
                                current,thaPerceptron->threshold_,
                                thaPerceptron->trainingRate_,thaPerceptron->trainedTimes);

            free(thaPerceptron);
        }
        lLog("// === C DUMP: END ===\n");
    },"Dump");
    #ifndef LUNOKIOT_DEBUG
    DumpButton->SetEnabled(false);
    #endif
    LoadButton=new ButtonTextWidget(5,64,54,110,[&,this](void * unused){
        lAppLog("Saving presets to NVS...\n");
        SavePerceptronWithSymbol('a',&perceptron_a);
        SavePerceptronWithSymbol('b',&perceptron_b);
        SavePerceptronWithSymbol('c',&perceptron_c);
        SavePerceptronWithSymbol('d',&perceptron_d);
        SavePerceptronWithSymbol('e',&perceptron_e);
        SavePerceptronWithSymbol('f',&perceptron_f);
        SavePerceptronWithSymbol('g',&perceptron_g);
        SavePerceptronWithSymbol('h',&perceptron_h);
        SavePerceptronWithSymbol('i',&perceptron_i);
        SavePerceptronWithSymbol('j',&perceptron_j);
        SavePerceptronWithSymbol('k',&perceptron_k);
        SavePerceptronWithSymbol('l',&perceptron_l);
        SavePerceptronWithSymbol('m',&perceptron_m);
        SavePerceptronWithSymbol('n',&perceptron_n);
        SavePerceptronWithSymbol('o',&perceptron_o);
        SavePerceptronWithSymbol('p',&perceptron_p);
        SavePerceptronWithSymbol('q',&perceptron_q);
        SavePerceptronWithSymbol('r',&perceptron_r);
        SavePerceptronWithSymbol('s',&perceptron_s);
        SavePerceptronWithSymbol('t',&perceptron_t);
        SavePerceptronWithSymbol('u',&perceptron_u);
        SavePerceptronWithSymbol('v',&perceptron_v);
        SavePerceptronWithSymbol('w',&perceptron_w);
        SavePerceptronWithSymbol('x',&perceptron_x);
        SavePerceptronWithSymbol('y',&perceptron_y);
        SavePerceptronWithSymbol('z',&perceptron_z);
        SavePerceptronWithSymbol(' ',&perceptron__);
    },"Pred");


    switchUseAltKeyb=new SwitchWidget(15,115,[&,this](void *unused){
        lAppLog("@TODO SWITCH VALUE: '%s'\n",(switchUseAltKeyb->switchEnabled?"true":"false"));
        //@TODO this option disable alternative keyboards (numeric, phone, credit-card...)
    });
    /*
    tryButton=new ButtonTextWidget(74,TFT_HEIGHT-59,54,75,[&,this](void * unused){
        LaunchApplication(new FreehandKeyboardTraining(false));
    },"Chk");
    */
    multiCheckButton=new ButtonTextWidget(TFT_WIDTH-80,TFT_HEIGHT-59,54,75,[&,this](void * unused){
        LaunchApplication(new FreehandKeyboardQuery());
    },"Qry");
    Tick();
}

Perceptron * FreeHandKeyboardSetupApplication::LoadPerceptronFromSymbol(char symbol) {
    //lAppLog("NVS: Load perceptron...\n");
    // get the key name
    char keyName[15];
    if ( ' ' == symbol ) { symbol = '_'; }
    sprintf(keyName,"freeHandSym_%c", symbol);

    // Load from NVS
    bool loaded = false; 
    size_t totalSize = NVS.getBlobSize(keyName);
    char *bufferdata = nullptr;
    if ( 0 == totalSize ) {
        //lAppLog("NVS: Unable to get perceptron from '%c'\n",symbol);
        return nullptr;
    }
    bufferdata = (char *)ps_malloc(totalSize); // create buffer "packed file"
    loaded = NVS.getBlob(keyName, (uint8_t*)bufferdata, totalSize);
    if ( false == loaded ) {
        //lAppLog("NVS: Unable to get '%c'\n",symbol);
        return nullptr;
    }
    Perceptron * currentSymbolPerceptron=(Perceptron *)bufferdata;
    currentSymbolPerceptron->weights_=(double*)(bufferdata+sizeof(Perceptron)); // correct the memory pointer
    //lAppLog("Perceptron loaded from NVS (trained: %u times)\n",currentSymbolPerceptron->trainedTimes);
    return currentSymbolPerceptron;
}

FreeHandKeyboardSetupApplication::~FreeHandKeyboardSetupApplication() {
    delete trainButton;
    delete DumpButton;
    delete btnEraseTrain;
    delete LoadButton;
    delete switchUseAltKeyb;
    //delete tryButton;
    delete multiCheckButton;
}

bool FreeHandKeyboardSetupApplication::Tick() {
    btnBack->Interact(touched,touchX,touchY); // this special button becomes from TemplateApplication
    trainButton->Interact(touched,touchX,touchY);
    DumpButton->Interact(touched,touchX,touchY);
    btnEraseTrain->Interact(touched,touchX,touchY);
    switchUseAltKeyb->Interact(touched,touchX,touchY);
    LoadButton->Interact(touched,touchX,touchY);
    //tryButton->Interact(touched,touchX,touchY);
    multiCheckButton->Interact(touched,touchX,touchY);

    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(ThCol(background)); // use theme colors
        btnEraseTrain->DrawTo(canvas);
        trainButton->DrawTo(canvas);
        DumpButton->DrawTo(canvas);
        //tryButton->DrawTo(canvas);
        multiCheckButton->DrawTo(canvas);
        LoadButton->DrawTo(canvas);
        // switch blob @TODO this approach can be a new SwitchWidgetText (UIControl)
        switchUseAltKeyb->DrawTo(canvas);
        int32_t x=switchUseAltKeyb->GetX()+switchUseAltKeyb->GetW()+14;
        int32_t y=switchUseAltKeyb->GetY()+30;
        canvas->setTextFont(0);
        canvas->setTextSize(2);
        canvas->setTextDatum(BL_DATUM);
        uint16_t textColor = ThCol(text);
        if ( false == switchUseAltKeyb->switchEnabled ) { textColor = ThCol(text_alt); }
        canvas->setTextColor(textColor);
        canvas->drawString("AI Only",x,y-2);
        canvas->setTextSize(1);
        canvas->setTextDatum(TL_DATUM);
        textColor = ThCol(text); // swap colors
        if ( true == switchUseAltKeyb->switchEnabled ) { textColor = ThCol(text_alt); }
        canvas->setTextColor(textColor);
        canvas->drawString("Specific control choice",x,y+2);

        btnBack->DrawTo(canvas); // redraw back button
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
