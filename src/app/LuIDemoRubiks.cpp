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

#include "../UI/AppLuITemplate.hpp"
#include "LuIDemoRubiks.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/Check.hpp"
#include "../UI/controls/IconMenu.hpp"
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/View3D/View3D.hpp"
#include "../UI/controls/Buffer.hpp"
#include "LogView.hpp"

#include "../UI/UI.hpp"
#include <LilyGoWatch.h>
using namespace LuI;
#include "../resources.hpp"

extern TFT_eSPI * tft;

LuIExperimentRubiksApplication::LuIExperimentRubiksApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    // fill view with background color
    canvas->fillSprite(TFT_BLACK); // use theme colors

    /// create root container with two slots horizontal
    Container * screen = new Container(LuI_Horizontal_Layout,2);
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,2);

    // main view space
    Container * viewContainer = new Container(LuI_Horizontal_Layout,1);
    //viewContainer->border=10;
    // if use quota, all the slot quotas must sum equal the total number of elements
    // example: default quota in 2 controls result 50/50% of view space with 1.0 of quota everyone (2.0 in total)

    screen->AddChild(viewContainer,1.68);
    // add bottom button bar shirnked
    screen->AddChild(bottomButtonContainer,0.32);

    // add back button to dismiss
    Button *backButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
    backButton->border=10;
    backButton->tapCallback=[](void * obj){ LaunchWatchface(); }; // callback when tap
    // load icon in XBM format
    XBM * backButtonIcon = new XBM(img_backscreen_24_width,img_backscreen_24_height,img_backscreen_24_bits);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.35);
    bottomButtonContainer->AddChild(nullptr,1.65);

    // here the main view
    LuI::View3D * view3DTest1 = new LuI::View3D();
    view3DTest1->RenderMode=LuI::View3D::RENDER::FULL; // only will show bilboards, not meshes

    float cubeSize=38;
    // fake center cube on 0,0,0
    /**
    LuI::Mesh3D * cube0 = new LuI::Mesh3D(&CubeMesh);
    cube0->Translate({0,0,0});
    view3DTest1->AddMesh3D(cube0);
    */
// cross
    LuI::Mesh3D * cube1 = new LuI::Mesh3D(&CubeMesh);
    cube1->Translate({cubeSize,0,0});
    view3DTest1->AddMesh3D(cube1);

    LuI::Mesh3D * cube2 = new LuI::Mesh3D(&CubeMesh);
    cube2->Translate({cubeSize*-1,0,0});
    view3DTest1->AddMesh3D(cube2);

    LuI::Mesh3D * cube3 = new LuI::Mesh3D(&CubeMesh);
    cube3->Translate({0,cubeSize,0});
    view3DTest1->AddMesh3D(cube3);

    LuI::Mesh3D * cube4 = new LuI::Mesh3D(&CubeMesh);
    cube4->Translate({0,cubeSize*-1,0});
    view3DTest1->AddMesh3D(cube4);

//center ring
    LuI::Mesh3D * cube5 = new LuI::Mesh3D(&CubeMesh);
    cube5->Translate({0,0,cubeSize});
    view3DTest1->AddMesh3D(cube5);

    LuI::Mesh3D * cube6 = new LuI::Mesh3D(&CubeMesh);
    cube6->Translate({0,0,cubeSize*-1});
    view3DTest1->AddMesh3D(cube6);

    LuI::Mesh3D * cube7 = new LuI::Mesh3D(&CubeMesh);
    cube7->Translate({cubeSize,0,cubeSize*-1});
    view3DTest1->AddMesh3D(cube7);

    LuI::Mesh3D * cube8 = new LuI::Mesh3D(&CubeMesh);
    cube8->Translate({cubeSize,0,cubeSize});
    view3DTest1->AddMesh3D(cube8);

    LuI::Mesh3D * cube9 = new LuI::Mesh3D(&CubeMesh);
    cube9->Translate({cubeSize*-1,0,cubeSize*-1});
    view3DTest1->AddMesh3D(cube9);

    LuI::Mesh3D * cube10 = new LuI::Mesh3D(&CubeMesh);
    cube10->Translate({cubeSize*-1,0,cubeSize});
    view3DTest1->AddMesh3D(cube10);

// down

    LuI::Mesh3D * cube11 = new LuI::Mesh3D(&CubeMesh);
    cube11->Translate({0,cubeSize,cubeSize});
    view3DTest1->AddMesh3D(cube11);

    LuI::Mesh3D * cube12 = new LuI::Mesh3D(&CubeMesh);
    cube12->Translate({0,cubeSize,cubeSize*-1});
    view3DTest1->AddMesh3D(cube12);

    LuI::Mesh3D * cube13 = new LuI::Mesh3D(&CubeMesh);
    cube13->Translate({cubeSize,cubeSize,cubeSize*-1});
    view3DTest1->AddMesh3D(cube13);

    LuI::Mesh3D * cube14 = new LuI::Mesh3D(&CubeMesh);
    cube14->Translate({cubeSize,cubeSize,cubeSize});
    view3DTest1->AddMesh3D(cube14);

    LuI::Mesh3D * cube15 = new LuI::Mesh3D(&CubeMesh);
    cube15->Translate({cubeSize*-1,cubeSize,cubeSize*-1});
    view3DTest1->AddMesh3D(cube15);

    LuI::Mesh3D * cube16 = new LuI::Mesh3D(&CubeMesh);
    cube16->Translate({cubeSize*-1,cubeSize,cubeSize});
    view3DTest1->AddMesh3D(cube16);

    // taps
    LuI::Mesh3D * cube17 = new LuI::Mesh3D(&CubeMesh);
    cube17->Translate({cubeSize,cubeSize,0});
    view3DTest1->AddMesh3D(cube17);

    LuI::Mesh3D * cube18 = new LuI::Mesh3D(&CubeMesh);
    cube18->Translate({cubeSize*-1,cubeSize,0});
    view3DTest1->AddMesh3D(cube18);

