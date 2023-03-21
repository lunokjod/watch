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
#include "AssistantPage0.hpp"
#include "AssistantPage1.hpp"
#include "Assistant.hpp"
#include "../UI/UI.hpp"
#include "../UI/controls/base/Control.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/Check.hpp"

#include "../../static/img_next_32.xbm"
#include "../../static/img_last_32.xbm"
#include "../static/img_sputnik_64.xbm" // sputnik image
#include "../static/img_background_firstrun.c"

#include "../system/SystemEvents.hpp"
extern bool UILongTapOverride;

AssistantPage0Application::AssistantPage0Application() {
    UILongTapOverride=true;
    directDraw=false;

    LuI::Image * screen = new LuI::Image(img_background_firstrun.width,img_background_firstrun.height,img_background_firstrun.pixel_data,true, LuI_Horizonal_Layout,3);
    LuI::Container * body = new LuI::Container(LuI_Horizonal_Layout,1);
    LuI::Container * foot = new LuI::Container(LuI_Vertical_Layout,5);
    foot->border=5;
    screen->AddChild(body,2.0);
    screen->AddChild(foot,0.8);
    screen->AddChild(nullptr,0.2);

    // back button
    LuI::Button * backButton = new LuI::Button(LuI_Vertical_Layout,1);
    LuI::XBM * backButtonImage = new LuI::XBM(img_last_32_width,img_last_32_height,img_last_32_bits);
    //backButton->tapCallbackParam=this;
    backButton->tapCallback = [](void * obj){
        //AssistantApplication * self = (AssistantApplication *)obj;
        LaunchApplication(new AssistantApplication());
    };
    backButton->AddChild(backButtonImage);

    // next button
    LuI::Button * nextButton = new LuI::Button(LuI_Vertical_Layout,2);
    LuI::XBM * nextButtonImage = new LuI::XBM(img_next_32_width,img_next_32_height,img_next_32_bits);
    LuI::Text * nextButtonText = new LuI::Text("Next",ThCol(text),true);
    //nextButton->tapCallbackParam=this;
    nextButton->tapCallback = [](void * obj){
        //AssistantApplication * self = (AssistantApplication *)obj;
        LaunchApplication(new AssistantPage1Application());
    };
    nextButton->AddChild(nextButtonText);
    nextButton->AddChild(nextButtonImage);

    foot->AddChild(nullptr,0.1);
    foot->AddChild(backButton,1.2);
    foot->AddChild(nullptr,1.3);
    foot->AddChild(nextButton,2.3);
    foot->AddChild(nullptr,0.1);


    LuI::Container * bodyContainer = new LuI::Container(LuI_Horizonal_Layout,3);
    bodyContainer->border=3;
    body->AddChild(bodyContainer);
    
    LuI::Container * checkContainer = new LuI::Container(LuI_Vertical_Layout,2);
    checkContainer->AddChild(new LuI::Check(),0.6);
    checkContainer->AddChild(new LuI::Text("Use stepcounter"),1.4);
    bodyContainer->AddChild(checkContainer);

    LuI::Container * checkContainer2 = new LuI::Container(LuI_Vertical_Layout,2);
    checkContainer2->AddChild(new LuI::Check(),0.6);
    checkContainer2->AddChild(new LuI::Text("Use activities"),1.4);
    bodyContainer->AddChild(checkContainer2);

    LuI::Container * checkContainer3 = new LuI::Container(LuI_Vertical_Layout,2);
    checkContainer3->AddChild(new LuI::Check(),0.6);
    checkContainer3->AddChild(new LuI::Text("Log activities"),1.4);
    bodyContainer->AddChild(checkContainer3);

    AddChild(screen);
    Tick();
    canvas->fillSprite(ThCol(background)); // use theme colors
    if (nullptr != child ) {
        TFT_eSprite * content = child->GetCanvas();
        content->setPivot(0,0);
        canvas->setPivot(0,0);
        content->pushRotated(canvas,0,Drawable::MASK_COLOR);
    }
    directDraw=true; // push to TFT beyond this point
}

AssistantPage0Application::~AssistantPage0Application() {
    if (nullptr != child ) { delete child; child=nullptr; }
}

void AssistantPage0Application::EventHandler() {
    if (nullptr == child ) { return; }
    //lAppLog("Begin Event handler\n");
    child->EventHandler();
}

bool AssistantPage0Application::Tick() {
    EventHandler();
    /*
    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(ThCol(background)); // use theme colors
        if (nullptr != child ) {
            TFT_eSprite * content = child->GetCanvas();
            content->setPivot(0,0);
            canvas->setPivot(0,0);
            content->pushRotated(canvas,0,Drawable::MASK_COLOR);
        }
        nextRefresh=millis()+(1000/4);
        return true;
    }*/
    return false;
}

void AssistantPage0Application::AddChild(INOUT LuI::Container *control ) {
    child=control;
    child->SetSize(canvas->width(),canvas->height());
    child->Refresh(true);
}
