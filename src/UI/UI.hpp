#ifndef __LUNOKIOT__UI__DEFINE__
#define __LUNOKIOT__UI__DEFINE__

#include <Arduino.h>
#include <LilyGoWatch.h>
#include <esp_event_base.h>
#include <functional>
#include "../system/Application.hpp"

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

void AlertLedEnable(bool enabled, uint32_t ledColor = TFT_RED, uint32_t x = 120, uint32_t y = 20);
extern bool directDraw;
extern unsigned long UINextTimeout;
extern const unsigned long UITimeout;
void UIStart();
// create user event loop for UI events
// https://github.com/espressif/esp-idf/tree/v4.2.2/examples/system/esp_event/user_event_loops
// create thread for feed the event loop
// https://docs.espressif.com/projects/esp-idf/en/v4.2.2/esp32/api-reference/system/esp_event.html
// @NOTE try to add some kind of animations, like fades, movements... (maybe tomorrow)

#include "representation/Point2D.hpp"
void _UINotifyPoint2DChange(Point2D *point);

// bool DescribeCircleCallbackExample(int x, int y, int cx, int cy, int angle, int step, void * payload)
typedef std::function<bool(int, int, int, int, int, int, void *)> DescribeCircleCallback;
// Implementing Mid-Point Circle Drawing Algorithm
void DescribeCircle(int x_centre, int y_centre, int r, DescribeCircleCallback callback, void *payload = nullptr);

void ScreenSleep();
void ScreenWake();

TFT_eSprite *TakeScreenShoot();
TFT_eSprite *DuplicateSprite(TFT_eSprite *view);
TFT_eSprite *ScaleSprite(TFT_eSprite *view, float divisor = 0.5);
TFT_eSprite * ScaleSpriteMAX(TFT_eSprite *view, float divisor, int16_t maxW, int16_t maxH);

#define OVERLAY_TRANSPARENT 6
#define OVERLAY_WHITE 15
#endif
