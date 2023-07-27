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
#include "../lunokiot_config.hpp"
#include "NuKeyboard.hpp"
#include "LogView.hpp"

NuKeyboardApplication::NuKeyboardApplication() {
    handDrawCanvas= new CanvasZWidget(canvas->height(),canvas->width(),1.0,1);
    handDrawCanvas->canvas->fillSprite(TFT_BLACK);

    for (int c=0;c<FrontLayer;c++) {
        frontLayer[c] = Perceptron_new(PerceptronMatrixSize*PerceptronMatrixSize, 0.3);
    }
    for (int c=0;c<InternalLayer;c++) {
        hiddenLayer[c] = Perceptron_new(FrontLayer, 0.3);
    }
    for (int c=0;c<OutLayer;c++) {
        outLayer[c] = Perceptron_new(InternalLayer, 0.3);
    }
}

void NuKeyboardApplication::Train() {
    TFT_eSprite * ScaledImg = handDrawCanvas->GetScaledImage(ScaleImage);
    lAppLog("Matrix size: %ux%u\n", ScaledImg->width(), ScaledImg->height());
    for(int y=0;y<ScaledImg->height();y++) {
        for(int x=0;x<ScaledImg->width();x++) {
            uint16_t color = ScaledImg->readPixel(x,y);
            if ( TFT_WHITE == color ) { lLog("X"); }
            else { lLog(" "); }
        }
        lLog("\n");
    }
    ScaledImg->deleteSprite();
    delete ScaledImg;
    /*
    for (int a=0;a<FrontLayer;a++) {
        //frontLayer[a] = Perceptron_new(PerceptronMatrixSize*PerceptronMatrixSize, 0.3);
        for (int b=0;b<InternalLayer;b++) {
            //hiddenLayer[b] = Perceptron_new(FrontLayer, 0.3);
            for (int c=0;c<OutLayer;c++) {
                //outLayer[c] = Perceptron_new(InternalLayer, 0.3);
            }
        }
    }*/
}

void NuKeyboardApplication::Query() {

}

NuKeyboardApplication::~NuKeyboardApplication() {
    if ( nullptr != handDrawCanvas ) { delete handDrawCanvas; }
    for (int c=0;c<FrontLayer;c++) { free(frontLayer[c]); }
    for (int c=0;c<InternalLayer;c++) { free(hiddenLayer[c]); }
    for (int c=0;c<OutLayer;c++) { free(outLayer[c]); }
}

bool NuKeyboardApplication::Tick() {
    btnBack->Interact(touched,touchX,touchY); // this special button becomes from TemplateApplication
    if ( touched ) {
        tft->fillCircle(touchX,touchY,TouchRadius,TFT_WHITE);
        handDrawCanvas->canvas->fillCircle(touchX,touchY,TouchRadius,TFT_WHITE);
        nextCleanup=millis()+1000;

    } else {
        if ( ( -1 != nextCleanup ) && ( millis() > nextCleanup )) { // redraw full canvas
            Train();
            handDrawCanvas->canvas->fillSprite(TFT_BLACK);
            nextCleanup=-1;
        }
        if ( millis() > nextRefresh ) { // redraw full canvas
            canvas->fillSprite(ThCol(background));
            handDrawCanvas->DrawTo(canvas);
            //if ( false == touched ) { handDrawCanvas->canvas->fillSprite(TFT_BLACK); }
            btnBack->DrawTo(canvas); // redraw back button
            nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
            return true;
        }
    }
    return false;
}
