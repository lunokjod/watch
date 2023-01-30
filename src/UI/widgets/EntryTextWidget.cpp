#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"
#include "EntryTextWidget.hpp"
#include "ButtonTextWidget.hpp"
#include "../../app/LogView.hpp"
#include "../UI.hpp"
#include "CanvasWidget.hpp"
#include <functional>

extern SoftwareKeyboard *keyboardInstance;
extern SemaphoreHandle_t UISemaphore;

void EntryTextWidget::HideKeyboard(char *newValue) {
    lUILog("HideKeyboard()\n");
    if( xSemaphoreTake( UISemaphore, portMAX_DELAY) == pdTRUE )  {
        if ( nullptr != newValue ) {
            strcpy(ptrToText,newValue);
            label=(const char*)ptrToText;
            // must be synched with UI, or glitches can appear:-/
            InternalRedraw();
        }
        if ( nullptr != keyboardInstance ) {
            delete keyboardInstance;
            keyboardInstance=nullptr;
        }
        xSemaphoreGive( UISemaphore );
    }
}
void EntryTextWidget::ShowKeyboard() {
    lUILog("ShowKeyboard()\n");
    if ( nullptr != keyboardInstance ) {
        delete keyboardInstance;
        keyboardInstance=nullptr;
    }
    if ( KEYBOARD_NUMERIC == keyboarTypeToShow ) {
        keyboardInstance=new SoftwareNumericKeyboard(this);
    } else if ( KEYBOARD_ALPHANUMERIC_FREEHAND == keyboarTypeToShow ) {
        keyboardInstance=new SoftwareFreehandKeyboard(this);
    }
    if ( nullptr != ptrToText ) {
        strcpy(keyboardInstance->textEntry,ptrToText);
    }
}

EntryTextWidget::EntryTextWidget(int16_t x,int16_t y, int16_t h, int16_t w, 
            const char *text, lUIKeyboardType keybType)
            : ButtonTextWidget(x,y,h,w,[](void *data){
                EntryTextWidget * receiver=(EntryTextWidget *)data;
                if ( nullptr != keyboardInstance ) {
                    receiver->HideKeyboard();
                } else {
                    receiver->ShowKeyboard();
                }
                //receiver->label="LOLOLO";
                receiver->InternalRedraw();

            },text,TFT_BLACK,TFT_WHITE,true), keyboarTypeToShow(keybType) {
    lUIDeepLog("%s new %p\n",__PRETTY_FUNCTION__,this);
    ButtonTextWidget::paramCallback=this;
    ptrToText=(char*)malloc(256);
    ptrToText[0]=0;
    if ( nullptr != text ) { strcpy(ptrToText,text); }
    InternalRedraw();
}

void EntryTextWidget::InternalRedraw() {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    ButtonWidget::InternalRedraw(); // call button normal draw
    // add the text
    if ( nullptr != label ) {
        buffer->canvas->setTextFont(0);
        buffer->canvas->setTextSize(3);
        buffer->canvas->setTextColor(btnTextColor);
        buffer->canvas->setTextDatum(CC_DATUM);
        buffer->canvas->drawString(label,(w/2),(h/2));
    }

    buffer->canvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
}

void EntryTextWidget::DrawTo(TFT_eSprite * endCanvas) {
    lUIDeepLog("%s %p\n",__PRETTY_FUNCTION__,this);
    if ( nullptr == canvas ) { return; }
    if ( nullptr == endCanvas ) { return; }
    CanvasWidget::DrawTo(endCanvas,x,y);
    //lastCanvas=endCanvas;
}
