#ifndef __LUNOKIOT__CALCULATOR_APP__
#define __LUNOKIOT__CALCULATOR_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/ButtonWidget.hpp"
class CalculatorApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        ButtonWidget * seven=nullptr;
        ButtonWidget * eight=nullptr;
        ButtonWidget * nine=nullptr;
        ButtonWidget * four=nullptr;
        ButtonWidget * five=nullptr;
        ButtonWidget * six=nullptr;
        ButtonWidget * one=nullptr;
        ButtonWidget * two=nullptr;
        ButtonWidget * three=nullptr;
        ButtonWidget * zero=nullptr;
        ButtonWidget * dotBtn=nullptr;

        ButtonWidget * divBtn=nullptr;
        ButtonWidget * mulBtn=nullptr;
        ButtonWidget * lessBtn=nullptr;
        ButtonWidget * addBtn=nullptr;

        ButtonWidget * correctionBtn=nullptr;
        ButtonWidget * resetBtn=nullptr;
        ButtonWidget * resultBtn=nullptr;

    public:
        bool containDecimalNumber=false;
        const char *errMsg = "err";
        bool firstValueProvided=false;
        float firstValue = 0.0;
        float resultValue = 0.0;
        char lastOp = 0;
        char displayText[32] = { 0 };
        CalculatorApplication();
        ~CalculatorApplication();
        bool Tick();
};

#endif
