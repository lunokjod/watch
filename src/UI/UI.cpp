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

#ifdef LILYGO_DEV
#include <LilyGoWatch.h>
#elif defined(M5_DEV)
#include <M5Core2.h>
#endif

//#include <LilyGoWatch.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_event.h>
#include <esp_event_base.h>
#include <functional>
#include <esp_task_wdt.h>
#include <ArduinoNvs.h>

#include "lunokiot_config.hpp"
#include "UI.hpp"
#include "BootSplash.hpp"
#include "../system/SystemEvents.hpp"
#include "../system/Application.hpp"
//#include "representation/Point2D.hpp"
#include "widgets/CanvasWidget.hpp"
#include "../app/LogView.hpp"

#ifdef LILYGO_WATCH_2020_V3
#include <driver/i2s.h>
#include "AudioFileSourcePROGMEM.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#endif


//#include "../app/TaskSwitcher.hpp"

#include "../system/Application.hpp"
#include "../lunokIoT.hpp"

extern SoftwareKeyboard *keyboardInstance;

#ifdef LILYGO_DEV
extern TTGOClass *ttgo; // ttgo library shit ;)
extern TFT_eSPI * tft;
#elif defined(M5_DEV)
//extern TTGOClass *ttgo; // ttgo library shit ;)
extern M5Display * tft;
#endif

bool UIlongTap=false; // used to trigger only once the long tap event
bool UILongTapOverride=false; // disable long tap to task switcher feature
extern const PROGMEM uint8_t screenshoot_sound_start[] asm("_binary_asset_screenshoot_sound_mp3_start");
extern const PROGMEM uint8_t screenshoot_sound_end[] asm("_binary_asset_screenshoot_sound_mp3_end");

// default theme and palette
const TemplateColorPalette * currentColorPalette = &DefaultThemeColorPalette;
const TemplateThemeScheme * currentThemeScheme = &DefaultThemeScheme;

bool touched=false; // is screen touch?
int16_t touchX=0;   // thumb coords
int16_t touchY=0;
double touchDragAngle=0;    // angle of drag
double touchDragDistance=0; // distance
int16_t touchDragVectorX=0; // XY vectors
int16_t touchDragVectorY=0;
unsigned long touchDownTimeMS=0; // time push

bool directDraw =false; // don't refresh screen (application-driven)
uint32_t FPS=0;         // current FPS

//bool UIAlertLedEnabled = false;
int16_t downTouchX=120;
int16_t downTouchY=120;

//size_t screenShootCurrentImageX=0;
//size_t screenShootCurrentImageY=0;


//std::list<TFT_eSprite *>ScreenShots;

uint8_t pendingNotifications = 0;

SemaphoreHandle_t UISemaphore = xSemaphoreCreateMutex();

const unsigned long UITimeout = 20*1000;

unsigned long UINextTimeout = 0;
bool UIRunning = true;

/* Event source task related definitions */
ESP_EVENT_DEFINE_BASE(UI_EVENTS);

// Event loops
esp_event_loop_handle_t uiEventloopHandle;
TFT_eSprite *screenShootCanvas = nullptr;
bool screenShootInProgress = false; // @TODO watchface must show it

void SetUserBrightness() {
    uint8_t userBright = NVS.getInt("lBright");
    if ( 0 == userBright ) {
        userBright=BaseBackLightBrightness;
        NVS.setInt("lBright",userBright,false);
    }
#ifdef LILYGO_DEV
    if ( ttgo->bl->isOn() ) { ttgo->setBrightness(userBright); }
#elif defined(M5_DEV)
    tft->setBrightness(userBright);
#endif
}

