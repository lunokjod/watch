#ifndef __LUNOKIOT__UI__BUTTON___
#define __LUNOKIOT__UI__BUTTON___

#include "../activator/ActiveRect.hpp"
#include "CanvasWidget.hpp"
#include <functional>
#include "../UI.hpp"

class ButtonWidget: public ActiveRect, public CanvasWidget {
    protected:
        bool lastPushed=false;
    public:
        CanvasWidget * buffer;

        bool Interact(bool touch, int16_t tx,int16_t ty);
        virtual ~ButtonWidget();

        void BuildCanvas(int16_t h, int16_t w);
        bool CheckBounds(); // rebuild the canvas if is changed (future animations/transitions) don't need to call directly
        ButtonWidget(int16_t x,int16_t y, int16_t h, int16_t w, std::function<void ()> notifyTo=nullptr, uint32_t btnBackgroundColor=ThCol(button), bool borders=true);

        virtual void InternalRedraw();
        virtual void DrawTo(TFT_eSprite * endCanvas);
        virtual void DirectDraw();

        bool borders=true; // want borders or only ActiveRect?
        uint32_t btnBackgroundColor; // optional color

};
#endif
