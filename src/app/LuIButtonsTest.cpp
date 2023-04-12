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
#include "LuIButtonsTest.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/View3D/View3D.hpp"

#include "../resources.hpp"

using namespace LuI;

DebugLuIButtonsApplication::DebugLuIButtonsApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    // fill view with background color
    canvas->fillSprite(ThCol(background)); // use theme colors

    /// create root container with two slots horizontal
    Container * screen = new Container(LuI_Horizontal_Layout,2);
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,2);

    // main view space
    Container * viewContainer = new Container(LuI_Vertical_Layout,2);

    // if use quota, all the slot quotas must sum equal the total number of elements
    // example: default quota in 2 controls result 50/50% of view space with 1.0 of quota everyone (2.0 in total)

    // add main view with quota of 1.7
    screen->AddChild(viewContainer,1.7);
    // add bottom button bar shirnked
    screen->AddChild(bottomButtonContainer,0.3);
    // 1.7 + 0.3 = 2.0 of quota (fine for 2 slots)

    // add back button to dismiss
    Button *backButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
    backButton->tapCallback=[](void * obj){ LaunchWatchface(); }; // callback when tap
    // load icon in XBM format
    XBM * backButtonIcon = new XBM(img_backscreen_24_width,img_backscreen_24_height,img_backscreen_24_bits);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.3);
    bottomButtonContainer->AddChild(nullptr,1.7);

    // creates the two columns of sample buttons
    Container * leftContainer= new Container(LuI_Horizontal_Layout,3);
    viewContainer->AddChild(leftContainer);
    Container * rightContainer= new Container(LuI_Horizontal_Layout,3);
    viewContainer->AddChild(rightContainer);

    // fill viewContainer with different type of buttons
    
    // left view side <-----------------------------------

    // -----> basic empty button
    //Button *basicButton = new Button(); // a basic default button
    //leftContainer->AddChild(basicButton);

    // -----> text button
    Button *textButton = new Button(); // a basic default button
    textButton->AddChild(new Text("Hello"));
    leftContainer->AddChild(textButton);

    // -----> text button with font
    Button *textButtonAlt = new Button(); // a basic default button
    textButtonAlt->AddChild(new Text("Yep!",TFT_YELLOW,false,1,&FreeSerifItalic12pt7b));
    leftContainer->AddChild(textButtonAlt);

    // -----> xbm 1 bit image button
    Button *XBMButton = new Button();
    XBMButton->AddChild(new XBM(img_xbm_32_width,img_xbm_32_height,img_xbm_32_bits));
    leftContainer->AddChild(XBMButton);


    // right view side ----------------------------------->

    // -----> 16 bit image button with mask color
    Button *ImageButton = new Button();
    ImageButton->AddChild(new Image(img_house_32.width,img_house_32.height,img_house_32.pixel_data,TFT_GREEN));
    rightContainer->AddChild(ImageButton);

    // -----> using Container properties of Button to add two controls
    Button *CombinedButton = new Button(LuI_Vertical_Layout,2);
    CombinedButton->AddChild(new Image(img_house_32.width,img_house_32.height,img_house_32.pixel_data,TFT_GREEN),0.8);
    CombinedButton->AddChild(new Text("Home"),1.2);
    rightContainer->AddChild(CombinedButton);


    // -----> can compose with multiple other components, any control can be added
    Button *Button3d = new Button(LuI_Vertical_Layout,2);
    Mesh3D * sparkles = new Mesh3D(&BackButtonMesh); // load 3D mesh
    View3D * sparklesView = new View3D(); // create view for 3D mesh
    sparklesView->SetGlobalScale(0.15);  // mesh methers=pixels 240Meters=240pixels
    sparkles->Scale(0.15);
    sparklesView->SetGlobalRotation({float(random(0,360)),float(random(0,360)),float(random(0,360))}); // add some random
    sparklesView->SetBackgroundColor(ThCol(button));
    sparklesView->RenderMode = View3D::RENDER::FLAT; // set rendering to color cheaper-cpu one
    sparklesView->stepCallbackParam=sparklesView; // pass view to callback
    // *** see the coments in the bottom of this constructor: "sparklesView->dirty=true;"
    sparklesView->stepCallback = [](void * obj){ // callback when refresh is done
        View3D * self = (View3D *)obj;
        // want rotate the view
        Angle3D currentRot = self->GetGlobalRotation();
        currentRot.x+=5;
        currentRot.y+=5;
        currentRot.z+=5;
        self->SetGlobalRotation(currentRot);
        self->dirty=true;
    };
    sparklesView->AddMesh3D(sparkles);
    Button3d->AddChild(sparklesView,0.8);
    Button3d->AddChild(new Text("Home"),1.2);
    rightContainer->AddChild(Button3d);

    AddChild(screen); // add root to view
    sparklesView->dirty=true; // why this?
    /* this is related with "step callback";
     * let me explain about "dirty" flag,
     * after AddChild call all GUI elements are updated and refreshed (don't need to refresh again)
     * this means, no elements refreshed again until someone is marked as dirty
     * the "sparklesView" uses "stepCallback" call, this means was called AFTER render
     * and the contents of callback updates the view Global rotation coordinates (marks as dirty)
     * creating a infinite loop, when the render is done, the mesh is updated, marked as dirty an called again
     * and mesh rotates infinitelly
     */
    directDraw=true; // allow controls to direct redraw itself instead of push whole view Sprite
    // Thats all! the app is running
}