void ScreenWake() {
    //lLog("@DEBUG TAKE SEMAPHORE WAKE\n");
    bool done = xSemaphoreTake(UISemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
    if (false == done) {
        lEvLog("Unable to obtain the Screen Lock!\n");
        return;
    }
    #ifdef LILYGO_DEV
    if ( false == ttgo->bl->isOn() ) {
        LoT().CpuSpeed(240);
        if ( ttgo->rtc->isValid() ) { ttgo->rtc->syncToSystem(); }
        ttgo->displayWakeup();
        UINextTimeout = millis()+UITimeout;
        #if defined(LILYGO_WATCH_2020_V2) || defined(LILYGO_WATCH_2020_V3)
        ttgo->touchWakup();
        #endif
        //delay(1); // get time to queue digest ;)
        //SystemEventBootEnd(); // perform a ready (and if all is ok, launch watchface)
        ttgo->bl->on();
        //tft->fillScreen(TFT_BLACK); // cleanup, better than show old watchface time! (missinformation)
    }
    #endif
    esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_CONTINUE,nullptr, 0, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
    FPS = MAXFPS;
    xSemaphoreGive(UISemaphore);
}

void ScreenSleep() {
    bool done = xSemaphoreTake(UISemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS);
    if (false == done) {
        lEvLog("Unable to obtain the Screen Lock!\n");
        return;
    }
    #ifdef LILYGO_DEV
    if ( true == ttgo->bl->isOn() ) {
        ttgo->bl->off();
        lUILog("Put screen to sleep now\n");
        ttgo->displaySleep();
        delay(1);
        ttgo->touchToSleep();
    #endif
        esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_STOP,nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        LoT().CpuSpeed(80);
    #ifdef LILYGO_DEV
    }
    #endif
    xSemaphoreGive(UISemaphore);
    
}

/*
static void UIAnchor2DChange(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // @TODO recovered object contains uninitialized data ¿maybe a copy constructor?
    //return;
    Point2D *anchor = reinterpret_cast<Point2D *>(event_data);
    //Point2D *anchor = (Point2D *)event_data;
    int16_t deltaX = 0;
    int16_t deltaY = 0;
    anchor->GetDelta(deltaX,deltaY);
    lUILog("UI: Anchor(%p) Changed X: %d Y: %d\n",anchor, deltaX, deltaY);
}*/

/*
void TakeScreenShootSound() {
#ifdef LUNOKIOT_SILENT_BOOT
    lUILog("Audio: Not initialized due Silent boot is enabled\n");
    delay(150);
    return;
#endif
#ifdef LILYGO_WATCH_2020_V3
    // Audio fanfare x'D
    lUILog("Audio: Initialize\n");
    ttgo->enableAudio();

    // from https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/examples/UnitTest/HardwareTest/HardwareTest.ino
    AudioGeneratorMP3 *mp3;
    AudioFileSourcePROGMEM *file;
    AudioOutputI2S *out;
    AudioFileSourceID3 *id3;
    // file = new AudioFileSourcePROGMEM(image, sizeof(image));
    file = new AudioFileSourcePROGMEM(screenshoot_sound_start, (uint32_t)(screenshoot_sound_end-screenshoot_sound_start));
    id3 = new AudioFileSourceID3(file);
    out = new AudioOutputI2S();
    out->SetPinout(TWATCH_DAC_IIS_BCK, TWATCH_DAC_IIS_WS, TWATCH_DAC_IIS_DOUT);
    lUILog("Audio: MP3 Screenshoot\n");
    mp3 = new AudioGeneratorMP3();
    mp3->begin(id3, out);
    while (true) {
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();
            }
        } else {
            lUILog("Audio: MP3 done\n");
            break;
        }
    }
    delete file;
    delete id3;
    delete out;
    delete mp3;

    ttgo->disableAudio();

    i2s_driver_uninstall(I2S_NUM_0);
    delay(2);
    ttgo->shake();
#endif

}
*/

// https://www.gamedev.net/forums/topic.asp?topic_id=295943
/* IsPointInTri() - 
 * Used by IsPointInQuad(). Function takes the point and the triangle's 
 * vertices. It finds the area of the passed triangle (v1 v2 v3), and then the
 * areas of the three triangles (pt v2 v3), (pt v1 v3), and (pt v1 v2). If the
 * sum of these three is greater than the first, then the point is outside of
 * the triangle.
 */
bool IsPointInTri(u32Point *pt, u32Point *v1, u32Point *v2, u32Point *v3) {
  float TotalArea = CalcTriArea(v1, v2, v3);
  float Area1 = CalcTriArea(pt, v2, v3);
  float Area2 = CalcTriArea(pt, v1, v3);
  float Area3 = CalcTriArea(pt, v1, v2);

  if((Area1 + Area2 + Area3) > TotalArea)
    return false;
  else
    return true;
}

/* CalcTriArea() - 
 * Find the area of a triangle. This function uses the 1/2 determinant
 * method. Given three points (x1, y1), (x2, y2), (x3, y3):
 *             | x1 y1 1 |
 * Area = .5 * | x2 y2 1 |
 *             | x3 y3 1 |
 * From: http://mcraefamily.com/MathHelp/GeometryTriangleAreaDeterminant.htm
 */
float CalcTriArea(u32Point *v1,u32Point *v2,u32Point *v3) {
  float det = 0.0f;
  det = ((v1->x - v3->x) * (v2->y - v3->y)) - ((v2->x - v3->x) * (v1->y - v3->y));
  return (det / 2.0f);
}

//https://stackoverflow.com/questions/13300904/determine-whether-point-lies-inside-triangle
float area(int x1, int y1, int x2, int y2, int x3, int y3) {
   return abs((x1*(y2-y3) + x2*(y3-y1)+ x3*(y1-y2))/2.0);
}
/* A function to check whether point P(x, y) lies inside the triangle formed 
   by A(x1, y1), B(x2, y2) and C(x3, y3) */
bool isInside( int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
   /* Calculate area of triangle ABC */
   float A = area (x1, y1, x2, y2, x3, y3);

   /* Calculate area of triangle PBC */  
   float A1 = area (x, y, x2, y2, x3, y3);

   /* Calculate area of triangle PAC */  
   float A2 = area (x1, y1, x, y, x3, y3);

   /* Calculate area of triangle PAB */   
   float A3 = area (x1, y1, x2, y2, x, y);

   /* Check if sum of A1, A2 and A3 is same as A */
   return (A == A1 + A2 + A3);
}

TFT_eSprite * ShearSprite(TFT_eSprite *view, TransformationMatrix transform) {
    // https://www.mathsisfun.com/algebra/matrix-transform.html
    TFT_eSprite * canvas = new TFT_eSprite(tft); // build new sprite
    if ( nullptr == canvas ) { return nullptr; }
    canvas->setColorDepth(view->getColorDepth());
    if ( nullptr == canvas->createSprite(view->width()*transform.a, view->height()*transform.d) ) {
        delete canvas;
        return nullptr;
    }
    canvas->fillSprite(TFT_PINK); // transparency

    // apply transform
    for(int y=0;y<view->height();y++) {
        for(int x=0;x<view->width();x++) {
            uint16_t color = view->readPixel(x,y);
            int nx=transform.a*x+transform.b*y;
            int ny=transform.c*x+transform.d*y;
            if ( nx < 0 ) { continue; }
            else if ( nx > canvas->width() ) { continue; }
            if ( ny < 0 ) { continue; }
            else if ( ny > canvas->height() ) { continue; }
            //canvas->drawPixel(nx,ny,color);
            canvas->fillRect(nx,ny,transform.a*1.0,transform.d*1.0,color);
        }
    }
    /*
    // fill holes
    for(int y=0;y<canvas->height();y++) {
        for(int x=1;x<canvas->width();x++) {
            uint16_t color = canvas->readPixel(x,y);
            if ( TFT_PINK != color ) { continue; }
            color = canvas->readPixel(x-1,y);
            canvas->drawPixel(x,y,color);
        }
    }*/

    return canvas;
    /*
    for (let i = 0; i < shape.pts.length; i++) {
        let pt = shape.pts[i]
        let x = a * pt[0] + b * pt[1]
        let y = c * pt[0] + d * pt[1]
        newPts.push({ x: x, y: y })
    }*/
}
// reduce the size of image using float value (0.5=50%)
TFT_eSprite * ScaleSprite(TFT_eSprite *view, float divisor) {
    if ( nullptr == view ) { return nullptr; }
    //return ShearSprite(view,{divisor,0,0,divisor});

    //if ( 1.0 == divisor ) { return DuplicateSprite(view); } // is the same!!! @TODO Duplicate invert colors :( invert colors
    if ( divisor < 0.0 ) { divisor=0.05; } // dont allow 0 scale
    int16_t nh = view->height()*divisor; // calculate new size
    int16_t nw = view->width()*divisor;

    TFT_eSprite * canvas = new TFT_eSprite(tft); // build new sprite
    if ( nullptr == canvas ) { return nullptr; }
    canvas->setColorDepth(view->getColorDepth());
    if ( nullptr == canvas->createSprite(nw, nh) ) {
        delete canvas;
        return nullptr;
    }
    //canvas->fillSprite(TFT_RED);//CanvasWidget::MASK_COLOR);

    //Serial.printf("ScaleSprite: H: %d W: %d (divisor: %f) Calculated H: %d W: %d\n",
    //     view->height(),view->width(), divisor, canvas->height(),canvas->width());

    // iterate whole paint searching for pixels
    for(float y=0;y<nh;y++) {
        int32_t ry = int32_t(y/divisor);
        for(float x=0;x<nw;x++) {
            int32_t rx = int32_t(x/divisor);

            uint16_t originalColor = view->readPixel(rx,ry);
            canvas->drawPixel(x,y,originalColor);
            //Serial.printf("ScaleSprite: X: %f Y: %f cX: %d cY: %d COLOR: 0x%04x\n",x,y,rx,ry,originalColor);
        }
    }
    return canvas;
}

uint16_t Get16BitFromRGB(uint32_t color888) {
    uint16_t r = (color888 >> 8) & 0xF800;
    uint16_t g = (color888 >> 5) & 0x07E0;
    uint16_t b = (color888 >> 3) & 0x001F;

    return (r | g | b);
}

void GetRGBFrom16Bit(const uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
    double pr = ((color >> 11) & 0x1F) / 31.0; // red   0.0 .. 1.0
    double pg = ((color >> 5) & 0x3F) / 63.0;  // green 0.0 .. 1.0
    double pb = (color & 0x1F) / 31.0;         // blue  0.0 .. 1.0
    r=255*pr;
    g=255*pg;
    b=255*pb;
}

uint16_t ColorSwap(uint16_t colorToSwap) {
    // twist color from RGB to BRG for draw on canvas under certain circumstances
    double r = ((colorToSwap >> 11) & 0x1F) / 31.0; // red   0.0 .. 1.0
    double g = ((colorToSwap >> 5) & 0x3F) / 63.0;  // green 0.0 .. 1.0
    double b = (colorToSwap & 0x1F) / 31.0;         // blue  0.0 .. 1.0
    uint8_t rC=255*r;
    uint8_t gC=255*g;
    uint8_t bC=255*b;
    return tft->color565(rC,gC,bC);
}

uint16_t ByteSwap(uint16_t colorToSwap) {
    uint8_t wf = (colorToSwap % 256);
    uint8_t wb = ((colorToSwap / 256) % 256);
    uint16_t fcolor = (wf*256)+wb;
    return fcolor;
}

TFT_eSprite * ScaleSpriteMAX(TFT_eSprite *view, float divisor, int16_t maxW, int16_t maxH) {
    int16_t nh = view->height()*divisor; // calculate new size
    int16_t nw = view->width()*divisor;

    TFT_eSprite * canvas = new TFT_eSprite(tft); // build new sprite
    canvas->setColorDepth(view->getColorDepth());
    canvas->createSprite(nw, nh);
    canvas->fillSprite(TFT_RED);//CanvasWidget::MASK_COLOR);

    //Serial.printf("ScaleSprite: H: %d W: %d (divisor: %f) Calculated H: %d W: %d\n",
    //     view->height(),view->width(), divisor, canvas->height(),canvas->width());

    for(float y=0;y<nh;y++) {
        if ( y > maxH ) { break; }
        int32_t ry = int32_t(y/divisor);
        for(float x=0;x<nw;x++) {
            if ( x > maxW ) { break; }
            int32_t rx = int32_t(x/divisor);

            uint16_t originalColor = view->readPixel(rx,ry);
            canvas->drawPixel(x,y,originalColor);
            //Serial.printf("ScaleSprite: X: %f Y: %f cX: %d cY: %d COLOR: 0x%04x\n",x,y,rx,ry,originalColor);
        }
    }
    return canvas;
}
/*
TFT_eSprite *TakeScreenShoot() {
    TFT_eSprite *appView = currentApplication->canvas;
    if ( nullptr == appView ) { return nullptr; }
    TFT_eSprite *myCopy = DuplicateSprite(appView);
    ttgo->setBrightness(0);
    tft->fillScreen(TFT_WHITE);
    ttgo->setBrightness(255);
    tft->fillScreen(TFT_BLACK);
    ttgo->setBrightness(0);
    delay(10);
    tft->fillScreen(TFT_WHITE);
    ttgo->setBrightness(255);
    // show thumbnail centered on screen
    TFT_eSprite *scaledImg = ScaleSprite(appView,0.8);
    //TFT_eSprite *imgCopy = DuplicateSprite(scaledImg);
    //scaledImg->pushSprite((TFT_WIDTH/2)-(imgCopy->width()/2),(TFT_HEIGHT/2)-(imgCopy->width()/2));
    scaledImg->pushSprite((TFT_WIDTH/2)-(scaledImg->width()/2),(TFT_HEIGHT/2)-(scaledImg->width()/2));
    scaledImg->deleteSprite();
    delete scaledImg;
    //imgCopy->deleteSprite();
    //delete imgCopy;
    delay(200);
    TakeScreenShootSound();
#ifdef LILYGO_WATCH_2020_V3
    ttgo->shake();
#endif
    return myCopy;
}
*/
TFT_eSprite * GetSpriteRect(TFT_eSprite *view, int16_t x, int16_t y, int16_t h, int16_t w) {
    int16_t cx = x;
    int16_t cy = y;
    int16_t ch = h;
    int16_t cw = w;
    if ( cx < 0 ) { cx = 0; }
    if ( (cx+cw) > view->width() ) { cx=view->width()-cw; }
    if ( cy < 0 ) { cy = 0; }
    if ( (cy+ch) > view->height() ) { cy=view->height()-ch; }

    TFT_eSprite * out = new TFT_eSprite(tft);
    if ( nullptr == out ) { return nullptr; }
    out->setColorDepth(view->getColorDepth());
    out->createSprite(cw,ch);

    for(int ny=0;ny<ch;ny++) {
        for(int nx=0;nx<cw;nx++) {
            uint16_t color = view->readPixel(cx+nx,cy+ny);
            out->drawPixel(nx,ny,color);
        }
    }
    /*
    int16_t fx=0;
    int16_t fy=0; 
    for(int ny=cy;ny<ch;ny++) {
        for(int nx=cx;nx<cw;nx++) {
            uint16_t color = view->readPixel(nx,ny);
            out->drawPixel(fx,fy,color);
            fx++;
        }
        fx=0;
        fy++;
    }
    */
    return out;
}

TFT_eSprite * DuplicateSprite(TFT_eSprite *view) {
    screenShootCanvas = new TFT_eSprite(tft);
    if ( nullptr == screenShootCanvas ) { return nullptr; }
    screenShootCanvas->setColorDepth(view->getColorDepth());
    screenShootCanvas->createSprite(view->width(),view->height());
    //screenShootCanvas->fillSprite(CanvasWidget::MASK_COLOR);
    view->setPivot(view->width()/2,view->height()/2);
    view->pushRotated(screenShootCanvas,0);//,CanvasWidget::MASK_COLOR);
    return screenShootCanvas;
}

extern SemaphoreHandle_t I2cMutex;


// draw some user-waring about "thiking event" (app loading)
Ticker UIAnimationCareetTimer;
static void UIEventLoadingCareetStep() { // some loop to show
    //lLog("@DEBUG TAKE SEMAPHORE CAREEET\n");
    if ( pdTRUE != xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) ) { return; }
    if ( nullptr == currentApplication ) { xSemaphoreGive( UISemaphore ); return; }
    if ( nullptr == currentApplication->canvas ) { xSemaphoreGive( UISemaphore ); return; }
    static int cangle = 0; // current angle anim
    const int centerX=currentApplication->canvas->width()/2;
    const int centerY=currentApplication->canvas->height()/2;
    const int radius=20;
    const int circleRadius=5;
    const int circleBorder=2;
    const int borders=radius+circleRadius+circleBorder;
    TFT_eSprite *piece = GetSpriteRect(currentApplication->canvas,centerX-borders,centerY-borders,borders*2,borders*2);
    xSemaphoreGive( UISemaphore );
    if ( nullptr == piece ) { return; }

    // draw a "loading circle"
    // outer black border
    piece->fillCircle(borders,borders,borders,TFT_BLACK);
    // circle radius
    DescribeCircle2(borders,borders,radius, [&](int x, int y, int cx, int cy, int angle, int step, IGNORE_PARAM) {
        //if ( angle < 0 ) { return true; }
        if ( angle < cangle ) { return true; }
        // circle loop
        piece->fillCircle(x,y,circleRadius,TFT_WHITE);
        return true;
    });
    cangle+=(360 / 7); // animate
    if ( cangle > 360 ) { cangle = 0; }
    // push piece
    if ( pdTRUE == xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) ) {
        piece->pushSprite(centerX-borders,centerY-borders);
        xSemaphoreGive( UISemaphore );
    }
    // clean resources out of UI draw
    piece->deleteSprite();
    delete piece;
}

