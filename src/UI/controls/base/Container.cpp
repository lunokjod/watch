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

#include "../../../lunokIoT.hpp"
#include "Control.hpp"
#include "Container.hpp"
#include "../../../app/LogView.hpp"

using namespace LuI;

bool Container::AddChild(INOUT Control *control, IN float quota) {
    if ( currentChildSlot >= childNumber ) { // @TODO use for sentence to loop over free slots
        lUILog("Container %p full!\n",this);
        return false;
    }
    if ( nullptr == control ) { // empty hole
        lUILog("Child %p empty added\n",this);
        children[currentChildSlot] = nullptr;
        quotaValues[currentChildSlot] = quota;
        currentChildSlot++;
        return true;
    }

    if ( control->Attach(this) ) {
        //lLog("Container %p add child %p\n",this,control);
        children[currentChildSlot] = control;
        quotaValues[currentChildSlot] = quota;
        currentChildSlot++;
        dirty=true; // mark myself as dirty
        return true;
    }
    return false;
}

Container::Container(LuI_Layout layout, size_t childs): layout(layout),childNumber(childs) {
    lUILog("Container created on %p\n",this);
    children=(Control**)ps_calloc(sizeof(Control *),childNumber);
    quotaValues=(float*)ps_calloc(sizeof(float),childNumber);
}

Container::~Container() {
    lUILog("Container %p destroyed!\n",this);
    if ( nullptr != quotaValues ) {
        free(quotaValues);
        quotaValues=nullptr;
    }
    // destroy my childs
    if ( nullptr != children ) {
        for(size_t current=0;current<childNumber;current++) {
            if ( nullptr != children[current] ) {
                delete children[current];
                //children[current]=nullptr;
            }
        }
        free(children);
    }
    //lLog("Container destroyed on %p\n",this);
}

void Container::Refresh(bool direct) {
    bool wasDirty = dirty;
    if ( ( nullptr == canvas) || (dirty)) { Control::Refresh(false); } // allow children to draw whatsoever on canvas
    lUILog("Container %p refresh canvas at %p dirty: %s direct: %s\n",this,canvas,(wasDirty?"true":"false"),(direct?"true":"false"));
    // distribute children in my view space
    uint32_t displacement=border;
    for(size_t current=0;current<currentChildSlot;current++) {
        uint32_t sizeWidth=(width-border)/childNumber; // raw size
        uint32_t sizeHeight=(height-border)/childNumber;
        sizeWidth*=quotaValues[current]; // percentual size
        sizeHeight*=quotaValues[current];
        sizeWidth-=border;
        sizeHeight-=border;
        if ( nullptr == children[current] ) { // empty slot
            // count space, but control wants to draw itself with other methods
            if ( LuI_Vertical_Layout == layout ) {
                displacement+=(sizeWidth)+border;
            } else {
                displacement+=(sizeHeight)+border;
            }
            continue;
        }
        // re-set children size
        if ( LuI_Vertical_Layout == layout ) { // @TODO do this better x'D
            int32_t testH = height-(border*2);
            if ( testH < 0 ) { lUILog("Container %p Child: %u %p SO SMALL HEIGHT!\n",this,current,children[current]); }
            children[current]->SetSize(sizeWidth,testH);
            canvas->setPivot(displacement,border);
            displacement+=(children[current]->width)+border;
        } else {
            int32_t testW = width-(border*2);
            if ( testW < 0 ) { lUILog("Container %p Child: %u %p SO SMALL WIDTH!\n",this,current,children[current]); }
            children[current]->SetSize(testW,sizeHeight);
            canvas->setPivot(border,displacement);
            displacement+=(children[current]->height)+border;
        }
        // update children clip
        children[current]->clipX = this->clipX+canvas->getPivotX();
        children[current]->clipY = this->clipY+canvas->getPivotY();
        // refresh children appearance
        if ( wasDirty ) {
            // parent dirty, must refresh from my buffer
            //lLog("BUFFERED REFRESH DUE PARENT IS DIRTY\n");
            children[current]->Refresh(false); // canvas redraw
        } else {
            children[current]->Refresh(direct); // canvas or direct (parent choice)
        }
        // compose my canvas
        TFT_eSprite * childImg = children[current]->GetCanvas();
        if ( nullptr != childImg ) { // allow control to dismiss their canvas and refresh itself?
            // push children on the parent view

            childImg->setPivot(0,0);
            childImg->pushRotated(canvas,0,Drawable::MASK_COLOR);
            /*
            // perform dump from-to (this can be archieved with setWindow and writePixels )
            int offX = children[current]->clipX;
            int offY = children[current]->clipY;

            uint16_t maskColor = Drawable::MASK_COLOR;
            for(int y=0;y<childImg->height();y++) {
                for(int x=0;x<childImg->width();x++) {
                    uint16_t color = childImg->readPixel(x,y);
                    if ( Drawable::MASK_COLOR == color ) { continue; }
                    if ( false == direct ) { color=ByteSwap(color); }
                    canvas->drawPixel(offX+x,offY+y,color);
                }
            }*/
        }
    }
    // was dirty and can draw direct :D
    if ( ( wasDirty ) && ( direct ) ) {
        //lUILog("CONTAINER DIRECT\n");
        canvas->pushSprite(clipX,clipY,Drawable::MASK_COLOR);
    }
}

void Container::EventHandler() {
    //tft->drawRect(clipX,clipY,width,height,TFT_GREEN); //@DEBUG
    if ( dirty ) { Refresh(directDraw); return; }
    // I'm clean, search for dirty children....
    for(size_t current=0;current<childNumber;current++) {
        Control * currentControl = children[current]; 
        if ( nullptr == currentControl ) { continue; } // hole, prevously removed control ignored
        currentControl->EventHandler(); // pass the handling duty
        if ( currentControl->dirty ) { // I'm dirty?
            //lUILog("Container %p Refresh child %u\n",this,current);
            currentControl->Refresh(directDraw); // buffered render
            if ( false == directDraw ) {
                TFT_eSprite * what = currentControl->GetCanvas(); // get view
                if ( nullptr == what )  {
                    lUILog("Container %p Child %u at %p denies reveal their content\n",this,current,what);
                    continue;
                }
                // copy to my canvas (RAM)
                what->setPivot(0,0);
                canvas->setPivot(currentControl->clipX,currentControl->clipY);
                what->pushRotated(canvas,0,Drawable::MASK_COLOR); // push to app view
                dirty=true;
            }
        }
    }
}
