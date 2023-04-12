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
#include "LuIDemoMask.hpp"
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
#include "../../static/img_background_panda.c"
#include "../../static/img_background_toon.c"

extern TFT_eSPI * tft;

LuIExperimentMaskApplication::LuIExperimentMaskApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    // fill view with background color
    canvas->fillSprite(TFT_BLACK); // use theme colors

    background0 = new TFT_eSprite(tft);
    background0->setColorDepth(16);
    background0->createSprite(img_background_panda.width,img_background_panda.height);
    background0->setSwapBytes(true);
    background0->pushImage(0,0,img_background_panda.width,img_background_panda.height,(uint16_t *)img_background_panda.pixel_data);

    background1 = new TFT_eSprite(tft);
    background1->setColorDepth(16);
    background1->createSprite(img_background_toon.width,img_background_toon.height);
    background1->setSwapBytes(true);
    background1->pushImage(0,0,img_background_toon.width,img_background_toon.height,(uint16_t *)img_background_toon.pixel_data);

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
    backButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
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
    view3DTest1->MaskColor=TFT_PINK;
    view3DTest1->RenderMode=LuI::View3D::RENDER::MASK; // only will show bilboards, not meshes

    LuI::Mesh3D * cube0 = new LuI::Mesh3D(&CubeMesh);
    cube0->Translate({0,0,0});
    view3DTest1->AddMesh3D(cube0);



    view3DTest1->SetGlobalLocation({ 0,0,0 });
    view3DTest1->SetGlobalScale(0.5);
    view3DTest1->SetBackgroundColor(TFT_BLACK); //ThCol(background);
    view3DTest1->stepCallbackParam=view3DTest1;
    view3DTest1->stepCallback=[&](void * obj){  // called when data refresh is done (before render)
        LuI::View3D * self=(LuI::View3D *)obj;
        static int rotationDeg=359;
        static float scale=0.1;
        static float scaleIncrement=0.30;
        scale+=scaleIncrement;
        if ( scale > 7.0 ) { scaleIncrement*=-1; }
        else if ( scale < 0.1 ) { scaleIncrement*=-1; }
        rotationDeg-=8;
        if ( rotationDeg > 359 ) { rotationDeg-=360; }
        else if ( rotationDeg < 0 ) { rotationDeg+=360; }
        self->SetGlobalRotation({0,float(rotationDeg),0});
        self->mesh[0]->Scale(scale);
        
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

    /* post-render pixelation
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
    };
    */

    //view3DTest1->renderCallbackParam=view3DTest1;
    view3DTest1->renderCallback=[&](void * unused, void *canvas) {
        TFT_eSprite * myView=(TFT_eSprite *)canvas; // obtain the current frame
        for(int y=0;y<myView->height();y++) {
            for(int x=0;x<myView->width();x++) {
                uint16_t color = myView->readPixel(x,y);
                uint16_t finalColor;
                if ( TFT_PINK == color ) { finalColor = background0->readPixel(x,y); }
                else { finalColor = background1->readPixel(x,y); }
                myView->drawPixel(x,y,finalColor);
            }
        }
    };


    viewContainer->AddChild(view3DTest1);

    AddChild(screen);
    directDraw=true; // allow controls to direct redraw itself instead of push whole view Sprite
    // Thats all! the app is running
}


LuIExperimentMaskApplication::~LuIExperimentMaskApplication() {
    if ( nullptr != background0 ) { 
        background0->deleteSprite();
        delete background0;
    }
    if ( nullptr != background1 ) { 
        background1->deleteSprite();
        delete background1;
    }
}