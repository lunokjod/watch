#ifndef __LUNOKIOT__APPLICATION__SUPPORT__
#define __LUNOKIOT__APPLICATION__SUPPORT__

#include <Arduino.h>
//#include <TTGO.h>
//#include <libraries/TFT_eSPI/TFT_eSPI.h>
#include <LilyGoWatch.h>
#include <functional>

/*
 * Basic application with UI
 */
class LunokIoTApplication {
    public:
        TFT_eSprite *canvas; // application buffer (all must draw here)
        // build canvas
        LunokIoTApplication();
        virtual ~LunokIoTApplication();
        virtual bool Tick(); // the UI callback to manage input and screen
        TFT_eSprite *ScreenCapture();
        virtual const bool isWatchface() { return false; } // used to determine if the app must be replaced with watchface when PEK button
        virtual const char *AppName() = 0; // app title
        virtual const bool mustShowAsTask() { return true; } // false=Not show in task switcher (menus and intermediate views must be hidden)
        virtual void LowMemory(); // called when system fails to allocate memory (malloc)
        //virtual RunApplicationCallback GetRunCallback();
};


typedef std::function<LunokIoTApplication*()> RunApplicationCallback;

class LaunchApplicationDescriptor {
    public:
        LunokIoTApplication *instance;
        bool animation;
};

/*
 * Push the application to foreground
 */
void LaunchApplication(LunokIoTApplication *instance,bool animation=true,bool synced=false);
void LaunchWatchface(bool animation=true);
void LaunchApplicationTaskSync(LaunchApplicationDescriptor * appDescriptor,bool synched=false);


/*
 * Software keyboard is a special app
 */
enum lUIKeyboardType {
    KEYBOARD_NUMERIC,                   // 0~9
//    KEYBOARD_PHONE,                   // phone dial
//    KEYBOARD_ALPHANUMERIC_SEQUENCE,   // old phone 3x3 buttons ABC-DEF-GHI.... sequence
//    KEYBOARD_HEXADECIMAL,             // usefull for old hex passwords
//    KEYBOARD_PATTERN,                 // unlock pattern 3x3/4x4...
//    KEYBOARD_NUMERIC_FREEHAND,        // draw numbers to OCR
//    KEYBOARD_ALPHANUMERIC_FREEHAND,   // draw letters to be recognize
//    KEYBOARD_IMAGE_DRAW,              // attach image instead of text
//    KEYBOARD_FULL_BLUETOOTH,          // use full keyboard bluetooth
};

//#include "../UI/widgets/EntryTextWidget.hpp"
#include "../UI/widgets/ButtonTextWidget.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class SoftwareKeyboard : public LunokIoTApplication {
    public:
        //TFT_eSprite * appCopy=nullptr;
        char *textEntry=nullptr;
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
        void * destinationWidget=nullptr;
        //TFT_eSprite * destinationCanvas=nullptr;
        lUIKeyboardType keyboardType=KEYBOARD_NUMERIC;
        SoftwareKeyboard(void * destinationWidget=nullptr, lUIKeyboardType keyboardType=KEYBOARD_NUMERIC);
        ~SoftwareKeyboard();
        bool Tick();
        const char *AppName() override { return "Software keyboard"; }
        const bool mustShowAsTask() override { return false; }

};
#endif
