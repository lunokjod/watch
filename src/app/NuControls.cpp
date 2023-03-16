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
#include "../UI/UI.hpp"
#include "../UI/controls/base/Control.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/XBM.hpp"

#include "../../static/img_next_32.xbm"
#include "../../static/img_last_32.xbm"
#include "../static/img_sputnik_64.xbm" // sputnik image
extern bool UILongTapOverride;
NuControlsApplication::NuControlsApplication() {
    UILongTapOverride=true;
    LuI::Container * containerH0 = new LuI::Container(LuI_Horizonal_Layout,3);
    LuI::Container * headerV0 = new LuI::Container(LuI_Vertical_Layout,2);
    LuI::Container * bodyV1 = new LuI::Container(LuI_Vertical_Layout,1);
    LuI::Container * footerV2 = new LuI::Container(LuI_Vertical_Layout,3);

    // header
    LuI::XBM * sputnikImg = new LuI::XBM(img_sputnik_64_width,img_sputnik_64_height,img_sputnik_64_bits);
    LuI::Text * testText = new LuI::Text("Welcome!",TFT_YELLOW,false,&FreeMonoBold12pt7b);
    headerV0->AddChild(sputnikImg,0.6);
    headerV0->AddChild(testText,1.4);

    //@TODO fill the bodyV1
    LuI::Container * body0 = new LuI::Container(LuI_Vertical_Layout,2);
    LuI::Control *separator = new LuI::Control();
    LuI::Container * body1 = new LuI::Container(LuI_Horizonal_Layout,2);
    body0->AddChild(separator);
    body0->AddChild(body1);
    LuI::Container * body2 = new LuI::Container(LuI_Vertical_Layout,2);
    separator = new LuI::Control();
    body1->AddChild(separator);
    body1->AddChild(body2);
    LuI::Container * body3 = new LuI::Container(LuI_Horizonal_Layout,2);
    body2->AddChild(body3);
    LuI::Container * body4 = new LuI::Container(LuI_Vertical_Layout,2);
    body3->AddChild(body4);
    LuI::Container * body5 = new LuI::Container(LuI_Horizonal_Layout,2);
    separator = new LuI::Control();
    body4->AddChild(separator);
    body4->AddChild(body5);
    LuI::Container * body6 = new LuI::Container(LuI_Vertical_Layout,2);
    separator = new LuI::Control();
    body5->AddChild(separator);
    body5->AddChild(body6);
    LuI::Container * body7 = new LuI::Container(LuI_Horizonal_Layout,2);
    separator = new LuI::Control();
    body6->AddChild(separator);
    body6->AddChild(body7);

    bodyV1->AddChild(body0);

    // back button
    LuI::Button * backButton = new LuI::Button(LuI_Vertical_Layout,2);
    LuI::XBM * backButtonImage = new LuI::XBM(img_last_32_width,img_last_32_height,img_last_32_bits);
    LuI::Text * backButtonText = new LuI::Text("Back",ThCol(text),true);
    backButton->AddChild(backButtonImage);
    backButton->AddChild(backButtonText);
    backButton->callbackParam=this;
    backButton->callback = [](void * obj){
        NuControlsApplication * self = (NuControlsApplication *)obj;
        lLog("YEAHHH from %p\n",self);
    };
    footerV2->AddChild(backButton,1.4);

    // separator
    footerV2->AddChild(new LuI::Control(),0.2);


    // next button
    LuI::Button * nextButton = new LuI::Button(LuI_Vertical_Layout,2);
    LuI::XBM * nextButtonImage = new LuI::XBM(img_next_32_width,img_next_32_height,img_next_32_bits);
    LuI::Text * nextButtonText = new LuI::Text("Next",ThCol(text),true);
    nextButton->AddChild(nextButtonText);
    nextButton->AddChild(nextButtonImage);
    footerV2->AddChild(nextButton,1.4);
    nextButton->callbackParam=this;
    nextButton->callback = [](void * obj){
        NuControlsApplication * self = (NuControlsApplication *)obj;
        lLog("lol from %p\n",self);
    };

    // build the screen
    containerH0->AddChild(headerV0,0.8);
    containerH0->AddChild(bodyV1,1.6);
    containerH0->AddChild(footerV2,0.6);
    AddChild(containerH0);
    Tick();
}

NuControlsApplication::~NuControlsApplication() {
    if (nullptr != child ) { delete child; child=nullptr; }
}

void NuControlsApplication::EventHandler() {
    if (nullptr == child ) { return; }
    //lAppLog("Begin Event handler\n");
    child->EventHandler();
}

bool NuControlsApplication::Tick() {
    TemplateApplication::Tick();
    EventHandler();
    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(ThCol(background)); // use theme colors
        TemplateApplication::btnBack->DrawTo(canvas);
        if (nullptr != child ) {
            TFT_eSprite * content = child->GetCanvas();
            content->setPivot(0,0);
            canvas->setPivot(0,0);
            content->pushRotated(canvas,0,Drawable::MASK_COLOR);
        }
        EventHandler();
        nextRefresh=millis()+(1000/4);
        return true;
    }
    return false;
}

void NuControlsApplication::AddChild(INOUT LuI::Container *control ) {
    child=control;
    child->SetSize(canvas->width(),canvas->height());
    child->Refresh();
}
