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
#include "LuIGameOfLife.hpp"
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

const uint8_t pixelSize=20;
bool gameStart=false;

LuIDemoGameOfLifeApplication::LuIDemoGameOfLifeApplication() {
    directDraw=false; // disable direct draw meanwhile build the UI
    gameStart=false;
    // fill view with background color
    canvas->fillSprite(TFT_BLACK); // use theme colors

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



    // here the main view
    Buffer * gameOfLife=new Buffer(240,240);
    gameOfLife->showBars=false;
    gameOfLife->tapCallbackParam=gameOfLife;
    gameOfLife->tapCallback=[&](void *obj) {
        Buffer * game =(Buffer *)obj;
        int32_t correctedX = (abs((touchX+game->offsetX)/pixelSize)*pixelSize);
        int32_t correctedY = (abs((touchY+game->offsetY)/pixelSize)*pixelSize);
        TFT_eSprite * buff = game->GetBuffer();
        // read on center
        uint16_t current = buff->readPixel(correctedX+(pixelSize/2),correctedY+(pixelSize/2));
        uint16_t lastColor = TFT_WHITE;
        if ( TFT_WHITE == current ) { lastColor = TFT_BLACK; }
        buff->fillRect(correctedX,correctedY,pixelSize,pixelSize,lastColor);
    };
    gameOfLife->bufferPushCallbackParam=gameOfLife;
    gameOfLife->bufferPushCallback=[&](void *obj) {
        if ( false == gameStart ) { return; }
        Buffer * game =(Buffer *)obj;
        TFT_eSprite * source = game->GetBuffer();
        TFT_eSprite * buff = new TFT_eSprite(tft);
        buff->setColorDepth(source->getColorDepth());
        buff->createSprite(source->width(),source->height());
        buff->setSwapBytes(source->getSwapBytes());

        for(int y=0;y<source->height();y+=pixelSize) {
            for(int x=0;x<source->width();x+=pixelSize) {
                // read on center
                uint16_t mycell = source->readPixel(x+(pixelSize/2),y+(pixelSize/2));
                int neighbors=0;
                 // scan 3x3 for neighbors
                for(int ny=-1;ny<=1;ny++) {
                    for(int nx=-1;nx<=1;nx++) {
                        // read in the center of cell
                        int32_t fx=(x+(nx*pixelSize))+(pixelSize/2);
                        int32_t fy=(y+(ny*pixelSize))+(pixelSize/2);
                        // infinite world!
                        if ( fx < 0 ) { fx=fx+source->width(); }
                        else if ( fx > source->width() ) { fx=fx-source->width(); }
                        if ( fy < 0 ) { fy=fy+source->height(); }
                        else if ( fy > source->height() ) { fy=fy-source->height(); }
                        // edge world
                        //if ( ( fx <0 ) || ( fy <0 ) || ( fx > source->width()-1 ) || ( fy > source->height()-1 ) ) { continue; }
                        uint16_t currentN = source->readPixel(fx,fy);
                        if ( TFT_WHITE == currentN ) { neighbors++; }
                    }
                }
                uint16_t cellColor=TFT_BLACK; // dead by default
                // apply here Conway's world rules
                if ( TFT_WHITE == mycell ) {
                    neighbors-=1; // discount myself from count
                    //lLog("neighbors: %d\n",neighbors);
                    // cell alive
                    if ( neighbors <= 1 ) { cellColor=TFT_BLACK; } // dead by isolation
                    else if ( neighbors >= 4 ) { cellColor=TFT_BLACK; } // dead by overpoblation
                    else if ( ( neighbors == 2 )||( neighbors == 3 )) { cellColor=TFT_WHITE; } // fine
                } else {
                    // cell dead
                    if ( neighbors == 3 ) { // born! in contact with 3 alive
                        cellColor=TFT_WHITE;
                    }
                }
                buff->fillRect(x,y,pixelSize,pixelSize,cellColor);
            }
        }
        game->SetBuffer(buff); // control destroy last buffer automatically
        game->dirty=true; // force loop
    };
    
    viewContainer->AddChild(gameOfLife);

    Button *launchButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
    launchButton->border=10;
    launchButton->AddChild(new Text((char*)"Start/Stop"));
    launchButton->tapCallbackParam=gameOfLife;
    launchButton->tapCallback=[&](void * obj){
        Buffer * gameOfLife = (Buffer *)obj; 
        gameStart=(!gameStart);
        lLog("GAME state: %s\n",(gameStart?"true":"false"));
        gameOfLife->dirty=true;
    }; // callback when tap
    bottomButtonContainer->AddChild(launchButton,1.65);


    AddChild(screen);
    directDraw=true; // allow controls to direct redraw itself instead of push whole view Sprite
    // Thats all! the app is running
}


LuIDemoGameOfLifeApplication::~LuIDemoGameOfLifeApplication() {
}