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

//#include "../../tool/head.c"
#include "../../tool/Cube.c"
//#include "../../tool/glass.c"
//#include "../../tool/Sphere.c"
#include "../../tool/BackButton.c"
//#include "../../tool/Planet001.c"
//#include "../../tool/robot001.c"
//#include "../../tool/bowser003.c"
//#include "../../tool/building001.c"
#include "../../tool/SuperMario003.c"
//#include "../../tool/Text004.c"
//#include "../../tool/Text005.c"

DebugLuIApplication::DebugLuIApplication() {
    directDraw=false; // buffer start
    LuI::Container *screen = new LuI::Container(LuI_Horizonal_Layout,1);
    //screen->border=8;
    LuI::Container * body = new LuI::Container(LuI_Vertical_Layout,1);
    body->border=5;
    //LuI::Container * foot = new LuI::Container(LuI_Vertical_Layout,1);
    screen->AddChild(body);
    //screen->AddChild(foot,0.4);

    //LuI::Container * footContainer = new LuI::Container(LuI_Vertical_Layout,2);
    //foot->AddChild(footContainer);
    /*
    // want button with 2 slots in vertical layout (50/50)
    LuI::Button * backButton = new LuI::Button(LuI_Vertical_Layout,2);

    // load mesh from blender python script .c generated
    LuI::Mesh3D * backAnimation = new LuI::Mesh3D(&BackButtonMesh);
    // create a view for backAnimation
    view3DTest0 = new LuI::View3D();
    // small please!
    backAnimation->Scale(0.15);
    // add to view
    view3DTest0->AddMesh3D(backAnimation);
    // ugly workarround, transparent causes drops
    view3DTest0->viewBackgroundColor=ThCol(button);
    // use this as paramether for the callback when the render is done
    view3DTest0->stepCallbackParam=view3DTest0;
    // set a callback when the render is done
    view3DTest0->stepCallback=[&](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj; // recover the view3DTest0
        self->mesh[0]->RelativeRotate({0,5,0}); // modify the first mesh (PlanetMesh) rotation
        self->dirty=true; // mark control as dirty (forces redraw)
    };
    // add 3DView to button on first slot
    backButton->AddChild(view3DTest0);
    backButton->AddChild(new LuI::Text("Back"));
    // add plain text to the next slot
    // what to do when tap?
    backButton->tapCallback = [](void * obj){ LaunchWatchface(); };
    // add button to before defined container
    footContainer->AddChild(backButton,0.9);
    footContainer->AddChild(nullptr,1.1);
    */
    // init
    canvas->fillSprite(ThCol(background)); // use theme colors
    /*
    if (nullptr != screen ) {
        TFT_eSprite * content = screen->GetCanvas();
        content->setPivot(0,0);
        canvas->setPivot(0,0);
        content->pushRotated(canvas,0,Drawable::MASK_COLOR);
    }*/
    view3DTest1 = new LuI::View3D();
    LuI::Mesh3D * myMesh3d = new LuI::Mesh3D(&SuperMario003Mesh);
    myMesh3d->Scale(0.9);
    myMesh3d->Rotate({90,-90,0});
    //myMesh3d->Translate({60,0,0});
    view3DTest1->stepCallbackParam=view3DTest1;
    // set a callback when the render is done
    view3DTest1->viewBackgroundColor=ThCol(background);
    view3DTest1->stepCallback=[&](void * obj){
        static float lightXDegree=0;
        LuI::View3D * self=(LuI::View3D *)obj; // recover the view3DTest0
        self->mesh[0]->RelativeRotate({0,-5,0}); // modify the first mesh (PlanetMesh) rotation
        if ( LuI::View3D::RENDER::FULL == self->RenderMode ) {
            lightXDegree+=5;
            if ( lightXDegree > 359 ) { lightXDegree-=360; }
            self->mesh[0]->RotateNormals({0,lightXDegree,0});
        }
        self->dirty=true; // mark control as dirty (forces redraw)
    };
    view3DTest1->AddMesh3D(myMesh3d);
    LuI::Container * mainDiv = new LuI::Container(LuI_Vertical_Layout,2);
    LuI::Container * btnDiv = new LuI::Container(LuI_Horizonal_Layout,5);


    LuI::Button * ShowWireframeFlat=new LuI::Button(LuI_Horizonal_Layout,1);
    ShowWireframeFlat->tapCallbackParam=view3DTest1;
    ShowWireframeFlat->tapCallback=[](void * obj){
        LuI::View3D * view = (LuI::View3D*)obj;
        view->RenderMode=LuI::View3D::RENDER::FLATWIREFRAME;
    };
    LuI::Mesh3D * wireFlatAnimation = new LuI::Mesh3D(&CubeMesh);
    LuI::View3D * wireFlatAnimationView = new LuI::View3D();
    wireFlatAnimation->Scale(0.3);
    wireFlatAnimation->Rotate({float(random(0,359)),float(random(0,359)),float(random(0,359))});
    wireFlatAnimationView->AddMesh3D(wireFlatAnimation);
    wireFlatAnimationView->viewBackgroundColor=ThCol(button);
    wireFlatAnimationView->RenderMode = LuI::View3D::RENDER::FLATWIREFRAME;
    ShowWireframeFlat->AddChild(wireFlatAnimationView);
    btnDiv->AddChild(ShowWireframeFlat);
    wireFlatAnimationView->stepCallbackParam=wireFlatAnimationView;
    wireFlatAnimationView->stepCallback=[&](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj; // recover the view3DTest0
        self->mesh[0]->RelativeRotate({3,5,-2}); // modify the first mesh (PlanetMesh) rotation
        self->dirty=true; // mark control as dirty (forces redraw)
    };
    ShowWireframeFlat->AddChild(wireFlatAnimationView);
    btnDiv->AddChild(ShowWireframeFlat);



    LuI::Button * ShowMask=new LuI::Button(LuI_Horizonal_Layout,1);
    ShowMask->tapCallbackParam=view3DTest1;
    ShowMask->tapCallback=[](void * obj){
        LuI::View3D * view = (LuI::View3D*)obj;
        view->RenderMode=LuI::View3D::RENDER::MASK;
    };
    LuI::Mesh3D * maskAnimation = new LuI::Mesh3D(&CubeMesh);
    LuI::View3D * maskAnimationView = new LuI::View3D();
    maskAnimation->Scale(0.3);
    maskAnimation->Rotate({float(random(0,359)),float(random(0,359)),float(random(0,359))});
    maskAnimationView->AddMesh3D(maskAnimation);
    maskAnimationView->viewBackgroundColor=ThCol(button);
    maskAnimationView->RenderMode = LuI::View3D::RENDER::MASK;
    ShowMask->AddChild(maskAnimationView);
    btnDiv->AddChild(ShowMask);
    maskAnimationView->stepCallbackParam=maskAnimationView;
    maskAnimationView->stepCallback=[&](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj; // recover the view3DTest0
        self->mesh[0]->RelativeRotate({3,5,-2}); // modify the first mesh (PlanetMesh) rotation
        self->dirty=true; // mark control as dirty (forces redraw)
    };
    ShowMask->AddChild(maskAnimationView);
    btnDiv->AddChild(ShowMask);



    LuI::Button * ShowWireframe=new LuI::Button(LuI_Horizonal_Layout,1);
    ShowWireframe->tapCallbackParam=view3DTest1;
    ShowWireframe->tapCallback=[](void * obj){
        LuI::View3D * view = (LuI::View3D*)obj;
        view->RenderMode=LuI::View3D::RENDER::WIREFRAME;
    };
    LuI::Mesh3D * wireAnimation = new LuI::Mesh3D(&CubeMesh);
    LuI::View3D * wireAnimationView = new LuI::View3D();
    wireAnimation->Scale(0.3);
    wireAnimation->Rotate({float(random(0,359)),float(random(0,359)),float(random(0,359))});
    wireAnimationView->AddMesh3D(wireAnimation);
    wireAnimationView->viewBackgroundColor=ThCol(button);
    wireAnimationView->RenderMode = LuI::View3D::RENDER::WIREFRAME;
    ShowWireframe->AddChild(wireAnimationView);
    btnDiv->AddChild(ShowWireframe);
    wireAnimationView->stepCallbackParam=wireAnimationView;
    wireAnimationView->stepCallback=[&](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj; // recover the view3DTest0
        self->mesh[0]->RelativeRotate({3,5,-2}); // modify the first mesh (PlanetMesh) rotation
        self->dirty=true; // mark control as dirty (forces redraw)
    };


    LuI::Button * ShowFlat=new LuI::Button(LuI_Horizonal_Layout,1);
    ShowFlat->tapCallbackParam=view3DTest1;
    ShowFlat->tapCallback=[](void * obj){
        LuI::View3D * view = (LuI::View3D*)obj;
        view->RenderMode=LuI::View3D::RENDER::FLAT;
    };
    LuI::Mesh3D * flatAnimation = new LuI::Mesh3D(&CubeMesh);
    LuI::View3D * flatAnimationView = new LuI::View3D();
    flatAnimation->Scale(0.3);
    flatAnimation->Rotate({float(random(0,359)),float(random(0,359)),float(random(0,359))});
    flatAnimationView->AddMesh3D(flatAnimation);
    flatAnimationView->viewBackgroundColor=ThCol(button);
    flatAnimationView->RenderMode = LuI::View3D::RENDER::FLAT;
    ShowFlat->AddChild(flatAnimationView);
    btnDiv->AddChild(ShowFlat);
    flatAnimationView->stepCallbackParam=flatAnimationView;
    flatAnimationView->stepCallback=[&](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj; // recover the view3DTest0
        self->mesh[0]->RelativeRotate({3,5,-2}); // modify the first mesh (PlanetMesh) rotation
        self->dirty=true; // mark control as dirty (forces redraw)
    };
    ShowFlat->AddChild(flatAnimationView);
    btnDiv->AddChild(ShowFlat);


    LuI::Button * ShowFull=new LuI::Button(LuI_Horizonal_Layout,1);
    ShowFull->tapCallbackParam=view3DTest1;
    ShowFull->tapCallback=[](void * obj){
        LuI::View3D * view = (LuI::View3D*)obj;
        view->RenderMode=LuI::View3D::RENDER::FULL;
    };
    LuI::Mesh3D * fullAnimation = new LuI::Mesh3D(&CubeMesh);
    LuI::View3D * fullAnimationView = new LuI::View3D();
    fullAnimation->Scale(0.3);
    fullAnimation->Rotate({float(random(0,359)),float(random(0,359)),float(random(0,359))});
    fullAnimationView->AddMesh3D(fullAnimation);
    fullAnimationView->viewBackgroundColor=ThCol(button);
    fullAnimationView->RenderMode = LuI::View3D::RENDER::FULL;
    ShowWireframe->AddChild(fullAnimationView);
    btnDiv->AddChild(ShowWireframe);
    fullAnimationView->stepCallbackParam=fullAnimationView;
    fullAnimationView->stepCallback=[&](void * obj){
        LuI::View3D * self=(LuI::View3D *)obj; // recover the view3DTest0
        self->mesh[0]->RelativeRotate({3,5,-2}); // modify the first mesh (PlanetMesh) rotation
        self->dirty=true; // mark control as dirty (forces redraw)
    };
    ShowFull->AddChild(fullAnimationView);
    btnDiv->AddChild(ShowFull);




    mainDiv->border=2;
    mainDiv->AddChild(view3DTest1,1.6);
    mainDiv->AddChild(btnDiv,0.4);
    body->AddChild(mainDiv);


    AddChild(screen);

    directDraw=true; // push to TFT handled by app beyond this point
    lLog("END LOAD\n");
}
/*
bool DebugLuIApplication::Tick() {
    
    if ( millis() > nextTime ) {
        nextTime=millis()+100;
        // can modify the meshes here if you want
    }
    EventHandler();
    return false; // is directDraw application
}*/