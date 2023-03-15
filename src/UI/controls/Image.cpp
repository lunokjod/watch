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

Image::Image(IN uint32_t width,IN uint32_t height, IN unsigned char *data) :
                            imageWidth(width),imageHeight(height),imageData(data) {
    imageCanvas=new TFT_eSprite(ttgo->tft);
    imageCanvas->setColorDepth(16);
    imageCanvas->createSprite(width,height);
    imageCanvas->setSwapBytes(true);
    imageCanvas->pushImage(0,0,width,height, (uint16_t *)data);
    imageCanvas->setSwapBytes(false);
}

void Image::Refresh() {
    Control::Refresh();
    //centered
    canvas->setPivot(canvas->width()/2,canvas->height()/2);
    imageCanvas->setPivot(imageCanvas->width()/2,imageCanvas->height()/2);
    imageCanvas->pushRotated(canvas,0);
}
