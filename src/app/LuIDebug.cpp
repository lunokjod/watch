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
#include "../static/fakepdf.c"


DebugLuIApplication::DebugLuIApplication() {
    LuI::Container *screen = new LuI::Container(LuI_Horizonal_Layout,2);

    // draw here!
    screen->border=5;
    LuI::Container * body = new LuI::Container(LuI_Horizonal_Layout,1);
    LuI::Container * foot = new LuI::Container(LuI_Vertical_Layout,1);
    screen->AddChild(body,1.6);
    screen->AddChild(foot,0.4);
    LuI::Buffer * testBuffer = new LuI::Buffer(fakepdf.width,fakepdf.height); //,true,LuI_Vertical_Layout,2);
    TFT_eSprite * buffImg = testBuffer->GetBuffer();
    buffImg->pushImage(0,0,fakepdf.width,fakepdf.height,(uint16_t *)fakepdf.pixel_data);
    //testBuffer->AddChild(new LuI::Check());
    //testBuffer->AddChild(nullptr);

    body->AddChild(testBuffer);
    LuI::Container * footContainer = new LuI::Container(LuI_Vertical_Layout,3);
    foot->AddChild(footContainer);

    LuI::Button * backButton = new LuI::Button(LuI_Vertical_Layout,1);
    backButton->AddChild(new LuI::Text("Back"));
    backButton->tapCallback = [](void * obj){ LaunchWatchface(); };
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
    directDraw=true; // push to TFT beyond this point
}