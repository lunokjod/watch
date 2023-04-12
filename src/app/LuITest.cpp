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
#include "LuITest.hpp"
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

LuITestApplication::LuITestApplication() {
    lAppLog("BEGIN CONSTRUCTOR HERE\n");
    canvas->fillSprite(ThCol(background));
    Container * screen = new Container(LuI_Horizontal_Layout,3); 
    Container * upContainer = new Container(LuI_Vertical_Layout,2); 
    Container * shit = new Container(LuI_Vertical_Layout,2);
    Container * shit2 = new Container(LuI_Horizontal_Layout,2);

    duc = new Text("World",TFT_SKYBLUE);
    duc->SetBackgroundColor(ThCol(background));

    upText = new Text("Hello\n",TFT_ORANGE);
    upText->SetBackgroundColor(ThCol(background));

    Button *testButtonUp = new Button();
    testButtonUp->AddChild(new Text("Alternate",TFT_ORANGE));
    testButtonUp->tapCallbackParam=this;
    testButtonUp->tapCallback=[](void *obj) {
        lLog("BUTTON UP\n");
        LuITestApplication * self=(LuITestApplication*)obj;
        static bool what=false;
        what=(!what);
        if ( what ) {
            self->upText->SetText("Hello");
            self->duc->SetText("World");
        }
        else {
            self->upText->SetText("I'm");
            self->duc->SetText("LuI");
        }
    };
    upContainer->AddChild(testButtonUp);
    upContainer->AddChild(upText);
    screen->AddChild(upContainer);
    shit2->AddChild(new Check());
    Button *testButton = new Button();
    testButton->AddChild(new Text("Hello world!"));

    Mesh3D * meshTest = new Mesh3D(&CubeMesh);
    view3DTest = new View3D();
    view3DTest->SetBackgroundColor(ThCol(background));
    view3DTest->AddMesh3D(meshTest);
    testButton->tapCallbackParam=this;
    testButton->tapCallback=[](void *obj) {
        LuITestApplication * self=(LuITestApplication *)obj;
        self->view3DTest->mesh[0]->RelativeRotate({0,10,0});
        self->view3DTest->mesh[0]->dirty=true;
        Angle3D currentRot = self->view3DTest->GetGlobalRotation();
        currentRot.y+=10;
        self->view3DTest->SetGlobalRotation(currentRot);
        self->view3DTest->dirty=true;
        lLog("PUSH THE BUTTOn!\n");
    };
    shit2->AddChild(testButton);
    shit->AddChild(shit2);
    shit->AddChild(duc);
    screen->AddChild(shit);

    screen->AddChild(view3DTest);

    AddChild(screen);
    directDraw=true;
    lAppLog("END CONSTRUCTOR HERE\n");
}

LuITestApplication::~LuITestApplication() {

}
