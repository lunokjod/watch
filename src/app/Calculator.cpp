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

#include "Calculator.hpp"
#include <Arduino.h>
#include <LilyGoWatch.h>

#include "../UI/UI.hpp"
#include "../static/img_happy_48.xbm" // center smiley to see the orientation
#include <ArduinoNvs.h> // persistent values

#include "LogView.hpp"

#include "../UI/widgets/ButtonWidget.hpp"
#include "../UI/widgets/ButtonTextWidget.hpp"

// better save only on exit to save flash writes
CalculatorApplication::~CalculatorApplication() {
    delete seven;
    delete eight;
    delete nine;
    delete four;
    delete five;
    delete six;
    delete one;
    delete two;
    delete three;
    delete zero;
    delete dotBtn;
    delete divBtn;
    delete mulBtn;
    delete lessBtn;
    delete addBtn;
    delete resetBtn;
    delete correctionBtn;
    delete resultBtn;
}

CalculatorApplication::CalculatorApplication() {
    const int16_t ButtonsOffset = 45;
    const int16_t ButtonsSize = 48;
    UICallback numberButtonCallback = [&](void *payload) {
        ButtonTextWidget * pushedBtn = (ButtonTextWidget *)payload;
        //lLog("This: %p Button: %p -> Label: %p\n",this,payload,pushedBtn->label);
        if ( strlen(displayText) > 30) {
            sprintf(displayText,errMsg);
            return;
        }
        char lastdisplayText[32];
        strcpy(lastdisplayText,displayText);
        sprintf(displayText,"%s%s",lastdisplayText,pushedBtn->label);
    };

    seven = new ButtonTextWidget(0,ButtonsOffset,ButtonsSize,ButtonsSize,numberButtonCallback,"7");
    seven->paramCallback=seven;
    eight = new ButtonTextWidget(ButtonsSize,ButtonsOffset,ButtonsSize,ButtonsSize,numberButtonCallback,"8");
    eight->paramCallback=eight;
    nine = new ButtonTextWidget(ButtonsSize*2,ButtonsOffset,ButtonsSize,ButtonsSize,numberButtonCallback,"9");
    nine->paramCallback=nine;

    four = new ButtonTextWidget(0,ButtonsOffset+(ButtonsSize),ButtonsSize,ButtonsSize,numberButtonCallback,"4");
    four->paramCallback=four;
    five = new ButtonTextWidget(ButtonsSize,ButtonsOffset+(ButtonsSize),ButtonsSize,ButtonsSize,numberButtonCallback,"5");
    five->paramCallback=five;
    six = new ButtonTextWidget(ButtonsSize*2,ButtonsOffset+(ButtonsSize),ButtonsSize,ButtonsSize,numberButtonCallback,"6");
    six->paramCallback=six;

    one = new ButtonTextWidget(0,ButtonsOffset+(ButtonsSize*2),ButtonsSize,ButtonsSize,numberButtonCallback,"1");
    one->paramCallback=one;
    two = new ButtonTextWidget(ButtonsSize,ButtonsOffset+(ButtonsSize*2),ButtonsSize,ButtonsSize,numberButtonCallback,"2");
    two->paramCallback=two;
    three = new ButtonTextWidget(ButtonsSize*2,ButtonsOffset+(ButtonsSize*2),ButtonsSize,ButtonsSize,numberButtonCallback,"3");
    three->paramCallback=three;

    zero = new ButtonTextWidget(0,ButtonsOffset+(ButtonsSize*3),ButtonsSize,ButtonsSize*2,numberButtonCallback,"0");
    zero->paramCallback=zero;

    dotBtn = new ButtonTextWidget(ButtonsSize*2,ButtonsOffset+(ButtonsSize*3),ButtonsSize,ButtonsSize,[this](void * unused) {
        if ( containDecimalNumber ) {
            return;
        }
        char lastdisplayText[32];
        strcpy(lastdisplayText,displayText);
        sprintf(displayText,"%s.",lastdisplayText);
        containDecimalNumber=true;
    },".",ThCol(text),ThCol(background_alt));
    
    divBtn = new ButtonTextWidget(ButtonsSize*3,ButtonsOffset,ButtonsSize,ButtonsSize,[this](void *unused) {
        lastOp='/';
        if ( false == firstValueProvided ) {
            firstValue = strtof(displayText,nullptr);
            firstValueProvided=true;
            sprintf(displayText,"0");
        } else {
            resultBtn->Trigger();
            sprintf(displayText,"0");
        }
    },"/",TFT_BLACK,ThCol(highlight));

    mulBtn = new ButtonTextWidget(ButtonsSize*3,ButtonsOffset+(ButtonsSize),ButtonsSize,ButtonsSize,[this](void *unused) {
        lastOp='*';
        if ( false == firstValueProvided ) {
            firstValue = strtof(displayText,nullptr);
            firstValueProvided=true;
            sprintf(displayText,"0");
        } else {
            resultBtn->Trigger();
            sprintf(displayText,"0");
        }
    },"X",TFT_BLACK,ThCol(highlight));

    lessBtn = new ButtonTextWidget(ButtonsSize*3,ButtonsOffset+(ButtonsSize*2),ButtonsSize,ButtonsSize,[this](void *unused) {
        lastOp='-';
        if ( false == firstValueProvided ) {
            firstValue = strtof(displayText,nullptr);
            firstValueProvided=true;
            sprintf(displayText,"0");
        } else {
            resultBtn->Trigger();
            sprintf(displayText,"0");
        }
    },"-",TFT_BLACK,ThCol(highlight));

    addBtn = new ButtonTextWidget(ButtonsSize*3,ButtonsOffset+(ButtonsSize*3),ButtonsSize,ButtonsSize,[this](void *unused) {
        lastOp='+';
        if ( false == firstValueProvided ) {
            firstValue = strtof(displayText,nullptr);
            firstValueProvided=true;
            sprintf(displayText,"0");
        } else {
            resultBtn->Trigger();
            sprintf(displayText,"0");
        }
    },"+",TFT_BLACK,ThCol(highlight));


    resetBtn = new ButtonTextWidget(ButtonsSize*4,ButtonsOffset,ButtonsSize,ButtonsSize,[this](void *unused) {
        sprintf(displayText,"0");
        containDecimalNumber=false;
        firstValueProvided=false;
        firstValue=0.0;
        resultValue=0.0;
        lastOp=0;
    },"C",TFT_BLACK,ThCol(light));

    correctionBtn = new ButtonTextWidget(ButtonsSize*4,ButtonsOffset+(ButtonsSize),ButtonsSize,ButtonsSize,[this](void *unused) {
        size_t currLen = strlen(displayText);
        if ( 0 == currLen ) {
            sprintf(displayText,"0");
        } else {
            displayText[currLen-1]=0;
        }
    },"<",TFT_BLACK,ThCol(light));


    resultBtn = new ButtonTextWidget(ButtonsSize*4,ButtonsOffset+(ButtonsSize*2),ButtonsSize*2,ButtonsSize,[this](void *bah) {
        if ( false == firstValueProvided ) { return; }
        float checkNonZero = strtof(displayText,nullptr);
        if ( 0.0 == checkNonZero ) {
            resetBtn->Trigger();
            sprintf(displayText,errMsg);
            return;
        }
        if ( 0 == lastOp ) {
            // no operation
            return;
        } else if ( '/' == lastOp ) {
            resultValue = firstValue/checkNonZero;
        } else if ( '*' == lastOp ) {
            resultValue = firstValue*checkNonZero;
        } else if ( '+' == lastOp ) {
            resultValue = firstValue+checkNonZero;
        } else if ( '-' == lastOp ) {
            resultValue = firstValue-checkNonZero;
        }
        lAppLog("resultValue: %f\n",resultValue);
        sprintf(displayText,"%.3f",resultValue);
        lastOp=0;
        firstValue=0.0;
        firstValueProvided=false;
    },"=",TFT_BLACK,ThCol(light));
    
    canvas->setTextFont(0);
    canvas->setTextSize(3);
    
    sprintf(displayText,"0");
    Tick();
}

