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
        lLog("Container out of memory\n");
        return false;
    }
    if ( control->Attach(this) ) {
        lLog("Container %p add child %p\n",this,control);
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
    lLog("Container created on %p\n",this);
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
    lLog("Container destroyed on %p\n",this);
}

void Container::Refresh() {
    Control::Refresh();
    lLog("Container refresh on %p\n",this);
    
    uint32_t displacement=0;
    uint32_t sizeWidth=(width/childNumber);
    uint32_t sizeHeight=(height/childNumber);

    for(size_t current=0;current<currentChildSlot;current++) {
        if ( nullptr == children[current] ) { continue; }
        // re-set children size
        if ( LuI_Vertical_Layout == layout ) {
            children[current]->SetSize(sizeWidth*quotaValues[current],height);
        } else {
            children[current]->SetSize(width,sizeHeight*quotaValues[current]);
        }
        // refresh children appearance
        children[current]->Refresh();
        TFT_eSprite * childImg = children[current]->GetCanvas();

        // push children on the parent view
        childImg->setPivot(0,0);
        if ( LuI_Vertical_Layout == layout ) {
            canvas->setPivot(displacement,0);
            displacement+=childImg->width();
        } else {
            canvas->setPivot(0,displacement);
            displacement+=childImg->height();
        }
        childImg->pushRotated(canvas,0);
    }
}
