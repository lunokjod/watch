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
#include "../app/LogView.hpp" // log capabilities
#include "../UI/AppLuITemplate.hpp"
#include "../UI/UI.hpp"
#include "../UI/controls/base/Control.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/Check.hpp"

extern bool UILongTapOverride;

TemplateLuIApplication::TemplateLuIApplication() {
    UILongTapOverride=true;
    directDraw=false;
    /*
    //AddChild();
    Tick();
    canvas->fillSprite(ThCol(background)); // use theme colors
    if (nullptr != child ) {
        TFT_eSprite * content = child->GetCanvas();
        content->setPivot(0,0);
        canvas->setPivot(0,0);
        content->pushRotated(canvas,0,Drawable::MASK_COLOR);
    }
    directDraw=true; // push to TFT beyond this point
    */
}

TemplateLuIApplication::~TemplateLuIApplication() {
    if (nullptr != child ) { delete child; child=nullptr; }
}

void TemplateLuIApplication::EventHandler() {
    if (nullptr == child ) { return; }
    child->EventHandler();
}

bool TemplateLuIApplication::Tick() {
    EventHandler();
    return false; // is directDraw application
}

void TemplateLuIApplication::AddChild(INOUT LuI::Container *control ) {
    child=control;
    child->SetSize(canvas->width(),canvas->height());
    child->Refresh(true); // do the wheel turn :D
}
