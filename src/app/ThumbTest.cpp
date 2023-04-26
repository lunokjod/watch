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
#include "ThumbTest.hpp"
#include "../UI/widgets/CanvasZWidget.hpp"
#include "../system/Datasources/perceptron.hpp"
extern TTGOClass *ttgo;

const double TRAINING_RATE = 0.2;
const int TRAINING_ITERATIONS = 16;
extern bool UILongTapOverride;

ThumbTest::ThumbTest() {
    learnerP0 = (Perceptron*)Perceptron_new(9*9, TRAINING_RATE);

    RedrawMe();
    UILongTapOverride=true;
}

ThumbTest::~ThumbTest() {
    free(learnerP0);
    UILongTapOverride=false;
}
void ThumbTest::RedrawMe() {
    // redraw interface
    if ( 15 < learnerP0->trainedTimes ) { learn=2; } // lock in check if are enought trained
    else { learn++; }
    if ( learn > 2 ) { learn=0; }

    if ( 0 == learn ) { // LEARN MODE
        /*
        size_t usedSpace=0;
        char * perceptronData=PerceptronSnapshoot(learnerP,usedSpace);
        lAppLog("Perceptron dump on: %p size: %u\n", perceptronData,usedSpace);
        double *testPtr;
        testPtr=(double *)perceptronData;
        for(size_t off=0;off<learnerP->numInputs_;off++){
    		lLog("Weight off: %u Value: %lf\n", off, testPtr[off]);
        }
        free(perceptronData);
        */
        lAppLog("LEARN MODE\n"); // YES MODE
        canvas->fillSprite(TFT_GREEN);
    } else if ( 1 == learn ) { // NO MODE
        lAppLog("NOT VALID MODE\n");
        canvas->fillSprite(TFT_RED);
    } else if ( 2 == learn ) { // CHECK MODE
        lAppLog("CHECK MODE\n");
        canvas->fillSprite(TFT_BLUE);
    }


    boxX=TFT_WIDTH; // restore default values
    boxY=TFT_HEIGHT;
    boxH=0;
    boxW=0;
}
bool ThumbTest::Tick() {
    const int32_t RADIUS=18;
    if ( touched ) {
        canvas->fillCircle(touchX,touchY,RADIUS, ThCol(text));
        if ( touchX < boxX ) { boxX = touchX; }
        if ( touchY < boxY ) { boxY = touchY; }
        if ( touchX > boxW ) { boxW = touchX; }
        if ( touchY > boxH ) { boxH = touchY; }
        triggered=true;
        oneSecond=0;
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
                canvas->drawRect(boxX,boxY,boxW-boxX,boxH-boxY,TFT_RED); // mark the area to recognize
                oneSecond=millis()+800;
                return true;
            } else if ( millis() > oneSecond ) {
                triggered=false;
                oneSecond=0;
                if ( ( 1 > (boxW-boxX) )||( 1 > (boxH-boxY) ) ) {
                    lAppLog("Discarded image (small)\n");
                    RedrawMe();
                    return true;
                }
                // crop the data and simplify colors
                TFT_eSprite * cropImage = new TFT_eSprite(tft);
                if ( nullptr == cropImage ) {
                    lAppLog("ERROR: Unable to crop the image!\n");
                    RedrawMe();
                    return true;
                }
                cropImage->setColorDepth(16);
                void *cropDone = cropImage->createSprite(boxW-boxX,boxH-boxY);
                if ( nullptr == cropDone ) {
                    lAppLog("ERROR: Unable to create sprite crop image!\n");
                    delete cropImage;
                    RedrawMe();
                    return true;
                }

                for (int16_t y=boxY;y<boxH;y++) {
                    for (int16_t x=boxX;x<boxW;x++) {
                        uint32_t color = canvas->readPixel(x,y);
                        // simplify colors
                        if ( ThCol(text) == color ){ color=TFT_WHITE; }
                        else { color=TFT_BLACK; }
                        cropImage->drawPixel(x-boxX,y-boxY,color);
                    }
                }
                // scale the image
                TFT_eSprite * scaledImage = ScaleSprite(cropImage,0.04);
                if (( scaledImage->width() < 2 )||( scaledImage->height() < 2 )) {
                    // destroy the crop
                    cropImage->deleteSprite();
                    delete cropImage;
                    scaledImage->deleteSprite();
                    delete scaledImage;
                    lAppLog("Discarded image (too small after scale)\n");
                    RedrawMe();
                    return true;
                }
                // destroy the crop version
                cropImage->deleteSprite();
                delete cropImage; // dont use again!!!

                const size_t MemNeeds =sizeof(double)*(9*9);
                lAppLog("Recognizer matrix (%u byte):\n",MemNeeds);
                double *perceptronData=(double*)ps_malloc(MemNeeds); // alloc size for train the perceptrons
                memset(perceptronData,0,MemNeeds); // all 0
                double *pDataPtr;
                pDataPtr=perceptronData;
                for (int16_t y=0;y<scaledImage->height();y++) {
                    pDataPtr=perceptronData+(9*y);

                    for (int16_t x=0;x<scaledImage->width();x++) {
                        uint32_t color = scaledImage->readPixel(x,y);

                        // simplify colors
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
                // clean!
                scaledImage->deleteSprite();
                delete scaledImage;

                lAppLog("Converted: ");
                for(int c=0;c<9*9;c++) {
                        lLog("%c",(perceptronData[c]?'1':'0'));
                }
                lLog("\n");
                // try to get response from perceptron

                int pResponse0 = Perceptron_getResult(learnerP0,perceptronData);
                lAppLog("Perceptron0 %p response: %d\n",learnerP0,pResponse0);


                if ( learn < 2 ) { // learn about yes or NO
                    

                    for (int i = 0 ; i < TRAINING_ITERATIONS ; i++) {
                        lAppLog("Perceptron0 %p iteration %d\n", learnerP0,i);
                        Perceptron_train(learnerP0, perceptronData, learn);

                    }
                    pResponse0 = Perceptron_getResult(learnerP0,perceptronData);
                    lAppLog("Perceptron0 %p response: %d\n",learnerP0,pResponse0);

                } else if ( 2 == learn ) {
                    canvas->setTextFont(0);
                    canvas->setTextColor(TFT_WHITE);

                    canvas->fillSprite(ThCol(background));
                    canvas->setTextDatum(CC_DATUM);

                    canvas->setTextSize(3);

                    if ( 0 == pResponse0 ) {
                        canvas->drawString("MATCH",canvas->width()/2, canvas->height()/2);
                    } else if ( 1 == pResponse0 ) {
                        canvas->drawString("UNKNOWN",canvas->width()/2, canvas->height()/2);
                    }

                    canvas->pushSprite(0,0);
                    delay(1200);
                }

                //cleanup
                free(perceptronData);

                RedrawMe();
                return true;
            }
        }
    }

    if ( millis() > nextRefresh ) { // redraw full canvas        
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
