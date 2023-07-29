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

#ifndef __LUNOKIOT__UI__DEFINE__
#define __LUNOKIOT__UI__DEFINE__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include <esp_event_base.h>
#include <functional> // callbacks 

const uint8_t UICORE = 1;
const UBaseType_t UIPRIORITY = tskIDLE_PRIORITY+6;
const UBaseType_t UITRANSITIONPRIORITY = UIPRIORITY+6;

typedef std::function<void (void* payload)> UICallback; // repeated declaration
typedef std::function<void (void* payload,void* payload2)> UICallback2;
typedef std::function<void (void* payload,void* payload2,void* payload3)> UICallback3;

extern TFT_eSPI * tft;
//extern TTGOClass *ttgo; // ttgo lib
//#include "../system/Application.hpp"
// caller contains the object (must be converted)

#define MINIMUM_BACKLIGHT 5
void SetUserBrightness();
typedef struct {
    float a;
    float b;
    float c;
    float d;
} TransformationMatrix;

// A floating point
struct u32Point {
  uint32_t x;
  uint32_t y;
};

float CalcTriArea(u32Point *v1,u32Point *v2,u32Point *v3);
bool IsPointInTri(u32Point *pt, u32Point *v1, u32Point *v2, u32Point *v3);
bool isInside( int x, int y, int x1, int y1, int x2, int y2, int x3, int y3);
float area(int x1, int y1, int x2, int y2, int x3, int y3);
/*
 * Use template to get the theme colors
 */
struct TemplateColorPalette {
    const uint32_t *colors;
    const size_t size;
};

// associate TemplateColorPalette.colors offset point to this
struct TemplateThemeScheme {
        uint32_t background;
        uint32_t min;
        uint32_t shadow;
        uint32_t background_alt;
        uint32_t button;
        uint32_t darken;
        uint32_t boot_splash_foreground;
        uint32_t text_alt;
        uint32_t middle;
        uint32_t max;
        uint32_t mark;
        uint32_t highlight;
        uint32_t text;
        uint32_t light;
        uint32_t boot_splash_background;
        uint32_t dark;
        uint32_t high;
        uint32_t clock_hands_second;
        uint32_t medium;
        uint32_t low;
};

#include "themes/all_themes.hpp"

extern const TemplateColorPalette * currentColorPalette;
extern const TemplateThemeScheme * currentThemeScheme;

// color utilities
void GetRGBFrom16Bit(const uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b);
uint16_t ColorSwap(uint16_t colorToSwap);
uint16_t ByteSwap(uint16_t colorToSwap);

#define ThCol(WHAT) tft->color24to16(currentThemeScheme->WHAT)


extern uint8_t pendingNotifications;
/*
 * Convenient uint32_t to R/G/B union
 */
union rgbColor
{
    uint32_t u32;
    uint8_t rgb[3];
    uint8_t rgba[4];
    struct
    {
        uint8_t G;
        uint8_t R;
        uint8_t B;
    } color;
};

union rgb565
{
    uint16_t u16;
    uint8_t byte[2];
};

// declare the UI loop (system independent)
ESP_EVENT_DECLARE_BASE(UI_EVENTS);

// UI Events
enum
{
    UI_EVENT_READY,           // when the UI becomes active
    UI_EVENT_TICK,            // variable, depends of esp32 load
    UI_EVENT_CONTINUE,        // resume UI events please (get tick)
    UI_EVENT_STOP,            // stop all UI events please (stop tick)
    UI_EVENT_REFRESH,         // force application refresh
    UI_EVENT_APP_LAUNCH,      // announce system to show some "loading" thing meanwhile
    UI_EVENT_APP_LAUNCH_END,  // announce system to show hide "loading" thing meanwhile
    UI_EVENT_NOTIFICATION,    // @TODO
    UI_EVENT_ANCHOR2D_CHANGE, //@TODO
};

extern size_t screenShootCurrentImageX;
extern size_t screenShootCurrentImageY;

// Event loops

extern esp_event_loop_handle_t uiEventloopHandle;

/*
struct UICallbackData {
    LunokIoTApplication *application;
    void *callMe;
};
*/

//void AlertLedEnable(bool enabled, uint32_t ledColor = TFT_RED, uint32_t x = 120, uint32_t y = 20);
extern bool directDraw;
extern unsigned long UINextTimeout;
extern const unsigned long UITimeout;
void UIStart();
void UIRefresh();
// create user event loop for UI events
// https://github.com/espressif/esp-idf/tree/v4.2.2/examples/system/esp_event/user_event_loops
// create thread for feed the event loop
// https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-reference/system/esp_event.html
// @NOTE try to add some kind of animations, like fades, movements... (maybe tomorrow)

//#include "representation/Point2D.hpp"
//void _UINotifyPoint2DChange(Point2D *point);

// bool DescribeCircleCallbackExample(int x, int y, int cx, int cy, int angle, int step, void * payload)
typedef std::function<bool(int, int, int, int, int, int, void *)> DescribeCircleCallback;
typedef std::function<bool(int, int, void *)> DescribeLineCallback;
// Implementing Mid-Point Circle Drawing Algorithm
// https://zingl.github.io/bresenham.html <== awesome!
void DescribeCircle(int x_centre, int y_centre, int r, DescribeCircleCallback callback, void *payload = nullptr);
void DescribeCircle2(int x_centre, int y_centre, int r, DescribeCircleCallback callback, void *payload=nullptr);
void DescribeLine(int x0, int y0, int x1, int y1, DescribeLineCallback callback, void *payload=nullptr);
// https://zingl.github.io/blurring.html
void BlurSprite(TFT_eSprite *view, int radius);

void ScreenSleep();
void ScreenWake();

TFT_eSprite * GetSpriteRect(TFT_eSprite *view, int16_t x, int16_t y, int16_t h, int16_t w);
TFT_eSprite * DuplicateSprite(TFT_eSprite *view);
TFT_eSprite * ShearSprite(TFT_eSprite *view, TransformationMatrix transform);
TFT_eSprite * ScaleSprite(TFT_eSprite *view, float divisor = 0.5);
TFT_eSprite * ScaleSpriteMAX(TFT_eSprite *view, float divisor, int16_t maxW, int16_t maxH);

#endif
