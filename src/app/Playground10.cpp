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
    
}

PlaygroundApplication10::PlaygroundApplication10() {
    testEntryText = new EntryTextWidget(10,10, 40, 180, "aaaa");
    Tick();
}

bool PlaygroundApplication10::Tick() {
    TemplateApplication::btnBack->Interact(touched,touchX,touchY);
    testEntryText->Interact(touched, touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        testEntryText->DrawTo(canvas);
        TemplateApplication::btnBack->DrawTo(canvas);
        nextRedraw=millis()+(1000/8); // tune your required refresh
        return true;
    }
    return false;
}