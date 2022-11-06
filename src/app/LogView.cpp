#include <Arduino.h>
#include <LilyGoWatch.h>
#include "LogView.hpp"
#include "../static/img_back_32.xbm"
#include "Watchface.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

TFT_eSprite * LogViewApplication::LogTextBuffer=nullptr;
SemaphoreHandle_t lLogSemaphore = NULL;

void lLogCreate() {
    if ( nullptr == LogViewApplication::LogTextBuffer ) {
        LogViewApplication::LogTextBuffer = new TFT_eSprite(ttgo->tft);
        LogViewApplication::LogTextBuffer->setColorDepth(1);
        LogViewApplication::LogTextBuffer->createSprite(TFT_WIDTH,TFT_HEIGHT-70);
        LogViewApplication::LogTextBuffer->fillSprite(TFT_BLACK);
        //LogViewApplication::LogTextBuffer->setTextWrap(true,true);
        LogViewApplication::LogTextBuffer->setTextFont(0);
        LogViewApplication::LogTextBuffer->setTextSize(1);
        LogViewApplication::LogTextBuffer->setTextColor(TFT_WHITE);
        LogViewApplication::LogTextBuffer->setCursor(0,(LogViewApplication::LogTextBuffer->height()-LogViewApplication::TextHeight));
    }
    if ( NULL == lLogSemaphore ) { lLogSemaphore = xSemaphoreCreateMutex(); }
}
void lLog(const char *fmt, ...) {
    lLogCreate();
    va_list args;
    va_start(args, fmt);
    static char buf[1024];
    int resLen = vsnprintf(buf, 1024, fmt, args);
    #ifdef LUNOKIOT_DEBUG
        Serial.printf(buf);
    #endif
    if( xSemaphoreTake( lLogSemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) == pdTRUE )  {
        if ( nullptr != LogViewApplication::LogTextBuffer ) {
            LogViewApplication::LogTextBuffer->printf(buf);
            int16_t x = LogViewApplication::LogTextBuffer->getCursorX();
            int16_t y = LogViewApplication::LogTextBuffer->getCursorY();
            if ( y > (LogViewApplication::LogTextBuffer->height()-LogViewApplication::TextHeight) ) {
                LogViewApplication::LogTextBuffer->scroll(0,LogViewApplication::TextHeight*-1);
                LogViewApplication::LogTextBuffer->setCursor(x,(LogViewApplication::LogTextBuffer->height()-LogViewApplication::TextHeight));
            }
        }
        xSemaphoreGive( lLogSemaphore );
    } else {
        #ifdef LUNOKIOT_DEBUG
            Serial.println("UI: last message wasn't included on visual log due draw timeout");
        #endif
    }
    va_end(args);
}
LogViewApplication::~LogViewApplication() {
    if ( nullptr != btnBack ) { delete btnBack; }
    // don't destroy textBuffer
}

LogViewApplication::LogViewApplication() {
    btnBack=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](){
        LaunchApplication(new WatchfaceApplication());
    },img_back_32_bits,img_back_32_height,img_back_32_width,TFT_WHITE,ttgo->tft->color24to16(0x353e45),false);

}

bool LogViewApplication::Tick() {
    UINextTimeout = millis()+UITimeout; // disable screen timeout on this app

    btnBack->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillRect(0,0,TFT_WIDTH,TFT_HEIGHT-70,TFT_BLACK);
        canvas->fillRect(0,TFT_HEIGHT-69,TFT_WIDTH,69,canvas->color24to16(0x212121));
        btnBack->DrawTo(canvas);        
        if ( nullptr != LogTextBuffer ) {            
            LogTextBuffer->setPivot(0,0);
            canvas->setPivot(0,0);
            LogTextBuffer->pushRotated(canvas,0,0);
        }
        nextRedraw=millis()+(1000/10);
        return true;
    }
    return false;
}