#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Playground11.hpp"
#include "../UI/UI.hpp"
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "ApplicationBase.hpp"
#include "../UI/widgets/EntryTextWidget.hpp"

PlaygroundApplication11::~PlaygroundApplication11() {
}

PlaygroundApplication11::PlaygroundApplication11() {
    Tick();
}

bool PlaygroundApplication11::Tick() {
    
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        TemplateApplication::Tick();
        nextRedraw=millis()+(1000/12); // tune your required refresh
        return true;
    }
    return false;
}