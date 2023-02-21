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
