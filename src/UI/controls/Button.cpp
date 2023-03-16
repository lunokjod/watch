#include "../../lunokIoT.hpp"
#include "base/Control.hpp"
#include "base/Container.hpp"

#include "Button.hpp"
#include "../../app/LogView.hpp"
#include "../UI.hpp"
using namespace LuI;

void Button::Refresh(bool swap) {
    //lLog("BUTTON REFRESH %s color: %x received: %x\n",(swap?"true":"false"),ThCol(button),color);
    //if ( nullptr == canvas ) { Control::Refresh((!swap)); }
    Control::Refresh((!swap));
    canvas->fillSprite(Drawable::MASK_COLOR);
    // draw button here!
    const int32_t radius = 8;
    // final color
    uint16_t workColor=color;
    if (false == swap) { workColor = ColorSwap(color); }
    uint16_t finalColor = workColor;
    if ( lastTouched ) { finalColor=canvas->alphaBlend(128,finalColor,TFT_WHITE); }
    /*
    if ( touched ) {
        if ( ( touchX > clipX ) && ( touchX < clipX+width ) 
            && ( touchY > clipY ) && ( touchY < clipY+height )  ) {
                finalColor=canvas->alphaBlend(128,finalColor,TFT_WHITE);
        }
    }*/

    /*
    double r = ((workColor >> 11) & 0x1F) / 31.0; // red   0.0 .. 1.0
    double g = ((workColor >> 5) & 0x3F) / 63.0;  // green 0.0 .. 1.0
    double b = (workColor & 0x1F) / 31.0;         // blue  0.0 .. 1.0
    */
    // bright
    workColor = canvas->alphaBlend(64,color,TFT_WHITE);
    canvas->drawRoundRect( 0, 0,canvas->width()-1,canvas->height()-1,radius,workColor);
    // shadow
    workColor = canvas->alphaBlend(192,TFT_BLACK,color);
    canvas->drawRoundRect( 1, 1,canvas->width()-2,canvas->height()-2,radius,workColor);
    // face
    canvas->fillRoundRect(1,1,canvas->width()-3,canvas->height()-3,radius-1,finalColor);
    //parentRefresh=swap;

    Container::Refresh(swap);
}

Button::Button(LuI_Layout layout, size_t childs,uint16_t color): Container(layout,childs),color(color) {
    border=5;
    touchEnabled=true;
}

void Button::EventHandler() {
    // use two methods, due button is an object with childs
    Control::EventHandler();
    Container::EventHandler();
}
