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
#include "../UI/controls/View3D/Mesh3D.hpp"
#include "../UI/controls/View3D/View3D.hpp"

#include "../../tool/head.c"
#include "../../tool/Cube.c"
//#include "../../tool/glass.c"
//#include "../../tool/Sphere.c"
//#include "../../tool/Planet.c"
//#include "../../tool/robot001.c"
//#include "../../tool/bowser001.c"
//#include "../../tool/bowser002.c"
//#include "../../tool/SuperMario.c"
//#include "../../tool/Text004.c"

DebugLuIApplication::DebugLuIApplication() {
    directDraw=false; // buffer start
    LuI::Container *screen = new LuI::Container(LuI_Horizonal_Layout,2);
    LuI::Container * body = new LuI::Container(LuI_Horizonal_Layout,1);
    body->border=5;
    LuI::Container * foot = new LuI::Container(LuI_Vertical_Layout,1);
    screen->AddChild(body,1.6);
    screen->AddChild(foot,0.4);
    
    LuI::Mesh3D *test = new LuI::Mesh3D(&CubeMesh);
    test->Scale({0.5,0.5,0.5});
    test->Rotate({180,15,90});
    test->Translate({30,0,0});

    LuI::Mesh3D *test1 = new LuI::Mesh3D(&CubeMesh);
    test1->Scale({0.5,0.5,0.5});
    //test1->Rotate({180,15,90});
    test1->Translate({60,0,0});

    LuI::Mesh3D *test2 = new LuI::Mesh3D(&headMesh);
    test2->Scale({1.5,1.5,1.5});
    test2->Rotate({0,0,0});
    test2->Translate({-60,0,-60});

    view3DTest0 = new LuI::View3D();
    view3DTest0->AddMesh3D(test);
    view3DTest0->AddMesh3D(test1);
    view3DTest0->AddMesh3D(test2);

    body->AddChild(view3DTest0);

    view3DTest0->stepCallbackParam=view3DTest0;
    view3DTest0->stepCallback=[&](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj;
        self->mesh[0]->RelativeRotate({0,5,0});
        self->mesh[1]->RelativeRotate({5,0,0});
        if ( GrowOrDie ) {
            self->mesh[0]->RelativeTranslate({-10,0,0});
            self->mesh[1]->RelativeTranslate({0,-15,0});
            self->mesh[2]->RelativeScale({0.1,0.1,0.1});
        } else {
            self->mesh[0]->RelativeTranslate({10,0,0});
            self->mesh[1]->RelativeTranslate({0,15,0});
            self->mesh[2]->RelativeScale({-0.1,-0.1,-0.1});

        }
        if ( self->mesh[2]->MeshScale.x < 0.2 ) { GrowOrDie=true; }
        if ( self->mesh[2]->MeshScale.x > 1.5 ) { GrowOrDie=false; }
        self->dirty=true; // force View3D refresh
    };
    view3DTest0->stepEnabled=true;


    /*
    LuI::Container *line0 = new LuI::Container(LuI_Vertical_Layout,3);
    //line0->border=1;
    LuI::Container *line1 = new LuI::Container(LuI_Vertical_Layout,3);
    //line1->border=1;
    LuI::Container *line2 = new LuI::Container(LuI_Vertical_Layout,3);
    //line2->border=1;
    body->AddChild(line0);
    body->AddChild(line1);
    body->AddChild(line2);

    view3DTest0 = new LuI::View3D();
    view3DTest0->ScaleMesh = { 0.3,0.3,0.3 };
    view3DTest0->AddModel(&headMesh);
    line0->AddChild(view3DTest0);
    view3DTest0->tapCallbackParam=view3DTest0;
    view3DTest0->tapCallback=[](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj;
        self->FinalRotationMesh = { float(random(0,359)), float(random(0,359)), float(random(0,359)) };
        self->dirty=true;
    };
    view3DTest0->touchEnabled=true;

    view3DTest1 = new LuI::View3D();
    view3DTest1->ScaleMesh = { 0.3,0.3,0.3 };
    view3DTest1->AddModel(&CubeMesh);
    line0->AddChild(view3DTest1);
    view3DTest1->tapCallbackParam=view3DTest1;
    view3DTest1->tapCallback=view3DTest0->tapCallback;
    view3DTest1->touchEnabled=true;
    
    view3DTest2 = new LuI::View3D();
    view3DTest2->ScaleMesh = { 0.3,0.3,0.3 };
    view3DTest2->AddModel(&SphereMesh);
    line0->AddChild(view3DTest2);
    view3DTest2->tapCallbackParam=view3DTest2;
    view3DTest2->tapCallback=view3DTest0->tapCallback;
    view3DTest2->touchEnabled=true;

    view3DTest3 = new LuI::View3D();
    view3DTest3->ScaleMesh = { 0.3,0.3,0.3 };
    view3DTest3->AddModel(&PlanetMesh);
    line1->AddChild(view3DTest3);
    view3DTest3->tapCallbackParam=view3DTest3;
    view3DTest3->tapCallback=view3DTest0->tapCallback;
    view3DTest3->touchEnabled=true;

    view3DTest4 = new LuI::View3D();
    view3DTest4->ScaleMesh = { 0.3,0.3,0.3 };
    view3DTest4->AddModel(&Text004Mesh);
    line1->AddChild(view3DTest4);
    view3DTest4->tapCallbackParam=view3DTest4;
    view3DTest4->tapCallback=view3DTest0->tapCallback;
    view3DTest4->touchEnabled=true;

    view3DTest5 = new LuI::View3D();
    view3DTest5->ScaleMesh = { 0.3,0.3,0.3 };
    view3DTest5->AddModel(&PlanetMesh);
    line1->AddChild(view3DTest5);
    view3DTest5->tapCallbackParam=view3DTest5;
    view3DTest5->tapCallback=view3DTest0->tapCallback;
    view3DTest5->touchEnabled=true;

    view3DTest6 = new LuI::View3D();
    view3DTest6->ScaleMesh = { 0.3,0.3,0.3 };
    view3DTest6->AddModel(&PlanetMesh);
    line2->AddChild(view3DTest6);
    view3DTest6->tapCallbackParam=view3DTest6;
    view3DTest6->tapCallback=view3DTest0->tapCallback;
    view3DTest6->touchEnabled=true;

    view3DTest7 = new LuI::View3D();
    view3DTest7->ScaleMesh = { 0.3,0.3,0.3 };
    view3DTest7->AddModel(&PlanetMesh);
    line2->AddChild(view3DTest7);
    view3DTest7->tapCallbackParam=view3DTest7;
    view3DTest7->tapCallback=view3DTest0->tapCallback;
    view3DTest7->touchEnabled=true;

    view3DTest8 = new LuI::View3D();
    view3DTest8->ScaleMesh = { 0.3,0.3,0.3 };
    view3DTest8->AddModel(&PlanetMesh);
    line2->AddChild(view3DTest8);
    view3DTest8->tapCallbackParam=view3DTest8;
    view3DTest8->tapCallback=view3DTest0->tapCallback;
    view3DTest8->touchEnabled=true;
    */


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
        /*
        view3DTest0->dirty=true;
        view3DTest1->dirty=true;
        view3DTest2->dirty=true;
        view3DTest3->dirty=true;
        view3DTest4->dirty=true;
        view3DTest5->dirty=true;
        view3DTest6->dirty=true;
        view3DTest7->dirty=true;
        view3DTest8->dirty=true;*/
        nextTime=millis()+100;
    }
    EventHandler();
    return false; // is directDraw application
}