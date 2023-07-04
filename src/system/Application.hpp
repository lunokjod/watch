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

#ifndef __LUNOKIOT__APPLICATION__SUPPORT__
#define __LUNOKIOT__APPLICATION__SUPPORT__
#include <Arduino.h>
#include "../UI/UI.hpp"
const uint8_t APPLICATIONCORE=UICORE;
//#include <Arduino.h>
//#include <TTGO.h>

//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include <LilyGoWatch.h>
#include <functional>
#include "../system/Datasources/perceptron.hpp"
#define tunable public // all settings accesable from outside
/*
 * Basic application with UI
 */
class LunokIoTApplication {
    public:
        bool dirty=false;
        TFT_eSprite *canvas; // application buffer (all must draw here)
        // build canvas
        LunokIoTApplication();
        virtual ~LunokIoTApplication();
        virtual bool Tick(); // the UI callback to manage input and screen
        //TFT_eSprite *ScreenCapture();
        virtual const bool isWatchface() { return false; } // used to determine if the app must be replaced with watchface when PEK button
        virtual const char *AppName() = 0; // app title
        virtual const bool mustShowAsTask() { return true; } // false=Not show in task switcher (menus and intermediate views must be hidden)
        virtual void LowMemory(); // called when system fails to allocate memory (malloc)
        //virtual RunApplicationCallback GetRunCallback();
};
typedef LunokIoTApplication* WatchfaceMaker();
template <class WFA> LunokIoTApplication* MakeWatch() { return new WFA; }
size_t WatchFacesAvailiable();
LunokIoTApplication *GetWatchFaceFromOffset(size_t off);
typedef std::function<LunokIoTApplication*()> RunApplicationCallback;

class LaunchApplicationDescriptor {
    public:
        LunokIoTApplication *instance;
        bool animation;
};

/*
 * Push the application to foreground
 */
void LaunchApplication(LunokIoTApplication *instance,bool animation=true,bool synced=false, bool force=false);
void LaunchWatchface(bool animation=false, bool force=false);
void LaunchApplicationTaskSync(LaunchApplicationDescriptor * appDescriptor,bool synched=false);

/*
 * Software keyboard is a special app
 */
enum lUIKeyboardType {
    KEYBOARD_NUMERIC,                   // 0~9
    KEYBOARD_ALPHANUMERIC_FREEHAND,     // draw letters to be recognize
//    KEYBOARD_PHONE,                   // phone dial
//    KEYBOARD_ALPHANUMERIC_SEQUENCE,   // old phone 3x3 buttons ABC-DEF-GHI.... sequence
//    KEYBOARD_HEXADECIMAL,             // usefull for old hex passwords
//    KEYBOARD_PATTERN,                 // unlock pattern 3x3/4x4...
//    KEYBOARD_NUMERIC_FREEHAND,        // draw numbers to OCR
//    KEYBOARD_IMAGE_DRAW,              // attach image instead of text
//    KEYBOARD_FULL_BLUETOOTH,          // use full keyboard bluetooth
};

//#include "../UI/widgets/EntryTextWidget.hpp"
#include "../UI/widgets/ButtonTextWidget.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

// base keyboard class
class SoftwareKeyboard : public LunokIoTApplication {
    public:
        char *textEntry=nullptr;
        const bool mustShowAsTask() final { return false; }
        SoftwareKeyboard(void * destinationWidget=nullptr);
        ~SoftwareKeyboard();
        void * destinationWidget=nullptr;
};

class SoftwareNumericKeyboard : public SoftwareKeyboard {
    public:
        uint8_t cursorBlinkStep=0;
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
        bool dotAdded=false;
        ButtonTextWidget * dotBtn=nullptr;

        ButtonImageXBMWidget * btnCancel=nullptr;
        ButtonImageXBMWidget * btnDiscard=nullptr;
        ButtonImageXBMWidget * btnSend=nullptr;
        //TFT_eSprite * destinationCanvas=nullptr;
        SoftwareNumericKeyboard(void * destinationWidget=nullptr );
        ~SoftwareNumericKeyboard();
        bool Tick() override;
        const char *AppName() override { return "Software numeric keyboard"; }
};

extern const char FreehandKeyboardLetterTrainableSymbols[];
extern const size_t FreehandKeyboardLetterTrainableSymbolsCount;

class SoftwareFreehandKeyboard : public SoftwareKeyboard {
    public:
        SoftwareFreehandKeyboard(void * destinationWidget=nullptr );
        ~SoftwareFreehandKeyboard();
        bool Tick() override;
        const char *AppName() override { return "Software freehand keyboard"; }
        Perceptron **perceptrons;
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
        /*
        const size_t BufferTextMax=20;
        char * bufferText=nullptr;
        size_t bufferTextOffset=0;
        */
        ButtonImageXBMWidget * btnCancel=nullptr;
        ButtonImageXBMWidget * btnDiscard=nullptr;
        ButtonImageXBMWidget * btnSend=nullptr;
};

#endif