// show the "please wait" untlil app is loaded
static void UIEventLaunchApp(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    lUILog("Event App launch\n");
    UIAnimationCareetTimer.detach();
    //UIAnimationCareetTimer.attach_ms((1000/5),UIEventLoadingCareetStep);
}

// hide the "please wait" when app is loaded
static void UIEventLaunchAppEnd(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    UIAnimationCareetTimer.detach();
    /*
    if ( pdTRUE == xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_FAST_TIME_TICKS) ) {
        if ( nullptr != currentApplication ) {
            // force screen refresh to override problems with directDraw applications
            currentApplication->canvas->pushSprite(0,0);
        }
        xSemaphoreGive( UISemaphore );
    }
    */
    lUILog("Event App launch end\n");
}

/*
 * Update touch and screen 
 */
static void UIEventScreenRefresh(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    if ( systemSleep ) { return; }
    // touch data handling
    static bool oldTouchState = false;

    int16_t newTouchX,newTouchY;
    bool newTouch=false;
    if( xSemaphoreTake( I2cMutex, LUNOKIOT_UI_SHORT_WAIT) == pdFALSE )  { return; }
    #ifdef LILYGO_DEV
        newTouch = ttgo->getTouch(newTouchX,newTouchY);
    #elif defined(M5_DEV)
        Point currPt = M5.Touch.getPressPoint();
        newTouch = currPt.valid();
        newTouchX = currPt.x;
        newTouchY = currPt.y;
    #endif
    xSemaphoreGive( I2cMutex );
    bool updateCoords=true;
    if ( ( !oldTouchState ) && ( newTouch ) ) { // thumb in
        touchDragAngle = 0;
        touchDragDistance = 0;
        touchDragVectorX=0;
        touchDragVectorY=0;
        touchDownTimeMS = millis();

        downTouchX = newTouchX;
        downTouchY = newTouchY;
    } else if ( ( oldTouchState ) && ( newTouch ) ) { // meanwhile have thumb...
        // obtain vector
        touchDragVectorX = (newTouchX-downTouchX);
        touchDragVectorY = (newTouchY-downTouchY);
        // obtain distance between two points
        touchDragDistance = sqrt(pow(touchDragVectorX, 2) + pow(touchDragVectorY, 2) * 1.0);
        // Obtain the angle from vector in degrees
        double radAngle = atan2(touchDragVectorY, touchDragVectorX);
        touchDragAngle = radAngle * (180/M_PI);
    } else {
        touchDownTimeMS=0;
        updateCoords=false;
        /*
        touchDragAngle = 0;
        touchDragDistance = 0;
        touchDragVectorX=0;
        touchDragVectorY=0;
        */
        //downTouchX=0;
        //downTouchY=0;
    }
    oldTouchState = touched;
    // make public the touch current data
    touched = newTouch;
    if ( updateCoords ) {
        touchX = newTouchX;
        touchY = newTouchY;
    }

    // try to get UI lock
    bool changes = false;
    if( xSemaphoreTake( UISemaphore, LUNOKIOT_UI_SHORT_WAIT) != pdTRUE )  { return; };
    // no app?, nothing to do
    if ( nullptr == currentApplication ) { xSemaphoreGive( UISemaphore ); return; }
    xSemaphoreGive( UISemaphore );

    // verify long tap to launch TaskSwitcher
    if ( 0 == touchDownTimeMS ) { // other way to detect touch :P
        UIlongTap=false;
    } else { 
        if  ( 0.0 == touchDragDistance ) {
            // touch full-system reactors, long tap to show the task list
            unsigned long pushTime = millis() - touchDownTimeMS; // how much time?
            if (( false == UILongTapOverride ) && ( pushTime > LUNOKIOT_TOUCH_LONG_MS )&&( false == UIlongTap )) {
                UIlongTap=true; // locked until thumb up
                lEvLog("UI: Touchpad long tap\n");
                // announce haptic on launch the long tap task
                /*
                #ifdef LILYGO_WATCH_2020_V3
                if( xSemaphoreTake( I2cMutex, LUNOKIOT_UI_SHORT_WAIT) == pdFALSE )  { return; }
                    ttgo->motor->onec(80);
                xSemaphoreGive( I2cMutex );
                #endif
                */

                //@TODO here is a good place to draw a circular menu and use the finger as arrow
                // Up tasks
                // Down appointments
                // Left email
                // Right notifications...
                // and more...
                
                if ( ( false == UILongTapOverride ) && ( nullptr != currentApplication ) && ( 0 == strcmp(currentApplication->AppName(),"Task switcher" ) ) ) {
                    LaunchWatchface(); // return to user configured watchface
                } // else { LaunchApplication(new TaskSwitcher()); }
                //xSemaphoreGive( UIDrawProcess );
                return;
            }
        }
    }
    esp_task_wdt_reset();
    // if screen-keyboard is enabled, send the Tick like normal application
    if ( nullptr != keyboardInstance ) {
        changes = keyboardInstance->Tick();
    } else {
        if ( false == UIlongTap ) { // only if no long tap in progress
            esp_task_wdt_reset();
            // Remove temporal watchdog here
            esp_err_t susbcribed = esp_task_wdt_status(NULL);
            if ( ESP_OK == susbcribed) { esp_task_wdt_delete(NULL); }
            changes = currentApplication->Tick();
            if ( ESP_OK == susbcribed) { esp_task_wdt_add(NULL); }
            //esp_task_wdt_add(NULL);
        }
    }
    esp_task_wdt_reset();
    if ( directDraw ) { changes=false; } // directDraw overrides application normal redraw
    if( xSemaphoreTake( UISemaphore, LUNOKIOT_UI_SHORT_WAIT) != pdTRUE )  { return; };
    if ( changes ) {
        if ( nullptr != keyboardInstance ) {
            keyboardInstance->canvas->pushSprite(0,0);
        } else {
            // get the current app view
            TFT_eSprite *appView = currentApplication->canvas;
            if ( nullptr != appView ) {
                appView->pushSprite(0,0); // push appView to tft
            }
        }
    }
    xSemaphoreGive( UISemaphore );
}

