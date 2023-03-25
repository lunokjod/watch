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
//#include "../../tool/Cube.c"
//#include "../../tool/glass.c"
//#include "../../tool/Sphere.c"
#include "../../tool/Planet.c"
//#include "../../tool/Planet001.c"
//#include "../../tool/robot001.c"
//#include "../../tool/bowser001.c"
//#include "../../tool/bowser002.c"
#include "../../tool/SuperMario.c"
//#include "../../tool/Text004.c"

DebugLuIApplication::DebugLuIApplication() {
    directDraw=false; // buffer start
    LuI::Container *screen = new LuI::Container(LuI_Horizonal_Layout,2);
    LuI::Container * body = new LuI::Container(LuI_Vertical_Layout,1);
    body->border=5;
    LuI::Container * foot = new LuI::Container(LuI_Vertical_Layout,1);
    screen->AddChild(body,1.6);
    screen->AddChild(foot,0.4);

    LuI::Container * footContainer = new LuI::Container(LuI_Vertical_Layout,2);
    foot->AddChild(footContainer);

    // want button with 2 slots in vertical layout (50/50)
    LuI::Button * backButton = new LuI::Button(LuI_Vertical_Layout,2);
    // add border between slots
    backButton->border=2;
    // load mesh from blender python script .c generated
    LuI::Mesh3D * backAnimation = new LuI::Mesh3D(&PlanetMesh);
    // create a view for backAnimation
    view3DTest0 = new LuI::View3D();
    // small please!
    backAnimation->Scale({0.15,0.15,0.15});
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
    // add plain text to the next slot
    backButton->AddChild(new LuI::Text("Back"));
    // what to do when tap?
    backButton->tapCallback = [](void * obj){ LaunchWatchface(); };
    // add button to before defined container
    footContainer->AddChild(backButton);

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
    directDraw=true; // push to TFT handled by app beyond this point
}

bool DebugLuIApplication::Tick() {
    /*
    if ( millis() > nextTime ) {
        nextTime=millis()+100;
        // can modify the meshes here if you want
    }*/
    EventHandler();
    return false; // is directDraw application
}