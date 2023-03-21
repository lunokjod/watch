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
#include "Assistant.hpp"
#include "../UI/UI.hpp"
#include "../UI/controls/base/Control.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/XBM.hpp"

#include "../../static/img_next_32.xbm"
#include "../../static/img_last_32.xbm"
//#include "../static/img_sputnik_64.xbm" // sputnik image
#include "../static/img_background_firstrun.c"
#include "AssistantPage0.hpp"

extern bool UILongTapOverride;

AssistantApplication::AssistantApplication() {
    UILongTapOverride=true;
    directDraw=false;

    LuI::Image * screen = new LuI::Image(img_background_firstrun.width,img_background_firstrun.height,img_background_firstrun.pixel_data,true, LuI_Horizonal_Layout,3);
    //LuI::Container * screen = new LuI::Container(LuI_Horizonal_Layout,2);
    //LuI::Container * header = new LuI::Container(LuI_Vertical_Layout,3);
    //header->border=5;
    LuI::Container * body = new LuI::Container(LuI_Horizonal_Layout,2);
    //LuI::Image * body = new LuI::Image(img_background_firstrun.width,img_background_firstrun.height,img_background_firstrun.pixel_data,false, LuI_Horizonal_Layout,2);
    LuI::Container * foot = new LuI::Container(LuI_Vertical_Layout,5);
    foot->border=5;
    // build the screen
    //screen->AddChild(header,0.8);
    screen->AddChild(body,2.0);
    screen->AddChild(foot,0.8);
    screen->AddChild(nullptr,0.2);
    /*
    // header
    LuI::XBM * sputnikImg = new LuI::XBM(img_sputnik_64_width,img_sputnik_64_height,img_sputnik_64_bits);
    LuI::Container * headerTextContainer = new LuI::Container(LuI_Horizonal_Layout,3);
    LuI::Text * headerText = new LuI::Text("Welcome!",ThCol(text),true,1,&FreeMonoBold12pt7b);
    LuI::Text * headerSubText0 = new LuI::Text("First run",ThCol(text),true,1,&FreeMonoBold9pt7b);
    LuI::Text * headerSubText1 = new LuI::Text("Assistant",ThCol(text),true,1,&FreeMonoBold9pt7b);
    // add logo left
    header->AddChild(sputnikImg,0.6);

    // compose 3 lines
    headerTextContainer->AddChild(headerText,1.5);
    headerTextContainer->AddChild(headerSubText0,0.75);
    headerTextContainer->AddChild(headerSubText1,0.75);
    // text right
    header->AddChild(headerTextContainer,1.4);
    */
    // fill body
    LuI::Container * bodyDiv = new LuI::Container(LuI_Vertical_Layout,4);
    bodyDiv->border=2;
    LuI::Container * bodyDiv2 = new LuI::Container(LuI_Vertical_Layout,4);
    
    //let me know about yourself
    bodyDiv->AddChild(nullptr,0.5);

    LuI::Container * letDiv = new LuI::Container(LuI_Horizonal_Layout,2);
    LuI::Text *myTextLet = new LuI::Text("let me",ThCol(text),true,1,&FreeMonoBold12pt7b);
    letDiv->AddChild(nullptr);
    letDiv->AddChild(myTextLet);
    bodyDiv->AddChild(letDiv,1.5);

    LuI::Container * warf = new LuI::Container(LuI_Horizonal_Layout,2);
    bodyDiv->AddChild(warf,1.5);
    warf->AddChild(nullptr);
    warf->AddChild(new LuI::Text("know",ThCol(text),true,1,&FreeMonoBold18pt7b));
    bodyDiv->AddChild(nullptr,0.5);

    bodyDiv2->AddChild(nullptr,0.7);

    LuI::Container * boutDiv = new LuI::Container(LuI_Horizonal_Layout,2);
    boutDiv->AddChild(new LuI::Text("about",ThCol(text),true,1,&FreeMonoBold18pt7b));
    boutDiv->AddChild(nullptr);
    bodyDiv2->AddChild(boutDiv,2.0);
    
    LuI::Container * youDiv = new LuI::Container(LuI_Horizonal_Layout,2);
    LuI::Text *myText = new LuI::Text("you",ThCol(text),true,1,&FreeMonoBold9pt7b);
    youDiv->AddChild(myText);
    youDiv->AddChild(nullptr);

    bodyDiv2->AddChild(youDiv,0.8);

    body->AddChild(bodyDiv,1.2);
    body->AddChild(bodyDiv2,0.8);

    // back button
    LuI::Button * backButton = new LuI::Button(LuI_Vertical_Layout,1);
    LuI::XBM * backButtonImage = new LuI::XBM(img_last_32_width,img_last_32_height,img_last_32_bits);
    //LuI::Text * backButtonText = new LuI::Text("Back",ThCol(text),true);
    //backButton->AddChild(backButtonText);
    backButton->tapCallbackParam=this;
    backButton->tapCallback = [](void * obj){
        AssistantApplication * self = (AssistantApplication *)obj;
        lLog("Button 'back' pushed on %p \n",self);
        LaunchWatchface();
    };
    backButton->AddChild(backButtonImage);

    // next button
    LuI::Button * nextButton = new LuI::Button(LuI_Vertical_Layout,2);
    LuI::XBM * nextButtonImage = new LuI::XBM(img_next_32_width,img_next_32_height,img_next_32_bits);
    LuI::Text * nextButtonText = new LuI::Text("Next",ThCol(text),true);
    nextButton->tapCallbackParam=this;
    nextButton->tapCallback = [](void * obj){
        AssistantApplication * self = (AssistantApplication *)obj;
        LaunchApplication(new AssistantPage0Application());
        //lLog("lol from %p\n",self);
    };
    nextButton->AddChild(nextButtonText);
    nextButton->AddChild(nextButtonImage);

    foot->AddChild(nullptr,0.1);
    foot->AddChild(backButton,1.2);
    foot->AddChild(nullptr,1.3);
    foot->AddChild(nextButton,2.3);
    foot->AddChild(nullptr,0.1);


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

AssistantApplication::~AssistantApplication() {
    if (nullptr != child ) { delete child; child=nullptr; }
}

void AssistantApplication::EventHandler() {
    if (nullptr == child ) { return; }
    //lAppLog("Begin Event handler\n");
    child->EventHandler();
}

bool AssistantApplication::Tick() {
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

void AssistantApplication::AddChild(INOUT LuI::Container *control ) {
    child=control;
    child->SetSize(canvas->width(),canvas->height());
    child->Refresh(true);
}
