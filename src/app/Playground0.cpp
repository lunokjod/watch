#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Playground0.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/UI.hpp"

PlaygroundApplication0::~PlaygroundApplication0() {
}

PlaygroundApplication0::PlaygroundApplication0() {
}

typedef struct {
    PlaygroundApplication0* app;
    float angle;
    uint32_t color;
} useAsPayload;

bool DrawCircleCallback(int x,int y, int cx, int cy, int angle, int step, void* payload) {
    useAsPayload * data = (useAsPayload *)payload;
    PlaygroundApplication0 * self = (PlaygroundApplication0 *)data->app;
    if ( angle < data->angle ) {
        self->canvas->fillCircle(x,y,8,data->color);
    }
    return true;
}
bool PlaygroundApplication0::Tick() {
    if (millis() > nextRedraw ) {
        canvas->fillSprite(TFT_BLACK);
        useAsPayload payload = { this, degX, TFT_BLUE };
        DescribeCircle(120,120,80,DrawCircleCallback,&payload);
        payload = { this, degY, TFT_GREEN };
        DescribeCircle(120,120,60,DrawCircleCallback,&payload);
        payload = { this, degZ, TFT_RED };
        DescribeCircle(120,120,40,DrawCircleCallback,&payload);
        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}
