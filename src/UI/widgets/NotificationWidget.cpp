#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"

#include "NotificationWidget.hpp"
NotificationWidget::NotificationWidget(std::function<void ()> notifyTo, char *message, uint32_t color)
// NotificationWidget::NotificationWidget(int16_t x,int16_t y, std::function<void ()> notifyTo, uint32_t btnBackgroundColor)
                : ButtonWidget(20,20,32,200,notifyTo,0xff0000), message(message), color(color) {
    buffer = new CanvasWidget(32,200);
    //if ( NULL == changeData ) { changeData = xSemaphoreCreateMutex(); }
    NotificationDraw();
}
void NotificationWidget::Show() {
    //if( xSemaphoreTake( changeData, portMAX_DELAY) == pdTRUE )  {
        this->dtime=0;
        this->ctime=millis()+ShowTime;
        //xSemaphoreGive( changeData );
    //}
    Serial.printf("Notification: %p show\n",this);
    //Serial.printf("OOOOOOOOOOOO CTIME: %lu DTIME: %lu\n",this->ctime,this->dtime);
}
NotificationWidget::~NotificationWidget() {
    if ( nullptr != buffer ) {
        delete buffer;
        buffer=nullptr;
    }
}
bool NotificationWidget::Interact(bool touch, int16_t tx,int16_t ty) {
    if ( dtime < millis() ) { return false; } 
    return ActiveRect::Interact(touch,tx,ty);
}
void NotificationWidget::NotificationDraw() {
    // box
    buffer->canvas->fillSprite(CanvasWidget::MASK_COLOR);
    buffer->canvas->fillRoundRect(0,0,w,h,5,color);
    buffer->canvas->drawRoundRect(0,0,w,h,5,TFT_WHITE);

    //message
    int32_t posX = 5;
    int32_t posY = 8;
    buffer->canvas->setTextSize(1);
    buffer->canvas->setTextDatum(TL_DATUM);
    buffer->canvas->setFreeFont(&FreeMonoBold9pt7b);
    buffer->canvas->setTextColor(TFT_BLACK);
    buffer->canvas->drawString(message, posX+1, posY+1);
    buffer->canvas->setTextColor(TFT_WHITE);
    buffer->canvas->drawString(message, posX, posY);

}
void NotificationWidget::DrawTo(TFT_eSprite * endCanvas) {
    if ( nullptr == endCanvas ) { return; }
    if ( CheckBounds() ) { // rebuild canvas if size is changed
        if ( nullptr != buffer ) {
            delete buffer;
        }
        buffer = new CanvasWidget(ActiveRect::h,ActiveRect::w);
        NotificationDraw();
    }
    uint8_t alpha = 255;
    //if( xSemaphoreTake( changeData, portMAX_DELAY) == pdTRUE )  {
        //Serial.printf("CTIME: %lu DTIME: %lu\n",this->ctime,this->dtime);
        if ( 0 != this->ctime ) {
            if ( millis() > this->ctime ) {
                Serial.printf("Notification: %p begans dissolve...\n",this);
                this->dtime=millis()+DissolveTime;
                this->ctime = 0;
            }
        } else { // ctime is 0
            if ( 0 == this->dtime ) { return; } // nothing to do, get out!
            if ( millis() > this->dtime ) {
                Serial.printf("Notification: %p Ends here\n",this);
                this->dtime  = 0;
                //xSemaphoreGive( changeData );
                return;
            }
        }
    //    xSemaphoreGive( changeData );
    //}

    if ( 0 != this->dtime ) {
        alpha = ((this->dtime-millis())*255/DissolveTime); // dtime is the future
        //Serial.printf("Dtimediff: %d alpha: %d\n",millis()-dtime, alpha);
    }
    if ( false == byteSwap ) {
        canvas->fillSprite(CanvasWidget::MASK_COLOR);
        buffer->DrawTo(canvas);
    }
    // fade magic
    for(int x=0;x<w;x++ ) {
        for(int y=0;y<h;y++ ) {
            uint16_t originalColor16 = endCanvas->readPixel(x+ActiveRect::x,y+ActiveRect::y);
            uint16_t myColor16;
            if ( byteSwap ) { // obtain from buffer
                myColor16 = buffer->canvas->readPixel(x,y);
            } else {
                myColor16 = canvas->readPixel(x,y); // obtain from canvas
            }
            endCanvas->drawPixel(x+ActiveRect::x,y+ActiveRect::y,canvas->alphaBlend(alpha,myColor16,originalColor16));
        }
    }
}
