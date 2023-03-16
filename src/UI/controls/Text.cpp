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
}

Text::Text(IN char * what, IN uint16_t color,IN bool swap, IN GFXfont *font) : text(what),color(color),swapColor(swap),font(font) {
    lLog("Created Text on %p swap: %s\n",this,(swap?"true":"false"));
    imageTextCanvas=new TFT_eSprite(ttgo->tft);
    imageTextCanvas->setColorDepth(16);
    imageTextCanvas->setFreeFont(font);
    uint16_t fcolor = color;
    if ( swapColor ) {
        double r = ((color >> 11) & 0x1F) / 31.0; // red   0.0 .. 1.0
        double g = ((color >> 5) & 0x3F) / 63.0;  // green 0.0 .. 1.0
        double b = (color & 0x1F) / 31.0;         // blue  0.0 .. 1.0
        fcolor = canvas->color565(255*b,255*r,255*g);
    }
    imageTextCanvas->setTextColor(fcolor);
    imageTextCanvas->setTextSize(1);
    imageTextCanvas->setTextDatum(TL_DATUM);
    int16_t width = imageTextCanvas->textWidth(text);
    int16_t height = imageTextCanvas->fontHeight();
    imageTextCanvas->createSprite(width,height);
    //if ( swapColor ) {
    //    imageTextCanvas->fillSprite(TFT_TRANSPARENT);
    //} else {
        imageTextCanvas->fillSprite(Drawable::MASK_COLOR);
    //}
    imageTextCanvas->drawString(text,0,0);
}

void Text::Refresh(bool swap) {
    lLog("Text %p Refresh swap: %s\n",this,(swap?"true":"false"));
    if ( nullptr == canvas ) { Control::Refresh(swap); }
    //centered
    canvas->setPivot(canvas->width()/2,canvas->height()/2);
    imageTextCanvas->setPivot(imageTextCanvas->width()/2,imageTextCanvas->height()/2);
    
    //if ( swapColor ) {
        canvas->fillSprite(TFT_TRANSPARENT); // redraw with my own mask
    //    imageTextCanvas->pushRotated(canvas,0,TFT_TRANSPARENT);
    //} else {
        canvas->fillSprite(Drawable::MASK_COLOR); // redraw with my own mask
    //}
    imageTextCanvas->pushRotated(canvas,0,Drawable::MASK_COLOR);
    //canvas->setSwapBytes(swap);
}