// up

    LuI::Mesh3D * cube19 = new LuI::Mesh3D(&CubeMesh);
    cube19->Translate({0,cubeSize*-1,cubeSize});
    view3DTest1->AddMesh3D(cube19);
    
    LuI::Mesh3D * cube20 = new LuI::Mesh3D(&CubeMesh);
    cube20->Translate({0,cubeSize*-1,cubeSize*-1});
    view3DTest1->AddMesh3D(cube20);

    LuI::Mesh3D * cube21 = new LuI::Mesh3D(&CubeMesh);
    cube21->Translate({cubeSize,cubeSize*-1,cubeSize*-1});
    view3DTest1->AddMesh3D(cube21);



    LuI::Mesh3D * cube22 = new LuI::Mesh3D(&CubeMesh);
    cube22->Translate({cubeSize,cubeSize*-1,cubeSize});
    view3DTest1->AddMesh3D(cube22);

    LuI::Mesh3D * cube23 = new LuI::Mesh3D(&CubeMesh);
    cube23->Translate({cubeSize*-1,cubeSize*-1,cubeSize*-1});
    view3DTest1->AddMesh3D(cube23);

    LuI::Mesh3D * cube24 = new LuI::Mesh3D(&CubeMesh);
    cube24->Translate({cubeSize*-1,cubeSize*-1,cubeSize});
    view3DTest1->AddMesh3D(cube24);

    // taps
    LuI::Mesh3D * cube25 = new LuI::Mesh3D(&CubeMesh);
    cube25->Translate({cubeSize,cubeSize*-1,0});
    view3DTest1->AddMesh3D(cube25);

    LuI::Mesh3D * cube26 = new LuI::Mesh3D(&CubeMesh);
    cube26->Translate({cubeSize*-1,cubeSize*-1,0});
    view3DTest1->AddMesh3D(cube26);


    view3DTest1->SetGlobalLocation({ 0,0,0 });
    view3DTest1->SetGlobalScale(0.025);
    view3DTest1->SetBackgroundColor(TFT_BLACK); //ThCol(background);
    view3DTest1->stepCallbackParam=view3DTest1;
    view3DTest1->stepCallback=[&](void * obj){  // called when data refresh is done (before render)
        LuI::View3D * self=(LuI::View3D *)obj;
        static int rotationDeg=359;
        static float scale=0.1;
        static float scaleIncrement=0.05;
        scale+=scaleIncrement;
        if ( scale > 3.5 ) { scaleIncrement*=-1; }
        else if ( scale < 0.1 ) { scaleIncrement*=-1; }
        rotationDeg-=5;
        if ( rotationDeg > 359 ) { rotationDeg-=360; }
        else if ( rotationDeg < 0 ) { rotationDeg+=360; }
        self->SetGlobalRotation({0,float(rotationDeg),0});
        //self->mesh[0]->RelativeRotate({0,3.3,0});
        //self->mesh[1]->RelativeRotate({3.3,0,0});
        //self->mesh[2]->RelativeRotate({0,0,3.3});
        self->dirty=true; // mark control as dirty (forces redraw)
    };
    /* no background
    view3DTest1->beforeRenderCallbackParam=view3DTest1;
    view3DTest1->beforeRenderCallback=[&](void * obj, void *canvas){  // called when data refresh is done (before render)
        LuI::View3D * self=(LuI::View3D *)obj; // recover the view3DTest0
        TFT_eSprite * myView=(TFT_eSprite *)canvas; // recover the view3DTest0
        myView->setSwapBytes(true);
        myView->pushImage(0,0,img_brokenMoon.width,img_brokenMoon.height,(const uint16_t *)img_brokenMoon.pixel_data);
        myView->setSwapBytes(false);
    };
    */

    /* no post-render
    view3DTest1->renderCallbackParam=view3DTest1;
    view3DTest1->renderCallback=[&](void * obj, void *canvas) {
        LuI::View3D * self=(LuI::View3D *)obj; // recover the view3DTest0
        TFT_eSprite * myView=(TFT_eSprite *)canvas; // recover the view3DTest0
        static int16_t pixelSize = 1;
        static int16_t incrementPixel=1;
        pixelSize+=incrementPixel;
        if ( pixelSize >= 12 ) { incrementPixel*=-1; }
        else if ( pixelSize <= 1 ) { incrementPixel*=-1; }
        for(int y=0;y<myView->height();y+=pixelSize) {
            for(int x=0;x<myView->width();x+=pixelSize) {
                uint16_t color = myView->readPixel(x,y);
                for(int cy=0;cy<pixelSize;cy++){
                    int32_t fy=y+cy;
                    if ( fy >= myView->height() ) { break; }
                    for(int cx=0;cx<pixelSize;cx++){
                        int32_t fx=x+cx;
                        if ( fx >= myView->width() ) { break; }
                        int16_t colorN = myView->readPixel(fx,fy);
                        color = tft->alphaBlend(128,color,colorN);
                    }
                }
                myView->fillRect(x,y,pixelSize,pixelSize,ColorSwap(color));
            }
        }
    };*/


    viewContainer->AddChild(view3DTest1);

    AddChild(screen);
    directDraw=true; // allow controls to direct redraw itself instead of push whole view Sprite
    // Thats all! the app is running
}
