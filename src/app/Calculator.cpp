#include "Calculator.hpp"

#include "../UI/UI.hpp"
#include "../static/img_happy_48.xbm" // center smiley to see the orientation
#include <ArduinoNvs.h> // persistent values

#include <libraries/TFT_eSPI/TFT_eSPI.h>
extern TFT_eSPI *tft;
#include "LogView.hpp"

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
    seven = new ButtonWidget(0,ButtonsOffset,ButtonsSize,ButtonsSize,[this]() {
        if ( strlen(displayText) > 30) {
            sprintf(displayText,errMsg);
            return;
        }
        sprintf(displayText,"%s%d",displayText,7);
    });
    eight = new ButtonWidget(ButtonsSize,ButtonsOffset,ButtonsSize,ButtonsSize,[this]() {
        if ( strlen(displayText) > 30) {
            sprintf(displayText,errMsg);
            return;
        }
        sprintf(displayText,"%s%d",displayText,8);
    });
    nine = new ButtonWidget(ButtonsSize*2,ButtonsOffset,ButtonsSize,ButtonsSize,[this]() {
        if ( strlen(displayText) > 30) {
            sprintf(displayText,errMsg);
            return;
        }
        sprintf(displayText,"%s%d",displayText,9);
    });
    
    four = new ButtonWidget(0,ButtonsOffset+(ButtonsSize),ButtonsSize,ButtonsSize,[this]() {
        if ( strlen(displayText) > 30) {
            sprintf(displayText,errMsg);
            return;
        }
        sprintf(displayText,"%s%d",displayText,4);
    });
    five = new ButtonWidget(ButtonsSize,ButtonsOffset+(ButtonsSize),ButtonsSize,ButtonsSize,[this]() {
        if ( strlen(displayText) > 30) {
            sprintf(displayText,errMsg);
            return;
        }
        sprintf(displayText,"%s%d",displayText,5);
    });
    six = new ButtonWidget(ButtonsSize*2,ButtonsOffset+(ButtonsSize),ButtonsSize,ButtonsSize,[this]() {
        if ( strlen(displayText) > 30) {
            sprintf(displayText,errMsg);
            return;
        }
        sprintf(displayText,"%s%d",displayText,6);        
    });
    one = new ButtonWidget(0,ButtonsOffset+(ButtonsSize*2),ButtonsSize,ButtonsSize,[this]() {
        if ( strlen(displayText) > 30) {
            sprintf(displayText,errMsg);
            return;
        }
        sprintf(displayText,"%s%d",displayText,1);
    });
    two = new ButtonWidget(ButtonsSize,ButtonsOffset+(ButtonsSize*2),ButtonsSize,ButtonsSize,[this]() {
        if ( strlen(displayText) > 30) {
            sprintf(displayText,errMsg);
            return;
        }
        sprintf(displayText,"%s%d",displayText,2);
    });
    three = new ButtonWidget(ButtonsSize*2,ButtonsOffset+(ButtonsSize*2),ButtonsSize,ButtonsSize,[this]() {
        if ( strlen(displayText) > 30) {
            sprintf(displayText,errMsg);
            return;
        }
        sprintf(displayText,"%s%d",displayText,3);
    });

    zero = new ButtonWidget(0,ButtonsOffset+(ButtonsSize*3),ButtonsSize,ButtonsSize*2,[this]() {
        if ( strlen(displayText) > 30) {
            sprintf(displayText,errMsg);
            return;
        }
        sprintf(displayText,"%s%d",displayText,0);
    });

    dotBtn = new ButtonWidget(ButtonsSize*2,ButtonsOffset+(ButtonsSize*3),ButtonsSize,ButtonsSize,[this]() {
        if ( containDecimalNumber ) {
            return;
        }
        sprintf(displayText,"%s.",displayText);
        containDecimalNumber=true;
    },ThCol(background_alt));

    divBtn = new ButtonWidget(ButtonsSize*3,ButtonsOffset,ButtonsSize,ButtonsSize,[this]() {
        lastOp='/';
        if ( false == firstValueProvided ) {
            firstValue = strtof(displayText,nullptr);
            firstValueProvided=true;
            sprintf(displayText,"0");
        } else {
            resultBtn->Trigger();
            sprintf(displayText,"0");
        }
    },ThCol(highlight));

    mulBtn = new ButtonWidget(ButtonsSize*3,ButtonsOffset+(ButtonsSize),ButtonsSize,ButtonsSize,[this]() {
        lastOp='*';
        if ( false == firstValueProvided ) {
            firstValue = strtof(displayText,nullptr);
            firstValueProvided=true;
            sprintf(displayText,"0");
        } else {
            resultBtn->Trigger();
            sprintf(displayText,"0");
        }
    },ThCol(highlight));

    lessBtn = new ButtonWidget(ButtonsSize*3,ButtonsOffset+(ButtonsSize*2),ButtonsSize,ButtonsSize,[this]() {
        lastOp='-';
        if ( false == firstValueProvided ) {
            firstValue = strtof(displayText,nullptr);
            firstValueProvided=true;
            sprintf(displayText,"0");
        } else {
            resultBtn->Trigger();
            sprintf(displayText,"0");
        }
    },ThCol(highlight));

    addBtn = new ButtonWidget(ButtonsSize*3,ButtonsOffset+(ButtonsSize*3),ButtonsSize,ButtonsSize,[this]() {
        lastOp='+';
        if ( false == firstValueProvided ) {
            firstValue = strtof(displayText,nullptr);
            firstValueProvided=true;
            sprintf(displayText,"0");
        } else {
            resultBtn->Trigger();
            sprintf(displayText,"0");
        }
    },ThCol(highlight));


    resetBtn = new ButtonWidget(ButtonsSize*4,ButtonsOffset,ButtonsSize,ButtonsSize,[this]() {
        sprintf(displayText,"0");
        containDecimalNumber=false;
        firstValueProvided=false;
        firstValue=0.0;
        resultValue=0.0;
        lastOp=0;
    },ThCol(light));

    correctionBtn = new ButtonWidget(ButtonsSize*4,ButtonsOffset+(ButtonsSize),ButtonsSize,ButtonsSize,[this]() {
        size_t currLen = strlen(displayText);
        if ( 0 == currLen ) {
            sprintf(displayText,"0");
        } else {
            displayText[currLen-1]=0;
        }
    },ThCol(light));


    resultBtn = new ButtonWidget(ButtonsSize*4,ButtonsOffset+(ButtonsSize*2),ButtonsSize*2,ButtonsSize,[this]() {
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
    },ThCol(light));
    
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

        canvas->setTextColor(TFT_WHITE);
        canvas->setTextDatum(CC_DATUM);

        seven->DrawTo(canvas);
        canvas->drawString("7",seven->x+(seven->w/2),seven->y+(seven->h/2));
        eight->DrawTo(canvas);
        canvas->drawString("8",eight->x+(eight->w/2),eight->y+(eight->h/2));
        nine->DrawTo(canvas);
        canvas->drawString("9",nine->x+(nine->w/2),nine->y+(nine->h/2));
        four->DrawTo(canvas);
        canvas->drawString("4",four->x+(four->w/2),four->y+(four->h/2));
        five->DrawTo(canvas);
        canvas->drawString("5",five->x+(five->w/2),five->y+(five->h/2));
        six->DrawTo(canvas);
        canvas->drawString("6",six->x+(six->w/2),six->y+(six->h/2));
        one->DrawTo(canvas);
        canvas->drawString("1",one->x+(one->w/2),one->y+(one->h/2));
        two->DrawTo(canvas);
        canvas->drawString("2",two->x+(two->w/2),two->y+(two->h/2));
        three->DrawTo(canvas);
        canvas->drawString("3",three->x+(three->w/2),three->y+(three->h/2));
        zero->DrawTo(canvas);
        canvas->drawString("0",zero->x+(zero->w/2),zero->y+(zero->h/2));
        dotBtn->DrawTo(canvas);
        canvas->drawString(".",dotBtn->x+(dotBtn->w/2),dotBtn->y+(dotBtn->h/2));
        divBtn->DrawTo(canvas);
        canvas->setTextColor(TFT_BLACK);
        canvas->drawString("/",divBtn->x+(divBtn->w/2),divBtn->y+(divBtn->h/2));
        mulBtn->DrawTo(canvas);
        canvas->drawString("X",mulBtn->x+(mulBtn->w/2),mulBtn->y+(mulBtn->h/2));
        lessBtn->DrawTo(canvas);
        canvas->drawString("-",lessBtn->x+(lessBtn->w/2),lessBtn->y+(lessBtn->h/2));
        addBtn->DrawTo(canvas);
        canvas->drawString("+",addBtn->x+(addBtn->w/2),addBtn->y+(addBtn->h/2));

        resetBtn->DrawTo(canvas);
        canvas->setTextColor(TFT_BLACK);
        canvas->drawString("C",resetBtn->x+(resetBtn->w/2),resetBtn->y+(resetBtn->h/2));
        correctionBtn->DrawTo(canvas);
        canvas->drawString("<",correctionBtn->x+(correctionBtn->w/2),correctionBtn->y+(correctionBtn->h/2));

        resultBtn->DrawTo(canvas);
        canvas->drawString("=",resultBtn->x+(resultBtn->w/2),resultBtn->y+(resultBtn->h/2));

        nextRedraw=millis()+(1000/3); // tune your required refresh
        return true;
    }
    return false;
}
