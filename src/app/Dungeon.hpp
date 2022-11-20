#ifndef __LUNOKIOT__DUNGEONS_GAME_APP__
#define __LUNOKIOT__DUNGEONS_GAME_APP__

#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"

class DungeonGameApplication: public LunokIoTApplication {
    public:
        static const size_t GridSizeW = 10;
        static const size_t GridSizeH = 8;
    private:
        bool lastThumb=true; //trick to force first redraw of gameScreen
        int16_t offsetX=0;
        int16_t offsetY=0;
    protected:
        CanvasWidget * gameScreen = nullptr;
        CanvasWidget * gameScreen2 = nullptr;
        const uint8_t tileW       = 16; // the typical scenery size
        const uint8_t tileH       = 16;
        //const float tileScale     = 1.0; // the scale
        unsigned long nextRefresh = 0;
    public:
        DungeonGameApplication();
        virtual ~DungeonGameApplication();
        bool Tick();
};

#endif
