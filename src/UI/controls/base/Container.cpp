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
        lLog("Container %p full!\n",this);
        return false;
    }
    if ( nullptr == control ) { // empty hole
        lLog("Child %p empty added\n",this);
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
        this->dirty=true;
        return true;
    }
    return false;
}

Container::Container(LuI_Layout layout, size_t childs): layout(layout),childNumber(childs) {
    children=(Control**)ps_calloc(sizeof(Control *),childNumber);
    quotaValues=(float*)ps_calloc(sizeof(float),childNumber);
    //lLog("Container created on %p\n",this);
}

Container::~Container() {
    lLog("Container %p destroyed!\n",this);
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

void Container::Refresh(bool direct,bool swap) {
    lLog("Container %p refresh canvas at %p swap: %s dirty: %s direct: %s\n",this,canvas,(swap?"true":"false"),(dirty?"true":"false"),(direct?"true":"false"));
    bool wasDirty = dirty;
    if (dirty) { Control::Refresh(direct,swap); } // allow children to draw whatsoever on canvas
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
            children[current]->SetSize(sizeWidth,height-(border*2));
            canvas->setPivot(displacement,border);
            displacement+=(children[current]->width)+border;
        } else {
            children[current]->SetSize(width-(border*2),sizeHeight);
            canvas->setPivot(border,displacement);
            displacement+=(children[current]->height)+border;
        }
        // update children clip
        children[current]->clipX = this->clipX+canvas->getPivotX();
        children[current]->clipY = this->clipY+canvas->getPivotY();
        // refresh children appearance
        if ( wasDirty ) {
            // parent dirty, must refresh from my buffer
            lLog("BUFFERED REFRESH DUE PARENT IS DIRTY\n");
            children[current]->Refresh(false,(!swap)); // canvas redraw
        } else {
            children[current]->Refresh(direct,(!swap)); // canvas or direct (parent choice)
        }
        // compose my canvas
        TFT_eSprite * childImg = children[current]->GetCanvas();
        if ( nullptr != childImg ) { // allow control to dismiss their canvas and refresh itself?
            // push children on the parent view
            childImg->setPivot(0,0);
            childImg->pushRotated(canvas,0,Drawable::MASK_COLOR);
        }
    }
    // was dirty and can draw direct :D
    if ( ( wasDirty ) && ( direct ) ) {
        lLog("CONTAINER DIRECT\n");
        canvas->pushSprite(clipX,clipY);
    }
}

void Container::EventHandler() {
    //tft->drawRect(clipX,clipY,width,height,TFT_GREEN); //@DEBUG
    // don't call parent eventhandler, container isn't sensitive
    if ( dirty ) {
        Refresh(directDraw);
        return;
    } // need cleanup?
    // iterate my slots
    for(size_t current=0;current<currentChildSlot;current++) {
        if ( nullptr == children[current] ) { continue; } // hole, prevously removed control ignored
        children[current]->EventHandler(); // pass the handling duty
        if ( children[current]->dirty ) { // I'm dirty?
            children[current]->Refresh(directDraw); // buffered render
            if ( directDraw ) {
                TFT_eSprite * what = children[current]->GetCanvas(); // get view
                if ( ( nullptr!=what) && ( directDraw ) ) {
                    // canvas can return null to auto-manage itself
                    tft->setSwapBytes(false);
                    //lLog("Container push\n");
                    what->pushSprite(children[current]->clipX,children[current]->clipY,Drawable::MASK_COLOR);
                }
            }
        }
    }
}