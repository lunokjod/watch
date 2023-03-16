#include "../../../lunokIoT.hpp"
#include "Control.hpp"
#include "Container.hpp"
#include "../../../app/LogView.hpp"

using namespace LuI;



bool Container::AddChild(INOUT Control *control, IN float quota) {
    if ( currentChildSlot >= childNumber ) {
        lLog("Container full!\n");
        return false;
    }
    if ( nullptr == children ) {
        lLog("Child empty?\n");
        return false;
    }
    if ( control->Attach(this) ) {
        //lLog("Container %p add child %p\n",this,control);
        children[currentChildSlot] = control;
        quotaValues[currentChildSlot] = quota;
        currentChildSlot++;
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
    if ( nullptr != quotaValues ) {
        free(quotaValues);
        quotaValues=nullptr;
    }
    // destroy my childs
    if ( nullptr != children ) {
        for(size_t current=0;current<childNumber;current++) {
            if ( nullptr != children[current] ) {
                delete children[current];
                children[current]=nullptr;
            }
        }
        free(children);
    }
    //lLog("Container destroyed on %p\n",this);
}

void Container::Refresh(bool swap) {
    if ( nullptr == canvas  ) { Control::Refresh(swap); }
    //lLog("Container refresh on %p <================\n",this);
    uint32_t displacement=border;
    uint32_t sizeWidth=((width-((border*2)+(border*childNumber)))/childNumber);
    uint32_t sizeHeight=((height-((border*2)+(border*childNumber)))/childNumber);
    for(size_t current=0;current<currentChildSlot;current++) {
        if ( nullptr == children[current] ) { continue; }
        // re-set children size
        if ( LuI_Vertical_Layout == layout ) {
            children[current]->SetSize(sizeWidth*quotaValues[current],height-(border*2));
        } else {
            children[current]->SetSize(width-(border*2),sizeHeight*quotaValues[current]);
        }

        // push children on the parent view
        if ( LuI_Vertical_Layout == layout ) { // @TODO do this better x'D
            canvas->setPivot(displacement,border);
            displacement+=children[current]->width+border;
        } else {
            canvas->setPivot(border,displacement);
            displacement+=children[current]->height+border;
        }
        // update children clip
        children[current]->clipX = this->clipX+canvas->getPivotX();
        children[current]->clipY = this->clipY+canvas->getPivotY();
        // refresh children appearance
        children[current]->Refresh((!swap));
        TFT_eSprite * childImg = children[current]->GetCanvas();
        childImg->setPivot(0,0);
        childImg->pushRotated(canvas,0,Drawable::MASK_COLOR);
    }
}

void Container::EventHandler() {
    //lLog("Container %p X: %u Y: %u W: %u H: %u\n",this, clipX, clipY, width, height);
    //ttgo->tft->drawRect(clipX,clipY,width,height,TFT_GREEN);
    for(size_t current=0;current<currentChildSlot;current++) {
        if ( nullptr == children[current] ) { continue; }
        children[current]->EventHandler();
        if ( children[current]->dirty ) {
            children[current]->Refresh(true);
            TFT_eSprite * what = children[current]->GetCanvas();
            ttgo->tft->setSwapBytes(true);
            what->pushSprite(children[current]->clipX,children[current]->clipY);
            //ttgo->tft->setSwapBytes(false);
            children[current]->dirty=false;
        }
    }
}
