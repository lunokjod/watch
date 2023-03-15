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
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "NuControls.hpp"

#include "../UI/controls/base/Control.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../static/img_poweroff_fullscreen.c"
#include "../UI/controls/Image.hpp"

NuControlsApplication::NuControlsApplication() {
    LuI::Image * test0 = new LuI::Image(img_poweroff_fullscreen.width,img_poweroff_fullscreen.height,img_poweroff_fullscreen.pixel_data);
    LuI::Image * test1 = new LuI::Image(img_poweroff_fullscreen.width,img_poweroff_fullscreen.height,img_poweroff_fullscreen.pixel_data);
    LuI::Image * test2 = new LuI::Image(img_poweroff_fullscreen.width,img_poweroff_fullscreen.height,img_poweroff_fullscreen.pixel_data);
    LuI::Image * test3 = new LuI::Image(img_poweroff_fullscreen.width,img_poweroff_fullscreen.height,img_poweroff_fullscreen.pixel_data);
    LuI::Image * test4 = new LuI::Image(img_poweroff_fullscreen.width,img_poweroff_fullscreen.height,img_poweroff_fullscreen.pixel_data);
    LuI::Image * test5 = new LuI::Image(img_poweroff_fullscreen.width,img_poweroff_fullscreen.height,img_poweroff_fullscreen.pixel_data);
    LuI::Image * test6 = new LuI::Image(img_poweroff_fullscreen.width,img_poweroff_fullscreen.height,img_poweroff_fullscreen.pixel_data);
    
    LuI::Container * testContainerH = new LuI::Container(LuI_Horizonal_Layout,3);
    LuI::Container * testContainerV0 = new LuI::Container(LuI_Vertical_Layout,2);
    LuI::Container * testContainerV1 = new LuI::Container(LuI_Vertical_Layout,3);
    LuI::Container * testContainerV2 = new LuI::Container(LuI_Vertical_Layout,2);
    
    testContainerV0->AddChild(test0,0.5);
    testContainerV0->AddChild(test1,1.5);

    testContainerV1->AddChild(test2,1.5);
    testContainerV1->AddChild(test3,1.0);
    testContainerV1->AddChild(test4,0.5);

    testContainerV2->AddChild(test5,1.0);
    testContainerV2->AddChild(test6,1.0);

    testContainerH->AddChild(testContainerV0,0.8);
    testContainerH->AddChild(testContainerV1,1.8);
    testContainerH->AddChild(testContainerV2,0.4);

    AddChild(testContainerH);
    testContainerH->Refresh(); //this triggers whole refresh of children
    Tick();
}

NuControlsApplication::~NuControlsApplication() {
    if (nullptr != child ) { delete child; child=nullptr; }
}

bool NuControlsApplication::Tick() {
    TemplateApplication::Tick();

    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(ThCol(background)); // use theme colors
        TemplateApplication::btnBack->DrawTo(canvas);
        if (nullptr != child ) {
            TFT_eSprite * content = child->GetCanvas();
            content->setPivot(0,0);
            canvas->setPivot(0,0);
            content->pushRotated(canvas,0,Drawable::MASK_COLOR);
        }
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}

void NuControlsApplication::AddChild(INOUT LuI::Container *control ) {
    child=control;
    child->SetSize(canvas->width(),canvas->height());
}