static void UIEventStop(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    lUILog("UIEventStop\n");
    xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    UIRunning = false;
    xSemaphoreGive( UISemaphore );
}
static void UIEventContinue(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    lUILog("UIEventContinue\n");
    xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    UIRunning = true;
    xSemaphoreGive( UISemaphore );
}

static void UIEventScreenTimeout(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    // touch restart the timeout
    if ( touched ) {
        UINextTimeout = millis()+UITimeout;
        return;
    }
    // if vbus get energy don't sleep
    if ( vbusPresent ) {
        //lUILog("@TODO disabled 'always on' when VBUS present\n");
        //UINextTimeout = millis()+UITimeout;
        //return;
    }

    // on timeout?
    if ( UINextTimeout > millis() ) { return; }

    // get a nap!
#ifdef LILYGO_DEV
    if ( ttgo->bl->isOn() ) {
#endif
        ScreenSleep();
        esp_event_post_to(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_STOP, nullptr, 0, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        DoSleep();
#ifdef LILYGO_DEV
    }
#endif
}

static void UIReadyEvent(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    lUILog("lunokIoT: UI event loop running\n");
}

TaskHandle_t UITickTaskHandler = NULL;              // save the handler
static void UITickTask(void* args) {
    FPS = MAXFPS;
    TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
    while(true) {
        const TickType_t LUNOKIOT_UI_TICK_TIME = (FPS / portTICK_PERIOD_MS); // try to get the promised speed
        BaseType_t isDelayed = xTaskDelayUntil( &nextCheck, LUNOKIOT_UI_TICK_TIME ); // wait a ittle bit

        if ( false == UIRunning ) { continue; }
        if ( systemSleep ) { continue; }
#ifdef LILYGO_DEV
        if ( false == ttgo->bl->isOn() ) { continue; } // do not send UI tick when screen is off
#endif
        esp_err_t what = esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_TICK, nullptr, 0, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS);
        // Dynamic FPS... the apps receives less events per second
        if (( ESP_OK != what )||( isDelayed )) {
            //FPS--;
            if ( FPS < 1 ) { FPS=1; }
        }
    }
    lEvLog("UITickTask: is dead!!!\n");
    vTaskDelete(NULL);
}

