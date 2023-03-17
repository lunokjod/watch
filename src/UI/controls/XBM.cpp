#include "XBM.hpp"
#include "../../app/LogView.hpp"

using namespace LuI;

XBM::~XBM() {
    lLog("XBM %p destroyed!\n",this);
    if ( nullptr != imageCanvas ) {
        imageCanvas->deleteSprite();
        delete imageCanvas;
        imageCanvas=nullptr;
    }
}

XBM::XBM(IN uint32_t width,IN uint32_t height, IN unsigned char *data, bool swap) :
                            imageWidth(width),imageHeight(height),imageData(data) {
    lLog("XBM %p created!\n",this);
    imageCanvas=new TFT_eSprite(ttgo->tft);
    imageCanvas->setColorDepth(1);
    imageCanvas->createSprite(width,height);
    imageCanvas->fillSprite(TFT_BLACK);
    imageCanvas->drawXBitmap(0,0,data,width,height,TFT_WHITE);

    //imageCanvas->pushImage(0,0,width,height, (uint16_t *)data);
}

void XBM::Refresh(bool swap) {
    //lLog("XBM %p refresh\n",this);
    if ( nullptr == canvas ) { Control::Refresh(swap); }
    //centered
    canvas->setPivot(canvas->width()/2,canvas->height()/2);
    imageCanvas->setPivot(imageCanvas->width()/2,imageCanvas->height()/2);
    imageCanvas->pushRotated(canvas,0,TFT_BLACK);
}
