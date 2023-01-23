#include <Arduino.h>
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "ApplicationBase.hpp"

ApplicationBase::ApplicationBase() {
    // Init here any you need
    lAppLog("Hello from ApplicationBase!\n");

    // Remember to dump anything to canvas to get awesome splash screen
    // draw of TFT_eSPI canvas for splash here

    Tick(); // OR call this if no splash 
}

ApplicationBase::~ApplicationBase() {
    // please remove/delete/free all to avoid leaks!
}

bool ApplicationBase::Tick() {
    bool interacted = btnBack->Interact(touched,touchX,touchY); // this special button becomes from TemplateApplication
    if ( interacted ) { return true; } // back button pressed
    
    // put your interacts here:
    //mywidget->Interact(touched,touchX,touchY);

    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(ThCol(background)); // use theme colors

        // draw your interface widgets here!!!
        //mywidget->DrawTo(canvas);

        btnBack->DrawTo(canvas); // redraw back button
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
