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

#include "Text.hpp"
#include "../../app/LogView.hpp"
#include "../UI.hpp"

using namespace LuI;

Text::~Text() {
    lLog("Text %p destroyed!\n",this);
    if ( nullptr != imageTextCanvas ) {
        imageTextCanvas->deleteSprite();
        delete imageTextCanvas;
        imageTextCanvas=nullptr;
    }
    if ( nullptr != text ) {
        free(text);
        text=nullptr;
    }
}

Text::Text(IN char * what, IN uint16_t color,IN bool swap, IN uint8_t tsize, IN GFXfont *font) : color(color),swapColor(swap),textSize(tsize),font(font) {
    lLog("Created Text on %p swap: %s\n",this,(swap?"true":"false"));
    SetText(what);
}
void Text::SetText(const char * what) {
    if ( ( nullptr != text ) && ( 0 == strcmp(what,text) ) ) { return; }
    if ( nullptr != text ) {
        free(text);
        text=nullptr;
    }
    text=(char*)ps_malloc(strlen(what)+1);
    strcpy(text,what);
    if ( nullptr != imageTextCanvas ) {
        imageTextCanvas->deleteSprite();
        delete imageTextCanvas;
    } 
    imageTextCanvas=new TFT_eSprite(tft);
    imageTextCanvas->setColorDepth(1);
    imageTextCanvas->setFreeFont(font);
    imageTextCanvas->setTextColor(TFT_WHITE);
    imageTextCanvas->setTextSize(textSize);
    imageTextCanvas->setTextDatum(TL_DATUM);
    int16_t twidth = imageTextCanvas->textWidth(text);
    int16_t theight = imageTextCanvas->fontHeight()*textSize;
    imageTextCanvas->createSprite(twidth,theight);
    imageTextCanvas->fillSprite(TFT_BLACK);
    imageTextCanvas->drawString(text,0,0);
    dirty=true;
}

void Text::Refresh(bool direct) {
    if ( false == dirty ) { return; }
    lLog("Text %p refresh dirty: %s direct: %s\n",this,(dirty?"true":"false"),(direct?"true":"false"));
    //lLog("Text %p refresh dirty: %s\n",this,(dirty?"true":"false"));
    Control::Refresh();

    uint16_t fcolor = color;
    uint16_t backColor = backgroundColor;
    /*
    if ( (false== direct) && ( directDraw )) { // parent push, swap bytes to match TFT
        lLog("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
        fcolor = ByteSwap(fcolor);
        backColor = ByteSwap(backColor);
    }*/
    canvas->fillSprite(backColor);
    // center text
    int cXOff=(int(width)-imageTextCanvas->width())/2;
    int cYOff=(int(height)-imageTextCanvas->height())/2;
    //lLog("COLOR: %x %x BACKGROUND: %x %x TRANSP: %x\n",fcolor,color,backColor,backgroundColor,Drawable::MASK_COLOR);
    for(int y=0;y<imageTextCanvas->height();y++) {
        for(int x=0;x<imageTextCanvas->width();x++) {
            bool isTextPart = imageTextCanvas->readPixel(x,y);
            if ( false == isTextPart ) { continue; } 
            canvas->drawPixel(cXOff+x,cYOff+y,fcolor);
        }
    }
    if ( direct ) {
        //lLog("DIRECT\n");
        canvas->pushSprite(clipX,clipY,Drawable::MASK_COLOR);
    }
}
