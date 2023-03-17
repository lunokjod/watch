#include "Image.hpp"
#include "../../app/LogView.hpp"

using namespace LuI;

Image::~Image() {
    lLog("Image %p destroyed!\n",this);
    if ( nullptr != imageCanvas ) {
        imageCanvas->deleteSprite();
        delete imageCanvas;
        imageCanvas=nullptr;
    }
}

Image::Image(IN uint32_t width,IN uint32_t height, IN unsigned char *data,
                                bool swap, LuI_Layout layout, size_t childs)
                    : imageWidth(width),imageHeight(height),imageData(data),Container(layout,childs) {
    lLog("Image %p created!\n",this);
    imageCanvas=new TFT_eSprite(ttgo->tft);
    imageCanvas->setColorDepth(16);
    imageCanvas->createSprite(width,height);
    imageCanvas->setSwapBytes(swap);
    imageCanvas->pushImage(0,0,width,height, (uint16_t *)data);
}

void Image::Refresh(bool swap) {
    lLog("Image %p refresh\n",this);
    if ( nullptr == canvas ) { Control::Refresh(swap); }
    //centered
    canvas->setPivot(canvas->width()/2,canvas->height()/2);
    imageCanvas->setPivot(imageCanvas->width()/2,imageCanvas->height()/2);
    //canvas->setSwapBytes(swap);
    imageCanvas->pushRotated(canvas,0);
    //canvas->setSwapBytes((!swap));
    Container::Refresh(swap);
}
