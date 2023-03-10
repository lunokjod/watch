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

#include <Arduino.h>

#include "Playground10.hpp"
#include "../UI/UI.hpp"
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "ApplicationBase.hpp"
#include "../UI/widgets/EntryTextWidget.hpp"

 // ttgo library shit ;)
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