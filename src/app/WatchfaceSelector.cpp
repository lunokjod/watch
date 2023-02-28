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
#include <ArduinoNvs.h>

#include "../system/Application.hpp"
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "WatchfaceSelector.hpp"
#include "../lunokIoT.hpp"

WatchfaceSelectorApplication::WatchfaceSelectorApplication() {
    //    selectedWatchFace=NVS.getInt("UserWatchface");
    watchfacesCaptures = WatchFacesAvailiable();
    watchfaceCapture = (TFT_eSprite**)ps_calloc(watchfacesCaptures,sizeof(TFT_eSprite**));
    for(size_t off=0;off<watchfacesCaptures;off++) {
        LunokIoTApplication *inst=GetWatchFaceFromOffset(off);
        lAppLog("Rendering Watchface (%u): %p '%s'...\n", off, inst, inst->AppName());
        inst->Tick();
        TFT_eSprite *capture = ScaleSprite(inst->canvas,0.25);
        delete inst;
        watchfaceCapture[off]=capture;
    }
    SwitchUseOnAlwaysOn = new SwitchWidget(170,180);
    bool useAlwaysOn = NVS.getInt("AOnWaface");
    SwitchUseOnAlwaysOn->switchEnabled=useAlwaysOn;
    SwitchUseOnAlwaysOn->InternalRedraw();
    mySelection = LoT().selectedWatchFace;
    Tick(); // OR call this if no splash 
}

WatchfaceSelectorApplication::~WatchfaceSelectorApplication() {
    if ( nullptr != watchfaceCapture ) {
        for(size_t off=0;off<watchfacesCaptures;off++) {
            if ( nullptr != watchfaceCapture[off] ) {
                watchfaceCapture[off]->deleteSprite();
                free(watchfaceCapture[off]);
                watchfaceCapture[off]=nullptr;
            }
        }
        free(watchfaceCapture);
    }
    if ( nullptr != SwitchUseOnAlwaysOn ) {
        NVS.setInt("AOnWaface",SwitchUseOnAlwaysOn->switchEnabled,false);
        delete SwitchUseOnAlwaysOn;
    }
    //if ( mySelection != LoT().selectedWatchFace ) {
    NVS.setInt("UWatchF",mySelection,false);
    //}
}

bool WatchfaceSelectorApplication::Tick() {
    TemplateApplication::btnBack->Interact(touched,touchX,touchY);
    SwitchUseOnAlwaysOn->Interact(touched,touchX,touchY);
    if ( touched ) {
        int cY = touchY/80;
        if ( touchY < 180 ) { // react only on upper screen
            int cX = touchX/80;
            int soff = cX+(cY*3);
            if ( soff >= watchfacesCaptures ) { soff = watchfacesCaptures-1; }
            mySelection=soff;
            LoT().selectedWatchFace=mySelection;
            //lAppLog("SOFFF: %d\n",soff);
        }
    }

    if ( millis() > nextRefresh ) { // redraw full canvas
        canvas->fillSprite(ThCol(background)); // use theme colors
        TemplateApplication::btnBack->DrawTo(canvas);
        
        canvas->setTextFont(1);
        canvas->setTextDatum(BR_DATUM);
        canvas->setTextSize(2);
        SwitchUseOnAlwaysOn->DrawTo(canvas);
        uint16_t textColor = ThCol(text);
        if ( false == SwitchUseOnAlwaysOn->switchEnabled ) { textColor = ThCol(text_alt); }
        canvas->setTextColor(textColor);
        canvas->drawString("Always on",SwitchUseOnAlwaysOn->GetX()-10,SwitchUseOnAlwaysOn->GetY()+(SwitchUseOnAlwaysOn->GetH()/2));
        canvas->setTextDatum(TR_DATUM);
        canvas->setTextSize(1);
        canvas->drawString("On fast wakeup",SwitchUseOnAlwaysOn->GetX()-10,SwitchUseOnAlwaysOn->GetY()+(SwitchUseOnAlwaysOn->GetH()/2));

        canvas->setTextDatum(TC_DATUM);
        canvas->setTextSize(2);
        canvas->setTextColor(ThCol(text));
        canvas->drawString("Select Watchface", canvas->width()/2, 8);
        int16_t x=0;
        int16_t y=30;
        for(size_t off=0;off<watchfacesCaptures;off++) {
            if ( nullptr != watchfaceCapture[off] ) {
                if ( off == mySelection ) {
                    canvas->fillRoundRect(x,y,80,80,5,ThCol(mark));
                }
                canvas->setPivot(x,y);
                CanvasWidget * colorBuffer = new CanvasWidget(watchfaceCapture[off]->height(),watchfaceCapture[off]->width());
                watchfaceCapture[off]->pushRotated(colorBuffer->canvas,0);
                colorBuffer->DrawTo(canvas,x+10,y+10);
                delete colorBuffer;
            }
            x+=canvas->width()/3;
            if ( x >= canvas->width() ) {
                x=40;
                y+=canvas->height()/3;
            }
        }
        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}
