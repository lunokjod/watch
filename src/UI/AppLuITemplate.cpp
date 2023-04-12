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
    lAppLog("ADDCHILD BEGIN CONSTRUCTOR HERE\n");
    child=control;
    child->SetSize(canvas->width(),canvas->height());
    child->Refresh(false); // force get first frame
    // push to my canvas
    TFT_eSprite * currentView = child->GetCanvas();
    currentView->setPivot(0,0);
    canvas->setPivot(0,0);
    currentView->pushRotated(canvas,0,Drawable::MASK_COLOR); // push to app view
    lAppLog("ADDCHILD END CONSTRUCTOR HERE\n");
}
