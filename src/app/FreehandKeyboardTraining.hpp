#ifndef __LUNOKIOT__APPLICATION__FreehandKeyboardTRAINING__
#define __LUNOKIOT__APPLICATION__FreehandKeyboardTRAINING__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"
#include "../system/Datasources/perceptron.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"


extern const char FreehandKeyboardLetterTrainableSymbols[];
extern const size_t FreehandKeyboardLetterTrainableSymbolsCount;

class FreehandKeyboardTraining : public TemplateApplication {
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
        
        //Perceptron *currentSymbolPerceptron=nullptr;
        size_t currentSymbolOffset=0;
        char currentSymbol='a';
        Perceptron **perceptrons; // all of them

        Perceptron **secondLayerPerceptrons;

        const uint8_t PerceptronMatrixSize=11;
        const int16_t MINIMALDRAWSIZE=PerceptronMatrixSize*4;
        const uint8_t PerceptronIterations=1; // 16 is big
        const uint8_t PerceptronIterationsNO=1;
        void Cleanup();
        void RedrawMe();
        ButtonImageXBMWidget * destroySymbolData = nullptr;
        Perceptron * BuildBlankPerceptron();
        //bool newPerceptron=true;
        //bool trainingMode=true;
        //bool trainNotSymbol=false;
        void SavePerceptronWithSymbol(char symbol,Perceptron *item);
        Perceptron * LoadPerceptronFromSymbol(char symbol);
        void RemovePerceptron(Perceptron * item);
    public:
        const char *AppName() override { return "FreehandKeyboardTraining"; };
        FreehandKeyboardTraining();
        ~FreehandKeyboardTraining();
        bool Tick();
};

#endif
