#include <Arduino.h>
#include "LogView.hpp"
#include "../UI/AppTemplate.hpp"
#include "ApplicationBase.hpp"

ApplicationBase::ApplicationBase() {
    // Init here any you need

    // Remember to dump anything to canvas to get splash screen
}

ApplicationBase::~ApplicationBase() {
    // please remove/delete/free all to avoid leaks!
}

bool ApplicationBase::Tick() {
    bool interacted = TemplateApplication::Tick();
    if ( interacted ) { return true; }
    
    // put your interacts here

    
    if ( millis() > nextRefresh ) {
        
        
        // draw your interface here!!!


        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