StaticTask_t UITickBuffer;
StackType_t UITickStack[LUNOKIOT_TINY_STACK_SIZE];
void UITickStart() { // @TODO THIS CAN BE REPLACED BY A Ticker.attach_ms
    if ( NULL == UITickTaskHandler ) {
        UITickTaskHandler = xTaskCreateStaticPinnedToCore( UITickTask,
                                                "lUITick",
                                                LUNOKIOT_TINY_STACK_SIZE,
                                                nullptr,
                                                tskIDLE_PRIORITY,
                                                UITickStack,
                                                &UITickBuffer,
                                                UICORE);

        //BaseType_t res = xTaskCreatePinnedToCore(UITickTask, "lUITick", LUNOKIOT_APP_STACK_SIZE, NULL,uxTaskPriorityGet(NULL), &UITickTaskHandler,1);
        lUILog("Tick started\n");
        return;
    }
    eTaskState currStat = eTaskGetState(UITickTaskHandler);
    if ( eSuspended == currStat ) {
        vTaskResume(UITickTaskHandler);
        lUILog("Tick resume\n");
    }
}

void UITickEnd() { // @TODO this can be replaced by a Ticker stop
    if ( NULL != UITickTaskHandler ) {
        eTaskState currStat = eTaskGetState(UITickTaskHandler);
        if ( eRunning == currStat ) {
            vTaskSuspend(UITickTaskHandler);
            lUILog("Tick suspended\n");
        }
        //vTaskDelete(UITickTaskHandler);
        //UITickTaskHandler=NULL;
    }
}

