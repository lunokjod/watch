#ifndef __LUNOKIOT__UI__SWITCH___
#define __LUNOKIOT__UI__SWITCH___

#include "../activator/ActiveRect.hpp"  // parents
#include "CanvasWidget.hpp"
#include <functional>                   // callbacks/lambda
#include "../UI.hpp"                    // theme

class SwitchWidget: public ActiveRect, public CanvasWidget {
    protected:
        bool lastInteract=false;
    public:
        CanvasWidget * buffer=nullptr;
        void SetEnabled(bool enabled);
        bool Interact(bool  touch, int16_t tx,int16_t ty);
        virtual ~SwitchWidget();

        SwitchWidget(int16_t x, int16_t y, UICallback notifyTo=nullptr, uint32_t switchColor=ThCol(button));

        virtual void InternalRedraw();
        virtual void DrawTo(TFT_eSprite * endCanvas);
        virtual void DirectDraw();

        volatile bool switchEnabled=false;
        uint32_t switchColor;
        const int32_t switchHeight = 20; //@TODO this must define canvas sizes

};
#endif
