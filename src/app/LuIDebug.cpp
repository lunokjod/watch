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
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/View3D/Mesh3D.hpp"
#include "../UI/controls/View3D/View3D.hpp"

#include "../resources.hpp"

DebugLuIApplication::DebugLuIApplication() {
    directDraw=false; // buffer start
    // defines the root container
    //LuI::Container *screen = new LuI::Container(LuI_Horizontal_Layout,1);

    // defines the body
    LuI::Container * body = new LuI::Container(LuI_Vertical_Layout,1);
    //body->border=5;
    //LuI::Container * foot = new LuI::Container(LuI_Vertical_Layout,1);

    // init
    canvas->fillSprite(ThCol(background)); // use theme colors

    LuI::Container * mainDiv = new LuI::Container(LuI_Vertical_Layout,3);
    LuI::Container * btnPanelRight = new LuI::Container(LuI_Horizontal_Layout,5);
    LuI::Container * btnPanelLeft = new LuI::Container(LuI_Horizontal_Layout,5);
    LuI::Container * myTopButtonSet = new LuI::Container(LuI_Vertical_Layout,4);
    LuI::Container * myBottomButtonSet = new LuI::Container(LuI_Vertical_Layout,4);

    LuI::Container * centerDiv = new LuI::Container(LuI_Horizontal_Layout,3);

    view3DTest1 = new LuI::View3D();
    LuI::Mesh3D * myMesh3d = new LuI::Mesh3D(&CubeMesh);
    LuI::Mesh3D * myMesh3d2 = new LuI::Mesh3D(&CubeMesh);
    LuI::Mesh3D * myMesh3d3 = new LuI::Mesh3D(&CubeMesh);
    myMesh3d->Scale(0.5);    
    myMesh3d2->Translate({60,0,0});
    myMesh3d2->Scale(0.1);
    myMesh3d3->Translate({-60,0,0});
    view3DTest1->SetGlobalLocation({ 60,0,0 });
    //view3DTest1->SetGlobalRotation({ 0,0,0 });
    view3DTest1->SetGlobalScale({ 0.2,0.2,0.2 });
    view3DTest1->SetBackgroundColor(TFT_BLACK); //ThCol(background);
    view3DTest1->stepCallbackParam=view3DTest1;
    view3DTest1->stepCallback=[&](void * obj){  // called when data refresh is done (before render)
        static float rotationDeg=359;
        static float scale=0.1;
        LuI::View3D * self=(LuI::View3D *)obj; // recover the view3DTest0

        scale+=0.05;
        if ( scale > 1.5 ) { scale = 0.1; }
        self->mesh[1]->Scale(scale);
        rotationDeg-=10;
        if ( rotationDeg > 359 ) { rotationDeg-=360; }
        else if ( rotationDeg < 0 ) { rotationDeg+=360; }
        if ( followCamera ) {
            self->SetGlobalRotation({0,rotationDeg,0});
            view3DTest1->SetGlobalLocation({ 60,0,0 });
        } else {
            self->SetGlobalRotation({0,0,0});
            view3DTest1->SetGlobalLocation({ 0,0,0 });
        }
        self->mesh[2]->Rotate({0,rotationDeg*-1,0});

        if ((showLights) && ( LuI::View3D::RENDER::FULL == self->RenderMode )) {
            self->mesh[0]->RotateNormals({0,rotationDeg,0});
            self->mesh[1]->RotateNormals({0,rotationDeg,0});
            self->mesh[2]->RotateNormals({0,rotationDeg,0});
        }
        //lLog("ROT: %f\n",rotationDeg);
        self->dirty=true; // mark control as dirty (forces redraw)
    };
    view3DTest1->AddMesh3D(myMesh3d);
    view3DTest1->AddMesh3D(myMesh3d2);
    view3DTest1->AddMesh3D(myMesh3d3);
    


    LuI::Button * backButton=new LuI::Button(LuI_Horizontal_Layout,1);
    backButton->tapCallback=[](void * obj){ LaunchWatchface(); };
    LuI::XBM * backIcon = new LuI::XBM(img_back_16_width,img_back_16_height,img_back_16_bits);
    backButton->AddChild(backIcon);
    myTopButtonSet->AddChild(backButton);


    LuI::Button * CameraButton=new LuI::Button(LuI_Horizontal_Layout,1);
    CameraButton->tapCallbackParam=this;
    CameraButton->tapCallback=[](void * obj){
        DebugLuIApplication * view = (DebugLuIApplication*)obj;
        view->FollowCamera();
    };
    LuI::Image * cameraIcon = new LuI::Image(img_camera_18.width,img_camera_18.height,img_camera_18.pixel_data);
    CameraButton->AddChild(cameraIcon);
    myTopButtonSet->AddChild(CameraButton);


    LuI::Button * fixedCameraButton=new LuI::Button(LuI_Horizontal_Layout,1);
    fixedCameraButton->tapCallbackParam=this;
    fixedCameraButton->tapCallback=[](void * obj){
        DebugLuIApplication * view = (DebugLuIApplication*)obj;
        view->FixedCamera();
    };
    LuI::Image * landscapeIcon = new LuI::Image(img_landscape_18.width,img_landscape_18.height,img_landscape_18.pixel_data);
    fixedCameraButton->AddChild(landscapeIcon);
    myTopButtonSet->AddChild(fixedCameraButton);

    LuI::Button * ShowLightButton=new LuI::Button(LuI_Horizontal_Layout,1);
    ShowLightButton->tapCallbackParam=this;
    ShowLightButton->tapCallback=[](void * obj){
        DebugLuIApplication * view = (DebugLuIApplication*)obj;
        view->SwitchLights();
    };
    LuI::Image * lightsIcon = new LuI::Image(img_lightbulb_18.width,img_lightbulb_18.height,img_lightbulb_18.pixel_data);
    ShowLightButton->AddChild(lightsIcon);
    myTopButtonSet->AddChild(ShowLightButton);




    LuI::Button * ShowWireframeFlat=new LuI::Button(LuI_Horizontal_Layout,1);
    ShowWireframeFlat->tapCallbackParam=view3DTest1;
    ShowWireframeFlat->tapCallback=[](void * obj){
        LuI::View3D * view = (LuI::View3D*)obj;
        view->RenderMode=LuI::View3D::RENDER::FLATWIREFRAME;
        view->Render();
    };
    LuI::Image * wireIcon = new LuI::Image(img_wireframe_18.width,img_wireframe_18.height,img_wireframe_18.pixel_data);
    ShowWireframeFlat->AddChild(wireIcon);
    btnPanelLeft->AddChild(ShowWireframeFlat);


    LuI::Button * ShowMask=new LuI::Button(LuI_Horizontal_Layout,1);
    ShowMask->tapCallbackParam=view3DTest1;
    ShowMask->tapCallback=[](void * obj){
        LuI::View3D * view = (LuI::View3D*)obj;
        view->RenderMode=LuI::View3D::RENDER::MASK;
        view->Render();
    };

    LuI::Image * alphaIcon = new LuI::Image(img_alpha_18.width,img_alpha_18.height,img_alpha_18.pixel_data);
    ShowMask->AddChild(alphaIcon);
    btnPanelLeft->AddChild(ShowMask);

    LuI::Button * ShowWireframe=new LuI::Button(LuI_Horizontal_Layout,1);
    ShowWireframe->tapCallbackParam=view3DTest1;
    ShowWireframe->tapCallback=[](void * obj){
        LuI::View3D * view = (LuI::View3D*)obj;
        view->RenderMode=LuI::View3D::RENDER::WIREFRAME;
        view->Render();
    };
    LuI::Image * wireColorIcon = new LuI::Image(img_wirecolor_18.width,img_wirecolor_18.height,img_wirecolor_18.pixel_data);
    ShowWireframe->AddChild(wireColorIcon);
    btnPanelLeft->AddChild(ShowWireframe);

    LuI::Button * ShowFlat=new LuI::Button(LuI_Horizontal_Layout,1);
    ShowFlat->tapCallbackParam=view3DTest1;
    ShowFlat->tapCallback=[](void * obj){
        LuI::View3D * view = (LuI::View3D*)obj;
        view->RenderMode=LuI::View3D::RENDER::FLAT;
        view->Render();
    };
    LuI::Image * flatColorIcon = new LuI::Image(img_flatcolor_18.width,img_flatcolor_18.height,img_flatcolor_18.pixel_data);
    ShowFlat->AddChild(flatColorIcon);
    btnPanelLeft->AddChild(ShowFlat);

    LuI::Button * ShowFull=new LuI::Button(LuI_Horizontal_Layout,1);
    ShowFull->tapCallbackParam=view3DTest1;
    ShowFull->tapCallback=[](void * obj){
        LuI::View3D * view = (LuI::View3D*)obj;
        view->RenderMode=LuI::View3D::RENDER::FULL;
        view->Render();
    };

    LuI::Image * fullrenderIcon = new LuI::Image(img_fullrender_18.width,img_fullrender_18.height,img_fullrender_18.pixel_data);
    ShowFull->AddChild(fullrenderIcon);
    btnPanelLeft->AddChild(ShowFull);


    //mainDiv->border=5;
    centerDiv->AddChild(myTopButtonSet,0.55);
    centerDiv->AddChild(view3DTest1,2.45);
    centerDiv->AddChild(myBottomButtonSet,0.0);

    mainDiv->AddChild(btnPanelRight,0.0); // dyslexia test x'D
    mainDiv->AddChild(centerDiv,2.45);
    mainDiv->AddChild(btnPanelLeft,0.55);
    body->AddChild(mainDiv);

    //screen->AddChild(body);
    AddChild(body);
    // refresh view to first render splash

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


void DebugLuIApplication::FollowCamera() {
    followCamera=true;
}

void DebugLuIApplication::FixedCamera() {
    followCamera=false;
}

void DebugLuIApplication::SwitchLights() {
    showLights=(!showLights);

}