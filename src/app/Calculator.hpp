#ifndef __LUNOKIOT__CALCULATOR_APP__
#define __LUNOKIOT__CALCULATOR_APP__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../system/Application.hpp"
#include "../UI/widgets/ButtonTextWidget.hpp"

class CalculatorApplication: public LunokIoTApplication {
    private:
        unsigned long nextRedraw=0;
        ButtonTextWidget * seven=nullptr;
        ButtonTextWidget * eight=nullptr;
        ButtonTextWidget * nine=nullptr;
        ButtonTextWidget * four=nullptr;
        ButtonTextWidget * five=nullptr;
        ButtonTextWidget * six=nullptr;
        ButtonTextWidget * one=nullptr;
        ButtonTextWidget * two=nullptr;
        ButtonTextWidget * three=nullptr;
        ButtonTextWidget * zero=nullptr;
        ButtonTextWidget * dotBtn=nullptr;

        ButtonTextWidget * divBtn=nullptr;
        ButtonTextWidget * mulBtn=nullptr;
        ButtonTextWidget * lessBtn=nullptr;
        ButtonTextWidget * addBtn=nullptr;

        ButtonTextWidget * correctionBtn=nullptr;
        ButtonTextWidget * resetBtn=nullptr;
        ButtonTextWidget * resultBtn=nullptr;

    public:
        const char *AppName() override { return "Calculator"; };
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