static void UITickStartCallback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    UITickStart();
}

static void UITickStopCallback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    UITickEnd();
}

/*
 * UIRefresh force applications to refresh their UI
 */
void UIRefresh() {
    esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_REFRESH,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
}

/*
 * UIStart begin the UI loop, to handle UI events
 */
void UIStart() {
    // create the UI event loop
    esp_event_loop_args_t uiEventloopConfig = {
        .queue_size = 32,   // so much, but with multitask delays... maybe this is the most easy
        .task_name = "uiTask", // task will be created
        .task_priority = UIPRIORITY, // priorize UI over rest of system (responsive UI)
        .task_stack_size = LUNOKIOT_APP_STACK_SIZE, // normal arduinofw setup task stack
        .task_core_id = UICORE, //tskNO_AFFINITY // PRO_CPU // APP_CPU
    };

    UINextTimeout = millis()+UITimeout; // poweroff the screen when no activity timeout

    // Create the event loops
    esp_err_t uiLoopRc = esp_event_loop_create(&uiEventloopConfig, &uiEventloopHandle);
    if ( ESP_OK !=  uiLoopRc ) {
        lUILog("ERROR: Unable to create event loop: '%s' ABORTING UI!\n",esp_err_to_name(uiLoopRc));
        return;
    }
    // Register the handler for task iteration event. Notice that the same handler is used for handling event on different loops.
    // The loop handle is provided as an argument in order for this example to display the loop the handler is being run on.
    uiLoopRc = esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_TICK, UIEventScreenTimeout, nullptr, NULL);
    uiLoopRc = esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_READY, UIReadyEvent, nullptr, NULL);
    uiLoopRc = esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_TICK, UIEventScreenRefresh, nullptr, NULL);


    uiLoopRc = esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_APP_LAUNCH, UIEventLaunchApp, nullptr, NULL);
    uiLoopRc = esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_APP_LAUNCH_END, UIEventLaunchAppEnd, nullptr, NULL);
    //    esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_ANCHOR2D_CHANGE, UIAnchor2DChange, nullptr, NULL);

    // Useless if is sended later than whake x'D
    //    esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_CONTINUE, UIEventContinue, nullptr, NULL);
    //    esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_STOP, UIEventStop, nullptr, NULL);
    

    uiLoopRc = esp_event_handler_instance_register_with(uiEventloopHandle, UI_EVENTS, UI_EVENT_READY, UITickStartCallback, nullptr, NULL);

    // disable ticks
    uiLoopRc = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_LIGHTSLEEP, UITickStopCallback, nullptr, NULL);
    // enable ticks
    uiLoopRc = esp_event_handler_instance_register_with(systemEventloopHandler, SYSTEM_EVENTS, SYSTEM_EVENT_WAKE, UITickStartCallback, nullptr, NULL);

    uiLoopRc = esp_event_post_to(uiEventloopHandle, UI_EVENTS, UI_EVENT_READY,nullptr, 0, LUNOKIOT_EVENT_TIME_TICKS);
}

