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

#include "LuIDebug.hpp"
#include "LogView.hpp"
#include "../UI/controls/base/Control.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/Buffer.hpp"
#include "../UI/controls/Check.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/View3D.hpp"

#include "../../tool/head.c"
#include "../../tool/Cube.c"
//#include "../../tool/robot001.c"

DebugLuIApplication::DebugLuIApplication() {
    directDraw=false; // buffer start
    LuI::Container *screen = new LuI::Container(LuI_Horizonal_Layout,2);
    screen->border=5;
    LuI::Container * body = new LuI::Container(LuI_Vertical_Layout,3);
    LuI::Container * foot = new LuI::Container(LuI_Vertical_Layout,1);
    screen->AddChild(body,1.6);
    screen->AddChild(foot,0.4);

    view3DTest0 = new LuI::View3D();
    view3DTest0->ScaleMesh = { 0.6,0.6,0.6 };
    view3DTest0->AddModel(&headMesh);
    view3DTest0->RotateMesh = { 0,5,0};
    body->AddChild(view3DTest0);
    view3DTest0->tapCallbackParam=view3DTest0;
    view3DTest0->tapCallback=[](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj;
        self->RotateMesh = { float(random(-50,50)/10),float(random(-50,50)/10),float(random(-50,50)/10) };
    };
    view3DTest0->touchEnabled=true;

    LuI::Container * shit = new LuI::Container(LuI_Horizonal_Layout,3);

    view3DTest1 = new LuI::View3D();
    view3DTest1->ScaleMesh = { 0.3,0.3,0.3 };
    view3DTest1->AddModel(&CubeMesh);
    view3DTest1->RotateMesh = { 5,0,0};
    shit->AddChild(view3DTest1);
    view3DTest1->tapCallbackParam=view3DTest1;
    view3DTest1->tapCallback=[](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj;
        self->RotateMesh = { float(random(-50,50)/10),float(random(-50,50)/10),float(random(-50,50)/10) };
    };
    view3DTest1->touchEnabled=true;

    view3DTest2 = new LuI::View3D();
    view3DTest2->ScaleMesh = { 0.15,0.7,0.15 };
    view3DTest2->AddModel(&headMesh);
    view3DTest2->RotateMesh = { 0,0,5};
    shit->AddChild(view3DTest2);
    view3DTest2->tapCallbackParam=view3DTest2;
    view3DTest2->tapCallback=[](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj;
        self->RotateMesh = { float(random(-50,50)/10),float(random(-50,50)/10),float(random(-50,50)/10) };
    };
    view3DTest2->touchEnabled=true;

    view3DTest3 = new LuI::View3D();
    view3DTest3->ScaleMesh = { 1,1,1 };
    view3DTest3->AddModel(&headMesh);
    view3DTest3->RotateMesh = { 0,-5,5};
    view3DTest3->tapCallbackParam=view3DTest3;
    view3DTest3->tapCallback=[](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj;
        self->RotateMesh = { float(random(-50,50)/10),float(random(-50,50)/10),float(random(-50,50)/10) };
    };
    view3DTest3->touchEnabled=true;
    body->AddChild(shit);
    body->AddChild(view3DTest3);


    view3DTest4 = new LuI::View3D();
    //view3DTest0->ScaleMesh = { 0.1,0.1,0.1 };
    view3DTest4->AddModel(&headMesh);
    view3DTest4->RotateMesh = { -5,-5,0};
    view3DTest4->tapCallbackParam=view3DTest4;
    view3DTest4->tapCallback=[](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj;
        self->RotateMesh = { float(random(-50,50)/10),float(random(-50,50)/10),float(random(-50,50)/10) };
    };
    view3DTest4->touchEnabled=true;
    shit->AddChild(view3DTest4);


    LuI::Container * footContainer = new LuI::Container(LuI_Vertical_Layout,3);
    foot->AddChild(footContainer);
    LuI::Button * backButton = new LuI::Button(LuI_Vertical_Layout,1);
    backButton->AddChild(new LuI::Text("Back"));
    backButton->tapCallback = [](void * obj){ LaunchWatchface(); };
    footContainer->AddChild(backButton);

    //footContainer->AddChild(nullptr);
    /*
    LuI::Button * renderButton = new LuI::Button(LuI_Vertical_Layout,1);
    renderButton->AddChild(new LuI::Text("Render"));
    renderButton->tapCallbackParam=this;
    renderButton->tapCallback = [](void * obj){
        DebugLuIApplication * self=(DebugLuIApplication*)obj;
    };*/
    //footContainer->AddChild(nullptr);


    // init
    AddChild(screen);
    Tick();
    canvas->fillSprite(ThCol(background)); // use theme colors
    if (nullptr != screen ) {
        TFT_eSprite * content = screen->GetCanvas();
        content->setPivot(0,0);
        canvas->setPivot(0,0);
        content->pushRotated(canvas,0,Drawable::MASK_COLOR);
    }
    directDraw=true; // push to TFT beyond this point
}

bool DebugLuIApplication::Tick() {
    if ( millis() > nextTime ) {
        view3DTest0->dirty=true;
        view3DTest1->dirty=true;
        view3DTest2->dirty=true;
        view3DTest3->dirty=true;
        view3DTest4->dirty=true;
        nextTime=millis()+100;
    }
    EventHandler();
    return false; // is directDraw application
}