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

#ifndef __LUNOKIOT__APPLICATION__FreehandKeyboardQUERY___
#define __LUNOKIOT__APPLICATION__FreehandKeyboardQUERY___
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"
#include "../system/Datasources/perceptron.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"


extern const char FreehandKeyboardLetterTrainableSymbols[];
extern const size_t FreehandKeyboardLetterTrainableSymbolsCount;

class FreehandKeyboardQuery : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
        unsigned long oneSecond=0; // trigger clear pad
        int16_t boxX=TFT_WIDTH; // get the box of draw
        int16_t boxY=TFT_HEIGHT;
        int16_t boxH=0;
        int16_t boxW=0;
        bool triggered=false; // to launch the recognizer
        TFT_eSprite * perceptronCanvas=nullptr;
        TFT_eSprite * freeHandCanvas=nullptr;

        const uint8_t PerceptronMatrixSize=7;
        const int16_t MINIMALDRAWSIZE=PerceptronMatrixSize*4;
        void Cleanup();
        void RedrawMe();
        void RemovePerceptron(Perceptron * item);
        Perceptron * LoadPerceptronFromSymbol(char symbol);
        Perceptron * BuildBlankPerceptron();
        const size_t BufferTextMax=20;
        char * bufferText=nullptr;
        size_t bufferTextOffset=0;
    public:
        const char *AppName() override { return "FreehandKeyboardQuery"; };
        FreehandKeyboardQuery();
        ~FreehandKeyboardQuery();
        bool Tick();
        Perceptron **perceptrons;
};

#endif
