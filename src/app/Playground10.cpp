#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Playground10.hpp"
#include "../UI/UI.hpp"
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "ApplicationBase.hpp"
#include "../UI/widgets/EntryTextWidget.hpp"

extern TTGOClass *ttgo; // ttgo library shit ;)
PlaygroundApplication10::~PlaygroundApplication10() {
    delete testEntryText;
    delete testEntryText2;
}

PlaygroundApplication10::PlaygroundApplication10() {
    testEntryText = new EntryTextWidget(40,50, 40, 160, "1234",KEYBOARD_NUMERIC);
    testEntryText2 = new EntryTextWidget(10,120, 40, 220, "hello", KEYBOARD_ALPHANUMERIC_FREEHAND);
    Tick();
}

bool PlaygroundApplication10::Tick() {
    testEntryText->Interact(touched, touchX, touchY);
    testEntryText2->Interact(touched, touchX, touchY);
    TemplateApplication::btnBack->Interact(touched,touchX,touchY);

    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        testEntryText->DrawTo(canvas);
        testEntryText2->DrawTo(canvas);
        TemplateApplication::btnBack->DrawTo(canvas);
        nextRedraw=millis()+(1000/8); // tune your required refresh
        return true;
    }
    return false;
}