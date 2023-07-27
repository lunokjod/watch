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
#include "FreehandKeyboardQuery.hpp"
#include "FreehandKeyboardSetup.hpp"
#include "../system/Datasources/perceptron.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include <ArduinoNvs.h>

extern TTGOClass *ttgo;
#include "../static/img_trash_32.xbm"
extern bool UILongTapOverride;

FreehandKeyboardQuery::FreehandKeyboardQuery() {
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

    //symbolsTrained=(Perceptron*)ps_calloc(sizeof(TrainableSymbols),sizeof(Perceptron));
    lAppLog("Symbols trainables: %zu\n",FreehandKeyboardLetterTrainableSymbolsCount);
    perceptrons=(Perceptron **)ps_calloc(FreehandKeyboardLetterTrainableSymbolsCount,sizeof(Perceptron *));
    lAppLog("Loading all perceptrons...\n");
    for(size_t current=0;current<FreehandKeyboardLetterTrainableSymbolsCount;current++) {
        lAppLog("Loading symbol '%c'...\n",FreehandKeyboardLetterTrainableSymbols[current]);
        Perceptron * thaPerceptron = LoadPerceptronFromSymbol(FreehandKeyboardLetterTrainableSymbols[current]);
        perceptrons[current] = thaPerceptron;
    }
    bufferText=(char*)malloc(BufferTextMax);
    bufferText[0]=0; //EOL (empty string)
    RedrawMe();
    UILongTapOverride=true;
}

FreehandKeyboardQuery::~FreehandKeyboardQuery() {
    for(size_t current=0;current<FreehandKeyboardLetterTrainableSymbolsCount;current++) {
        if ( nullptr == perceptrons[current]) { continue; }
        RemovePerceptron(perceptrons[current]);
    }
    free(perceptrons);
    freeHandCanvas->deleteSprite();
    delete freeHandCanvas;
    perceptronCanvas->deleteSprite();
    delete perceptronCanvas;

    free(bufferText);
    UILongTapOverride=false;
}
Perceptron * FreehandKeyboardQuery::BuildBlankPerceptron() {
    lAppLog("Blank perceptron\n");
    // 0.2 of training rate
    void * ptrData = Perceptron_new(PerceptronMatrixSize*PerceptronMatrixSize, 0.2);
    Perceptron * current = (Perceptron *)ptrData;
    return current;
}
void FreehandKeyboardQuery::RemovePerceptron(Perceptron * item) {
    if ( nullptr != item ) {
        lAppLog("Free perceptron memory...\n");
        free(item);
        item=nullptr;
    }
}
Perceptron * FreehandKeyboardQuery::LoadPerceptronFromSymbol(char symbol) {
    lAppLog("NVS: Load perceptron...\n");
    // get the key name
    char keyName[15];
    if ( ' ' == symbol ) { symbol = '_'; } // changed to
    sprintf(keyName,"freeHandSym_%c", symbol);

    // Load from NVS
    bool loaded = false; 
    size_t totalSize = NVS.getBlobSize(keyName);
    char *bufferdata = nullptr;
    if ( 0 == totalSize ) {
        lAppLog("NVS: Unable to get perceptron from '%c'\n",symbol);
        return nullptr;
    }
    bufferdata = (char *)ps_malloc(totalSize); // create buffer "packed file"
    loaded = NVS.getBlob(keyName, (uint8_t*)bufferdata, totalSize);
    if ( false == loaded ) {
        lAppLog("NVS: Unable to get '%c'\n",symbol);
        return nullptr;
    }
    Perceptron * currentSymbolPerceptron=(Perceptron *)bufferdata;
    currentSymbolPerceptron->weights_=(double*)(bufferdata+sizeof(Perceptron)); // correct the memory pointer
    lAppLog("Perceptron loaded from NVS (trained: %u times)\n",currentSymbolPerceptron->trainedTimes);
    return currentSymbolPerceptron;
}

void FreehandKeyboardQuery::Cleanup() {
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

bool FreehandKeyboardQuery::Tick() {
    bool used=btnBack->Interact(touched,touchX,touchY); // this special button becomes from TemplateApplication
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
                    Cleanup();
                    return false;
                }


                const size_t MemNeeds =sizeof(double)*(PerceptronMatrixSize*PerceptronMatrixSize);
                lAppLog("Recognizer matrix (%zu byte):\n",MemNeeds);
                double *perceptronData=(double*)ps_malloc(MemNeeds); // alloc size for train the perceptrons
                memset(perceptronData,0,MemNeeds); // all 0

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
                        lLog("%c",(isData?'O':'.'));
                        pDataPtr++;
                    }
                    lLog("\n");
                }
                char seeChar = 0;
                for(size_t current=0;current<FreehandKeyboardLetterTrainableSymbolsCount;current++) {
                    Perceptron * thaPerceptron = perceptrons[current];
                    if (  nullptr == thaPerceptron ) { continue; }
                    int pResponse0 = Perceptron_getResult(thaPerceptron,perceptronData);
                    if ( 1 == pResponse0 ) {
                        lAppLog("Symbol: '%c' MATCH\n",FreehandKeyboardLetterTrainableSymbols[current]);
                        if ( 0 == seeChar ) {
                            seeChar=FreehandKeyboardLetterTrainableSymbols[current];
                            break; // stop search here
                        }
                    }
                }
                if ( 0 != seeChar ) {
                    bufferText[bufferTextOffset] = seeChar;
                    bufferText[bufferTextOffset+1]= 0; //EOL
                    bufferTextOffset++;
                    if ( bufferTextOffset >= BufferTextMax-1 ) { // 2 less due the \0
                        bufferTextOffset=0;
                        bufferText[bufferTextOffset]= 0; //EOL
                    }
                }
                seeChar=0;

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

void FreehandKeyboardQuery::RedrawMe() {
    canvas->fillSprite(ThCol(background)); // use theme colors
    const int32_t BORDER=20;
    canvas->fillRoundRect(BORDER,30,
    canvas->width()-(BORDER*2),
    canvas->height()-90,
    BORDER/2,ThCol(background_alt));

    canvas->drawFastHLine(30,50,canvas->width()-60,ThCol(shadow));
    canvas->drawFastHLine(30,140,canvas->width()-60,ThCol(light));
    canvas->drawFastHLine(30,155,canvas->width()-60,ThCol(light));

    if ( (false == touched) || ( 0 == touchDragDistance )) {
        btnBack->DrawTo(canvas); // redraw back button
    }
    canvas->setFreeFont(&FreeMonoBold9pt7b);
    canvas->setTextDatum(TR_DATUM);
    canvas->setTextColor(ThCol(text));
    canvas->drawString(bufferText,TFT_WIDTH-10,5);


    freeHandCanvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
}