// Implementing Mid-Point Circle Drawing Algorithm
void DescribeCircle(int x_centre, int y_centre, int r, DescribeCircleCallback callback, void *payload) {
    int x = r, y = 0;
    int step = 0;
    bool stop = false;
    // Printing the initial point on the axes
    // after translation
    {
        int curx = x + x_centre;
        int cury = y + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
        double radAngle = atan2(fromCenterX, fromCenterY);
        double dangle = radAngle * (180/M_PI);
        int angle = int(180-dangle) % 360;
        stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
        if ( false == stop ) { return; }
        step++;
    }
    //printf("(%d, %d) ", x + x_centre, y + y_centre);
    
    // When radius is zero only a single
    // point will be printed
    if (r > 0) {
        {
            int curx = x + x_centre;
            int cury = -y + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/M_PI);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; }
            step++;
        }
        //printf("(%d, %d) ", x + x_centre, -y + y_centre);
        {
            int curx = y + x_centre;
            int cury = x + y_centre;
            //int fromCenterX = (curx-120);
            //int fromCenterY = (cury-120);
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/M_PI);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; }
            step++;
        }
        //printf("(%d, %d) ", y + x_centre, x + y_centre);
        {
            int curx = -y + x_centre;
            int cury = x + y_centre;
            //int fromCenterX = (curx-120);
            //int fromCenterY = (cury-120);
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/M_PI);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; }
            step++;
        }
        //printf("(%d, %d)\n", -y + x_centre, x + y_centre);
    }
     
    // Initialising the value of P
    int P = 1 - r;
    while (x > y)
    {
        y++;
         
        // Mid-point is inside or on the perimeter
        if (P <= 0)
            P = P + 2*y + 1;
             
        // Mid-point is outside the perimeter
        else
        {
            x--;
            P = P + 2*y - 2*x + 1;
        }
         
        // All the perimeter points have already been printed
        if (x < y)
            break;
         
        // Printing the generated point and its reflection
        // in the other octants after translation
        {
            int curx = x + x_centre;
            int cury = y + y_centre;
            //int fromCenterX = (curx-120);
            //int fromCenterY = (cury-120);
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/M_PI);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; } // end angle serch
            step++;
        }
        //printf("(%d, %d) ", x + x_centre, y + y_centre);
        {
            int curx = -x + x_centre;
            int cury = y + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/M_PI);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; } // end angle serch
            step++;
        }
        //printf("(%d, %d) ", -x + x_centre, y + y_centre);
        {
            int curx = x + x_centre;
            int cury = -y + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/M_PI);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; } // end angle serch
            step++;
        }
        //printf("(%d, %d) ", x + x_centre, -y + y_centre);
        {
            int curx = -x + x_centre;
            int cury = -y + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
            double radAngle = atan2(fromCenterX, fromCenterY);
            double dangle = radAngle * (180/M_PI);
            int angle = int(180-dangle) % 360;
            stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
            if ( false == stop ) { return; } // end angle serch
            step++;
        }
        //printf("(%d, %d)\n", -x + x_centre, -y + y_centre);
         
        // If the generated point is on the line x = y then
        // the perimeter points have already been printed
        if (x != y)
        {
            {
                int curx = y + x_centre;
                int cury = x + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
                double radAngle = atan2(fromCenterX, fromCenterY);
                double dangle = radAngle * (180/M_PI);
                int angle = int(180-dangle) % 360;
                stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
                if ( false == stop ) { return; } // end angle serch
                step++;
            }
            //printf("(%d, %d) ", y + x_centre, x + y_centre);
            {
                int curx = -y + x_centre;
                int cury = x + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);

                double radAngle = atan2(fromCenterX, fromCenterY);
                double dangle = radAngle * (180/M_PI);
                int angle = int(180-dangle) % 360;
                stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
                if ( false == stop ) { return; } // end angle serch
                step++;
            }
            //printf("(%d, %d) ", -y + x_centre, x + y_centre);
            {
                int curx = y + x_centre;
                int cury = -x + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
                double radAngle = atan2(fromCenterX, fromCenterY);
                double dangle = radAngle * (180/M_PI);
                int angle = int(180-dangle) % 360;
                stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
                if ( false == stop ) { return; } // end angle serch
                step++;
            }
            //printf("(%d, %d) ", y + x_centre, -x + y_centre);
            {
                int curx = -y + x_centre;
                int cury =  -x + y_centre;
        int fromCenterX = (curx-x_centre);
        int fromCenterY = (cury-y_centre);
                double radAngle = atan2(fromCenterX, fromCenterY);
                double dangle = radAngle * (180/M_PI);
                int angle = int(180-dangle) % 360;
                stop = callback(curx, cury, x_centre, y_centre, angle, step, payload);
                if ( false == stop ) { return; } // end angle serch
                step++;
            }
            //printf("(%d, %d)\n", -y + x_centre, -x + y_centre);
        }
    }
}

