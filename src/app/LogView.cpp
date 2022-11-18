//#include <Arduino.h>

#include <libraries/TFT_eSPI/TFT_eSPI.h>
extern TFT_eSPI *tft;

#include "LogView.hpp"
#include "../static/img_back_32.xbm"
#include "Watchface.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

#include "../UI/UI.hpp"

SemaphoreHandle_t lLogAsBlockSemaphore = xSemaphoreCreateMutex();
TFT_eSprite * LogViewApplication::LogTextBuffer=nullptr;
SemaphoreHandle_t lLogSemaphore = NULL;
bool LogViewApplication::dirty=false;

void lLogCreate() {
    if ( nullptr == LogViewApplication::LogTextBuffer ) {
        LogViewApplication::LogTextBuffer = new TFT_eSprite(tft);
        LogViewApplication::LogTextBuffer->setColorDepth(1);
        LogViewApplication::LogTextBuffer->createSprite(LogViewApplication::TextWidth*50,LogViewApplication::TextHeight*40);
        LogViewApplication::LogTextBuffer->fillSprite(TFT_BLACK);
        LogViewApplication::LogTextBuffer->setTextWrap(false,false);
        LogViewApplication::LogTextBuffer->setTextFont(0);
        LogViewApplication::LogTextBuffer->setTextSize(1);
        LogViewApplication::LogTextBuffer->setTextColor(TFT_WHITE);
        LogViewApplication::LogTextBuffer->setCursor(0,(LogViewApplication::LogTextBuffer->height()-LogViewApplication::TextHeight));
    }
    if ( NULL == lLogSemaphore ) { lLogSemaphore = xSemaphoreCreateMutex(); }
}

void lRawLog(const char *fmt, ...) {
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
            LogViewApplication::dirty=true;
        }
        xSemaphoreGive( lLogSemaphore );
    } else {
        #ifdef LUNOKIOT_DEBUG_UI
            Serial.println("[UI] Timeout! last message wasn't included on visual log due draw timeout");
        #endif
    }
    va_end(args);
}
LogViewApplication::~LogViewApplication() {
    ViewBuffer->deleteSprite();
    delete ViewBuffer;
}

LogViewApplication::LogViewApplication() {
    // build the area for log
    ViewBuffer = new TFT_eSprite(tft);
    ViewBuffer->setColorDepth(1);
    ViewBuffer->createSprite(TFT_WIDTH,TFT_HEIGHT-70);
    ViewBuffer->fillSprite(TFT_BLACK);
    if ( nullptr != LogViewApplication::LogTextBuffer ) {
        offsetY = LogTextBuffer->height()-ViewBuffer->height();
        LogTextBuffer->setPivot(offsetX,offsetY);

        ViewBuffer->setPivot(0,0);
        ViewBuffer->fillSprite(TFT_BLACK);
        LogTextBuffer->pushRotated(ViewBuffer,0,TFT_BLACK);
    }
    Tick();
}

bool LogViewApplication::Tick() {

    UINextTimeout = millis()+UITimeout; // disable screen timeout on this app
    btnBack->Interact(touched,touchX, touchY);

    if ( touched ) {
        if ( ActiveRect::InRect(touchX,touchY,0,0,ViewBuffer->height(),ViewBuffer->width()) ) {
            if ( touchDragVectorX != 0 ) {
                lastTouch=true;
                if ( millis() > nextSlideTime ) {
                    if ( nullptr != LogTextBuffer ) {
                        LogTextBuffer->setPivot(0,0);
                        int16_t offX=offsetX+touchDragVectorX;
                        if ( offX > 0 ) { offX = 0; }
                        else if ( offX < ((LogTextBuffer->width()-ViewBuffer->width())*-1) ) { offX = ((LogTextBuffer->width()-ViewBuffer->width())*-1); }
                        int16_t offY=offsetY+touchDragVectorY;
                        if ( offY > 0 ) { offY = 0; }
                        else if ( offY < ((LogTextBuffer->height()-ViewBuffer->height())*-1) ) { offY = ((LogTextBuffer->height()-ViewBuffer->height())*-1); }
                        ViewBuffer->setPivot(0,0);
                        LogTextBuffer->setPivot(offX*-1,offY*-1);
                        ViewBuffer->fillSprite(TFT_BLACK);
                        LogTextBuffer->pushRotated(ViewBuffer,0,TFT_BLACK);
                        ViewBuffer->pushSprite(0,0);
                    }
                    nextSlideTime=millis()+(1000/16);
                }
            }
            return false;
        }
    } else if ( lastTouch ) {
        if ( nullptr != LogViewApplication::LogTextBuffer ) {
            offsetX+=touchDragVectorX;
            if ( offsetX > 0 ) { offsetX = 0; }
            else if ( offsetX < ((LogTextBuffer->width()-ViewBuffer->width())*-1) ) { offsetX = ((LogTextBuffer->width()-ViewBuffer->width())*-1); }
            
            offsetY+=touchDragVectorY;
            if ( offsetY > 0 ) { offsetY = 0; }
            else if ( offsetY < ((LogTextBuffer->height()-ViewBuffer->height())*-1) ) { offsetY = ((LogTextBuffer->height()-ViewBuffer->height())*-1); }
        }
        lastTouch=false;
    }
    if (millis() > nextRedraw ) {
        canvas->fillRect(0,0,TFT_WIDTH,TFT_HEIGHT-70,TFT_BLACK);
        canvas->fillRect(0,TFT_HEIGHT-70,TFT_WIDTH,70,ThCol(background));
        btnBack->DrawTo(canvas);

        if ( nullptr != LogViewApplication::LogTextBuffer ) {
            ViewBuffer->setPivot(0,0);
            if ( LogViewApplication::dirty ) {
                //LogTextBuffer->setPivot(offsetX*-1,offsetY*-1);
                ViewBuffer->fillSprite(TFT_BLACK);
                LogTextBuffer->pushRotated(ViewBuffer,0,TFT_BLACK);
                LogViewApplication::dirty = false;
            }
            canvas->setPivot(0,0);
            ViewBuffer->pushRotated(canvas,0,TFT_BLACK);
        }
        nextRedraw=millis()+(1000/3);
        return true;
    }
    return false;
}