bool CalculatorApplication::Tick() {
    seven->Interact(touched,touchX,touchY);
    eight->Interact(touched,touchX,touchY);
    nine->Interact(touched,touchX,touchY);
    four->Interact(touched,touchX,touchY);
    five->Interact(touched,touchX,touchY);
    six->Interact(touched,touchX,touchY);
    one->Interact(touched,touchX,touchY);
    two->Interact(touched,touchX,touchY);
    three->Interact(touched,touchX,touchY);
    zero->Interact(touched,touchX,touchY);
    dotBtn->Interact(touched,touchX,touchY);
    divBtn->Interact(touched,touchX,touchY);
    mulBtn->Interact(touched,touchX,touchY);
    lessBtn->Interact(touched,touchX,touchY);
    addBtn->Interact(touched,touchX,touchY);
    resetBtn->Interact(touched,touchX,touchY);
    correctionBtn->Interact(touched,touchX,touchY);
    resultBtn->Interact(touched,touchX,touchY);

    if (millis() > nextRedraw ) {
        // draw code here
        canvas->fillSprite(ThCol(background));
        canvas->fillRoundRect(2,2,TFT_WIDTH-4,41,5,TFT_BLACK);

        if ( strlen(displayText) > 1 ) {
            if ('0' == displayText[0]) {
                sprintf(displayText,"%s",displayText+1);
            }
        }
        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(TR_DATUM);
        canvas->drawString(displayText,TFT_WIDTH-10,14);

        seven->DrawTo(canvas);
        eight->DrawTo(canvas);
        nine->DrawTo(canvas);
        four->DrawTo(canvas);
        five->DrawTo(canvas);
        six->DrawTo(canvas);
        one->DrawTo(canvas);
        two->DrawTo(canvas);
        three->DrawTo(canvas);
        zero->DrawTo(canvas);
        dotBtn->DrawTo(canvas);
        divBtn->DrawTo(canvas);        
        mulBtn->DrawTo(canvas);
        lessBtn->DrawTo(canvas);
        addBtn->DrawTo(canvas);

        resetBtn->DrawTo(canvas);
        correctionBtn->DrawTo(canvas);
        resultBtn->DrawTo(canvas);

        nextRedraw=millis()+(1000/3); // tune your required refresh
        return true;
    }
    return false;
}