int GetAngleFromVector(int vX, int vY) {

    double radAngle = atan2(vX, vY);
    double dangle = radAngle * (180/M_PI);
    int angle = int(180-dangle) % 360;

    /*
    double radAngle = atan2(vY, vX);
    int angle = radAngle * (180/M_PI);
    */
    //lLog("Angle: %d\n", angle);
    return angle;
}

void DescribeCircle2(int x_centre, int y_centre, int r, DescribeCircleCallback callback, void *payload) {
    int x = -r, y = 0, err = 2-2*r; /* II. Quadrant */ 
    bool stop;
    int step = 0;
    int angle,px,py;
    int fromCenterX,fromCenterY;
    do {
        //setPixel(xm-x, ym+y); /*   I. Quadrant */
        px=(x_centre-x);
        py=(y_centre+y);
        fromCenterX = (px-x_centre);
        fromCenterY = (py-y_centre);
        angle = GetAngleFromVector(fromCenterX,fromCenterY);
        callback(px, py, x_centre, y_centre, angle, step, payload);
        step++;

        //setPixel(xm-y, ym-x); /*  II. Quadrant */
        px=(x_centre-y);
        py=(y_centre-x);
        fromCenterX = (px-x_centre);
        fromCenterY = (py-y_centre);
        angle = GetAngleFromVector(fromCenterX,fromCenterY);
        callback(px, py, x_centre, y_centre, angle, step, payload);
        step++;

        //setPixel(xm+x, ym-y); /* III. Quadrant */
        px=(x_centre+x);
        py=(y_centre-y);
        fromCenterX = (px-x_centre);
        fromCenterY = (py-y_centre);
        angle = GetAngleFromVector(fromCenterX,fromCenterY);
        callback(px, py, x_centre, y_centre, angle, step, payload);
        step++;

        //setPixel(xm+y, ym+x); /*  IV. Quadrant */
        px=(x_centre+y);
        py=(y_centre+x);
        fromCenterX = (px-x_centre);
        fromCenterY = (py-y_centre);
        angle = GetAngleFromVector(fromCenterX,fromCenterY);
        callback(px, py, x_centre, y_centre, angle, step, payload);
        step++;

        r = err;
        if (r <= y) err += ++y*2+1;           /* e_xy+e_y < 0 */
        if (r > x || err > y) err += ++x*2+1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);
}

// https://zingl.github.io/bresenham.html
void DescribeLine(int x0, int y0, int x1, int y1, DescribeLineCallback callback, void *payload) {
    int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1; 
    int err = dx+dy, e2; /* error value e_xy */
    while(callback(x0, y0, payload)) {
        if (x0==x1 && y0==y1) break;
        e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }
}

// https://zingl.github.io/blurring.html
void BlurSprite(TFT_eSprite *view, int radius) { // first degree approximation
    lLog("@TODO BlurSprite isn't functional!\n");
    return;
    uint16_t buffer[radius]; // pixel buffer 
    for (int x = 0; x < view->width(); ++x) { // vertical blur
        long dif = 0, sum = 0;
        for (int y = 0-2*radius; y < view->height(); ++y) {
            sum += dif;            // accumulate pixel blur
            dif += view->readPixel(x, y+radius);   // next pixel
            if (y >= 0) {
                dif += buffer[y%radius];  // sum up differences: +1, -2, +1
                view->drawPixel(x, y, sum/(radius*radius)); // set blurred pixel
            }
            if (y+radius >= 0) {
                uint16_t p = view->readPixel(x,y);
                buffer[y%radius] = p;    // buffer pixel
                dif -= 2*p;
            }
        } // y
    } // x
    for (int y = 0; y < view->height(); ++y) { // horizontal blur...
        long dif = 0, sum = 0;
        for (int x = 0-2*radius; x < view->width(); ++x) {
            sum += dif;              // accumulate pixel blur
            dif += view->readPixel(x+radius, y);  // next pixel
            if (x >= 0) {
                dif += buffer[x%radius];  // sum up differences: +1, -2, +1
                view->drawPixel(x, y, sum/(radius*radius)); // set blurred pixel
            } 
            if (x+radius >= 0) {
                uint16_t p = view->readPixel(x,y);
                buffer[x%radius] = p;    // buffer pixel
                dif -= 2*p;
            }
        } // x
    } // y
} // blur
