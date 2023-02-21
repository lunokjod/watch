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

#include <Arduino.h>
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "ApplicationBase.hpp"

ApplicationBase::ApplicationBase() {
    // Init here any you need
    lAppLog("Hello from ApplicationBase!\n");

    // initalize here widgets
    //mywidget=new .....

    // Remember to dump anything to canvas to get awesome splash screen
    // draw of TFT_eSPI canvas for splash here

    Tick(); // OR call this if no splash 
}

ApplicationBase::~ApplicationBase() {
    // please remove/delete/free all to avoid leaks!
    //delete mywidget;
}

bool ApplicationBase::Tick() {
    // put your interacts here:
    //mywidget->Interact(touched,touchX,touchY);

    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(ThCol(background)); // use theme colors
        TemplateApplication::Tick();
        // draw your interface widgets here!!!
        //mywidget->DrawTo(canvas);

        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
