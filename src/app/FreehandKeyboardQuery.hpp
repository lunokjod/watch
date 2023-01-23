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

        const uint8_t PerceptronMatrixSize=11;
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
