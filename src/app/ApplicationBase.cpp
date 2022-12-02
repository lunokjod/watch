#include <Arduino.h>
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "ApplicationBase.hpp"

ApplicationBase::ApplicationBase() {
    // Init here any you need
    lAppLog("Hello from ApplicationBase!\n");

    canvas->fillSprite(ThCol(background)); // use theme colors

    // Remember to dump anything to canvas to get awesome splash screen
    // draw of TFT_eSPI canvas for splash here

    Tick(); // OR call this if no splash 
}

ApplicationBase::~ApplicationBase() {
    // please remove/delete/free all to avoid leaks!
}

bool ApplicationBase::Tick() {
    bool interacted = TemplateApplication::Tick(); // calls to TemplateApplication::Tick()
    if ( interacted ) { return true; } // back button pressed
    
    // put your interacts here:
    //mywidget->Interact(touched,touchX,touchY);

    if ( millis() > nextRefresh ) {
        lAppLog("Loop in ApplicationBase!\n");
        canvas->fillSprite(ThCol(background)); // use theme colors

        // draw your interface widgets here!!!
        //mywidget->DrawTo(canvas);
        TemplateApplication::Tick(); // redraw back button

        nextRefresh=millis()+(1000/12); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
