#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Playground5.hpp"
#include "../UI/widgets/CanvasZWidget.hpp"
#include "../UI/UI.hpp"
#include "../static/img_sample_400.c"

#include <libraries/TFT_eSPI/TFT_eSPI.h>

PlaygroundApplication5::~PlaygroundApplication5() {
    directDraw=false;
    if ( nullptr != imageDeepTest ) {
        delete imageDeepTest;
        imageDeepTest=nullptr;
    }
}
PlaygroundApplication5::PlaygroundApplication5() {
    directDraw=true;
// 400x216
    imageDeepTest = new CanvasZWidget( //img_sample2_120.height, img_sample2_120.width);
        img_sample_400.height, img_sample_400.width);
    //imageDeepTest->canvas->setSwapBytes(true);
    imageDeepTest->canvas->pushImage(0,0,img_sample_400.width,img_sample_400.height,(uint16_t *)img_sample_400.pixel_data);
    //imageDeepTest->canvas->setSwapBytes(false);
}
bool PlaygroundApplication5::Tick() {
    float currVal = imageDeepTest->GetScale();
    currVal+=increment;
    if ( currVal > 2.9 ) {increment = increment*-1; }
    if ( currVal < 0.1 ) {increment = increment*-1; }

    //TFT_eSprite * scaledImage = imageDeepTest->GetScaledImage(currVal);
    TFT_eSprite * scaledImage = imageDeepTest->GetScaledImageClipped(currVal,TFT_WIDTH,TFT_HEIGHT);
    int16_t clippingW = scaledImage->width();
    int16_t clippingH = scaledImage->height();
    if ( clippingH > TFT_HEIGHT ) { clippingH = TFT_HEIGHT; }
    if ( clippingW > TFT_WIDTH ) { clippingW = TFT_WIDTH; }
    uint16_t * bufferData = (uint16_t *)ps_calloc(clippingH*clippingW,sizeof(uint16_t)); 
    size_t offset=0;
    for(int32_t oy=0;oy<clippingH;oy++) {
        for(int32_t ox=0;ox<clippingW;ox++) {
            bufferData[offset]=scaledImage->readPixel(ox,oy);
            offset++;
        }
    }
    int32_t cx = (TFT_WIDTH/2)-(clippingW/2);
    int32_t cy = (TFT_HEIGHT/2)-(clippingH/2);
    ttgo->tft->pushRect(cx,cy,clippingW,clippingH,bufferData);
    const uint8_t border = 12;
    int32_t canx = cx;
    int32_t cany = cy;
    int32_t canh = clippingH;
    int32_t canw = clippingW;

    ttgo->tft->fillRect(canx-border,cany,border,canh,TFT_BLACK);
    ttgo->tft->fillRect(canx-border,cany-border,canw+(border*2),border,TFT_BLACK);
    ttgo->tft->fillRect(canx+canw,cany,border,canh,TFT_BLACK);
    ttgo->tft->fillRect(canx-border,cany+canh,canw+(border*2),border,TFT_BLACK);


    free(bufferData);
    scaledImage->deleteSprite();
    delete scaledImage;

    return false;
}
