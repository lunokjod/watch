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
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "FreehandKeyboardLetterChoice.hpp"
#include "FreehandKeyboardSetup.hpp"
#include "../system/Datasources/perceptron.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include <ArduinoNvs.h>

extern TTGOClass *ttgo;
#include "../static/img_trash_32.xbm"
extern bool UILongTapOverride;

void FreehandKeyboardLetterChoiceApplication::SavePerceptronWithSymbol(char symbol,Perceptron *item) {
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

Perceptron * FreehandKeyboardLetterChoiceApplication::LoadPerceptronFromSymbol(char symbol) {
    lAppLog("NVS: Load perceptron...\n");
    if ( ' ' == symbol ) { symbol = '_'; }
    // get the key name
    char keyName[15];
    sprintf(keyName,"freeHandSym_%c", symbol);

    // Load from NVS
    bool loaded = false; 
    size_t totalSize = NVS.getBlobSize(keyName);
    char *bufferdata = nullptr;
    if ( 0 == totalSize ) {
        //lAppLog("NVS: Unable to get perceptron from '%c'\n",symbol);
        return BuildBlankPerceptron();
    }
    bufferdata = (char *)ps_malloc(totalSize); // create buffer "packed file"
    loaded = NVS.getBlob(keyName, (uint8_t*)bufferdata, totalSize);
    if ( false == loaded ) {
        //lAppLog("NVS: Unable to get '%c'\n",symbol);
        return BuildBlankPerceptron();
    }
    Perceptron * pfromNVS=(Perceptron *)ps_malloc(sizeof(Perceptron));
    memcpy(pfromNVS,bufferdata,sizeof(Perceptron));
    pfromNVS->weights_=(double*)ps_calloc(pfromNVS->numInputs_,sizeof(double));
    memcpy(pfromNVS->weights_,bufferdata+sizeof(Perceptron),sizeof(double)*pfromNVS->numInputs_);
    lAppLog("Perceptron loaded from NVS (trained: %u times)\n",pfromNVS->trainedTimes);
    free(bufferdata);
    return pfromNVS;
}
FreehandKeyboardLetterChoiceApplication::FreehandKeyboardLetterChoiceApplication(bool training): trainingMode(training) {
    // rewrite back button
    btnBack->tapActivityCallback = [](void *unused) {
        LaunchApplication(new FreeHandKeyboardSetupApplication());
    };
    freeHandCanvas = new TFT_eSprite(tft);
    freeHandCanvas->setColorDepth(16);
    freeHandCanvas->createSprite(canvas->width(),canvas->height());
    freeHandCanvas->fillSprite(Drawable::MASK_COLOR);

    perceptronCanvas = new TFT_eSprite(tft);
    perceptronCanvas->setColorDepth(1);
    perceptronCanvas->createSprite(PerceptronMatrixSize,PerceptronMatrixSize);
    perceptronCanvas->fillSprite(0);

    destroySymbolData = new ButtonImageXBMWidget(85,TFT_HEIGHT-69,64,64,[&,this](void *bah){
        RemovePerceptron(perceptrons[currentSymbolOffset]);
        perceptrons[currentSymbolOffset]=BuildBlankPerceptron();
        RedrawMe();
    },img_trash_32_bits,img_trash_32_height,img_trash_32_width,ThCol(text),ThCol(high),false);
    destroySymbolData->SetEnabled(false);

    lAppLog("Symbols trainables: %zu\n",FreehandKeyboardLetterTrainableSymbolsCount);
    perceptrons=(Perceptron **)ps_calloc(FreehandKeyboardLetterTrainableSymbolsCount,sizeof(Perceptron *));

    lAppLog("Loading all perceptrons...\n");
    for(size_t current=0;current<FreehandKeyboardLetterTrainableSymbolsCount;current++) {
        lAppLog("Loading symbol '%c'...\n",FreehandKeyboardLetterTrainableSymbols[current]);
        perceptrons[current] = LoadPerceptronFromSymbol(FreehandKeyboardLetterTrainableSymbols[current]);
    }
    // first election random
    currentSymbolOffset=random(0,FreehandKeyboardLetterTrainableSymbolsCount);
    currentSymbol=FreehandKeyboardLetterTrainableSymbols[currentSymbolOffset];
    lAppLog("Pick one random: %lu '%c'\n",currentSymbolOffset,currentSymbol);
    /*
    currentSymbolOffset=0;
    if ( currentSymbolOffset > FreehandKeyboardLetterTrainableSymbolsCount-1 ) { currentSymbolOffset = 0; }
    currentSymbol=FreehandKeyboardLetterTrainableSymbols[currentSymbolOffset];
    lAppLog("Next: %lu '%c'\n",currentSymbolOffset,currentSymbol);
    */
    RedrawMe();
    UILongTapOverride=true;
    lAppLog("Begin!\n");
}
void FreehandKeyboardLetterChoiceApplication::RemovePerceptron(Perceptron * item) {
    if ( nullptr != item ) {
        free(item);
        item=nullptr;
    }
}

FreehandKeyboardLetterChoiceApplication::~FreehandKeyboardLetterChoiceApplication() {
    freeHandCanvas->deleteSprite();
    delete freeHandCanvas;
    perceptronCanvas->deleteSprite();
    delete perceptronCanvas;

    delete destroySymbolData;

    for(size_t current=0;current<FreehandKeyboardLetterTrainableSymbolsCount;current++) {
        if ( nullptr == perceptrons[current]) { continue; }
        SavePerceptronWithSymbol(FreehandKeyboardLetterTrainableSymbols[current],perceptrons[current]);
        RemovePerceptron(perceptrons[current]);
    }
    free(perceptrons);

   // RemovePerceptron();
    UILongTapOverride=false;
}
Perceptron * FreehandKeyboardLetterChoiceApplication::BuildBlankPerceptron() {
    lAppLog("Blank perceptron\n");
    // 0.2 of training rate
    return Perceptron_new(PerceptronMatrixSize*PerceptronMatrixSize, 0.2);
}

void FreehandKeyboardLetterChoiceApplication::Cleanup() {
    // cleanup
    freeHandCanvas->fillSprite(Drawable::MASK_COLOR);
    perceptronCanvas->fillSprite(0);
    triggered=false;
    oneSecond=0;

    boxX=TFT_WIDTH; // restore default values
    boxY=TFT_HEIGHT;
    boxH=0;
    boxW=0;
}

bool FreehandKeyboardLetterChoiceApplication::Tick() {
    bool used=btnBack->Interact(touched,touchX,touchY); // this special button becomes from TemplateApplication
    if ( used ) {
        RedrawMe();
        return true;
    }
    used=destroySymbolData->Interact(touched,touchX,touchY);
    if ( used ) {
        RedrawMe();
        return true;
    }

    const int32_t RADIUS=14;
    if ( touched ) {
        // draw on freehand canvas
        freeHandCanvas->fillCircle(touchX,touchY,RADIUS, ThCol(text)); // to the buffer (ram operation is fast)
        tft->fillCircle(touchX,touchY,RADIUS, ThCol(text)); // hardware direct
        const float ratioFromScreen = canvas->width()/PerceptronMatrixSize; 
        perceptronCanvas->drawPixel(touchX/ratioFromScreen,touchY/ratioFromScreen,1);
        if ( touchX < boxX ) { boxX = touchX; }
        if ( touchY < boxY ) { boxY = touchY; }
        if ( touchX > boxW ) { boxW = touchX; }
        if ( touchY > boxH ) { boxH = touchY; }
        triggered=true;
        oneSecond=0;
        return false; // fake no changes (already draw on tft)
    } else {
        if ( triggered ) {
            if ( 0 == oneSecond ) {
                boxX-=RADIUS; // zoom out to get whole draw
                boxY-=RADIUS;
                boxH+=RADIUS*2;
                boxW+=RADIUS*2;
                // don't get out of margins
                if ( boxX < 0 ) { boxX=0; }
                if ( boxY < 0 ) { boxY=0; }
                if ( boxH > canvas->height() ) { boxH=canvas->height(); }
                if ( boxW > canvas->width() ) { boxW=canvas->width(); }

                lAppLog("RECT: %d %d %d %d\n",boxX,boxY,boxH-boxY,boxW-boxX);
                canvas->drawRect(boxX-1,boxY-1,(boxW-boxX)+2,(boxH-boxY)+2,TFT_RED); // mark the area to recognize
                canvas->drawRect(boxX,boxY,boxW-boxX,boxH-boxY,TFT_RED);             // mark the area to recognize

                oneSecond=millis()+800; // grace time, maybe the user need to add something before send to the perceptron
                return true;
            } else if ( millis() > oneSecond ) {
                bool valid=false;
                if ( ( boxW-boxX > MINIMALDRAWSIZE ) || ( boxH-boxY > MINIMALDRAWSIZE)) { valid=true; }
                if ( false == valid ) {
                    lAppLog("Discarded (too small draw<%dpx)\n",MINIMALDRAWSIZE);
                    tft->fillCircle(TFT_WIDTH/2,TFT_HEIGHT/2,100,TFT_RED);
                    Cleanup();
                    return false;
                }


                const size_t MemNeeds =sizeof(double)*(PerceptronMatrixSize*PerceptronMatrixSize);
                lAppLog("Recognizer matrix (%zu byte):\n",MemNeeds);
                double *perceptronData=(double*)ps_malloc(MemNeeds); // alloc size for train the perceptrons
                memset(perceptronData,0,MemNeeds); // all 0
                char suggestedSymbol=0;
                size_t suggestedOffset=0;

                double *pDataPtr;
                pDataPtr=perceptronData;
                for (int16_t y=0;y<PerceptronMatrixSize;y++) {
                    pDataPtr=perceptronData+(PerceptronMatrixSize*y);

                    for (int16_t x=0;x<perceptronCanvas->width();x++) {
                        uint32_t color = perceptronCanvas->readPixel(x,y);

                        // 0 nothing 1 draw
                        bool isData= false;
                        if ( TFT_WHITE == color ){ // write only 1 (memset already put 0 on all)
                            *pDataPtr=1;
                            isData=true;
                        }
                        lLog("%c",(isData?'O':' '));
                        pDataPtr++;
                    }
                    lLog("\n");
                }
                if ( nullptr == perceptrons[currentSymbolOffset] ) {
                    perceptrons[currentSymbolOffset] = BuildBlankPerceptron();
                }

                int pResponse0 = Perceptron_getResult(perceptrons[currentSymbolOffset],perceptronData);
                lAppLog("Perceptron %p (%u:'%c') response (BEFORE TRAINING): %d (trained: %u times)\n",perceptrons[currentSymbolOffset], currentSymbolOffset, currentSymbol, pResponse0,perceptrons[currentSymbolOffset]->trainedTimes);
                bool isGood=false;
                bool needTraining=false;
                if ( trainingMode ) {
                    // Announche if match
                    if ( 1 == pResponse0 ) {
                        //if identified as TRUE instead of not in trainNotSymbol, must be trained again
                        if ( trainNotSymbol ) {
                            needTraining=true;
                            tft->fillScreen(TFT_RED);
                        } else {
                            tft->fillScreen(TFT_GREEN);
                            isGood=true;
                            needTraining=true; // TEST
                        }
                    } else {
                        if ( trainNotSymbol ) {
                            tft->fillScreen(TFT_GREEN);
                            isGood=true;
                        } else {
                            needTraining=true;
                            tft->fillScreen(TFT_RED);
                        }
                    }
                    if ( needTraining ) {
                        lAppLog("Perceptron %p %s '%c' training...\n", perceptrons[currentSymbolOffset], (trainNotSymbol?"learn NOT":"LEARN"),currentSymbol);
                        //train the perceptron here
                        for (size_t c=0;c<FreehandKeyboardLetterTrainableSymbolsCount;c++) {
                            if ( c == currentSymbolOffset ) { //is my letter
                                // train TRUE;
                                int desiredResult=1;
                                if ( trainNotSymbol ) { desiredResult=0; }
                                int8_t repeat=PerceptronIterations;
                                while (repeat > 0 ) {
                                    Perceptron_train(perceptrons[currentSymbolOffset], perceptronData, desiredResult);
                                    repeat--;
                                }
                            } else {
                                // train FALSE;
                                if ( trainNotSymbol ) { continue; } // unusable (can fit to any)
                                if ( nullptr != perceptrons[c] ) {
                                    int pResponse1 = Perceptron_getResult(perceptrons[c], perceptronData);
                                    if ( 1 == pResponse1 ) { // only one "not"
                                        lAppLog("Perceptron %p '%c' matches also :(\n", perceptrons[c], FreehandKeyboardLetterTrainableSymbols[c]);
                                        //int8_t repeat=PerceptronIterations/2;
                                        //while (repeat > 0 ) {
                                        Perceptron_train(perceptrons[currentSymbolOffset], perceptronData, 0);
                                        //    repeat--;
                                        //}
                                        suggestedSymbol=FreehandKeyboardLetterTrainableSymbols[c];
                                        suggestedOffset=c;

                                    }
                                }
                            }
                        }
                    } /*else { // don't need training
                        for (size_t c=0;c<FreehandKeyboardLetterTrainableSymbolsCount;c++) {
                            if ( c != currentSymbolOffset ) {
                            }
                        }
                    }*/
                }
                free(perceptronData);

                if ( isGood ) {
                    if ( 0 == suggestedSymbol ) { // pick random next letter to train
                        currentSymbolOffset=random(0,FreehandKeyboardLetterTrainableSymbolsCount);
                        currentSymbol=FreehandKeyboardLetterTrainableSymbols[currentSymbolOffset];
                    } else { // try to re-check the matched symbols
                        currentSymbolOffset = suggestedOffset;
                        currentSymbol=suggestedSymbol;
                    }
                    /*
                    // pick less training number (loops when training saturates)
                    size_t lessTrainedOffset=0;
                    int32_t lessTrainedValue=-1;
                    for (size_t c=0;c<FreehandKeyboardLetterTrainableSymbolsCount;c++) {
                        if (lessTrainedValue < perceptrons[c]->trainedTimes) { continue; }
                        lessTrainedOffset=c;
                    }
                    currentSymbolOffset=lessTrainedOffset;
                    currentSymbol=FreehandKeyboardLetterTrainableSymbols[currentSymbolOffset];
                    lAppLog("Choosing less trained symbol '%c' with: %u rounds\n",currentSymbol,perceptrons[currentSymbolOffset]->trainedTimes);
                    */
                    /*
                    // pick sequential
                    currentSymbolOffset++;
                    if ( currentSymbolOffset > FreehandKeyboardLetterTrainableSymbolsCount-1 ) { currentSymbolOffset = 0; }
                    currentSymbol=FreehandKeyboardLetterTrainableSymbols[currentSymbolOffset];
                    lAppLog("Next: %lu '%c'\n",currentSymbolOffset,currentSymbol);
                    /*
                    /*
                    if ( random(0,100) > 50 ) {
                        trainNotSymbol=(!trainNotSymbol); // flip
                    }*/
                }
                Cleanup();
            }
        }
    }

    if ( millis() > nextRefresh ) { // redraw full canvas
        RedrawMe();
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}

void FreehandKeyboardLetterChoiceApplication::RedrawMe() {
    canvas->fillSprite(ThCol(background)); // use theme colors
    const int32_t BORDER=20;
    canvas->fillRoundRect(BORDER,30,
    canvas->width()-(BORDER*2),
    canvas->height()-90,
    BORDER/2,ThCol(background_alt));

    canvas->drawFastHLine(30,50,canvas->width()-60,ThCol(shadow));
    canvas->drawFastHLine(30,140,canvas->width()-60,ThCol(light));
    canvas->drawFastHLine(30,155,canvas->width()-60,ThCol(light));

    if ( nullptr != perceptrons[currentSymbolOffset] ) {
        canvas->setFreeFont(&FreeMonoBold24pt7b);
        canvas->setTextDatum(BC_DATUM);
        canvas->setTextColor(TFT_DARKGREY);
        char buffer[20];
        canvas->setTextSize(1);
        if ( trainNotSymbol ) {
            canvas->drawString("NOT", canvas->width()/2,60);
        } else {
            canvas->drawString("DRAW", canvas->width()/2,60);
        }
        canvas->setTextSize(2);
        sprintf(buffer,"'%c'", currentSymbol);
        canvas->drawString(buffer, canvas->width()/2,145);

        canvas->setFreeFont(&FreeMonoBold12pt7b);
        canvas->setTextDatum(BR_DATUM);
        canvas->setTextSize(1);
        sprintf(buffer,"T: %d", perceptrons[currentSymbolOffset]->trainedTimes);
        canvas->drawString(buffer, canvas->width()-5,canvas->height()-5);

    }
    if ( (false == touched) || ( 0 == touchDragDistance )) {
        btnBack->DrawTo(canvas); // redraw back button
        destroySymbolData->DrawTo(canvas);
    }
    freeHandCanvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
}
