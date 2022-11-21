#ifndef __LUNOKIOT__DUNGEONS_GAME_APP__
#define __LUNOKIOT__DUNGEONS_GAME_APP__

#include "../../system/Application.hpp"
#include "../../UI/widgets/CanvasWidget.hpp"

// used by map generator
typedef struct {
    bool inProgress=false;
    bool valid=false;
    uint8_t mapType=0;          // 0 at this time of code maturity :-( (only one procedural map generator type)
    uint16_t width=40;          // default map width
    uint16_t height=40;         // default height
    uint8_t rooms=5;            // number of rooms
    uint8_t roomRadius=0;       // want circular corner?
    uint8_t minRoomSize=5;      // minimum room size
    uint8_t maxRoomSize=14;     // maximum
    uint8_t horizontalPaths=1;  // number of tracks left to right
    uint8_t verticalPaths=1;    // tracks up down
    TFT_eSprite *floorMap=nullptr;
    TFT_eSprite *wallMap=nullptr;
    TFT_eSprite *topMap=nullptr;

} levelDescriptor;


const static char *LoadSentences[] = { 
    "gathering random...",
    "building world...",
    "summoning demons...",
    "writing scroll of power",
    "healing bottles...",
    "what about zombies?",
    "adding mistery...",
    "learning to fly...",
    "making your life better",
};

class DungeonGameApplication: public LunokIoTApplication {
    public:
        unsigned long waitGeneratorTimestamp=0;
        unsigned long waitGeneratorTimeout=0;
        levelDescriptor currentLevel;
        size_t currentSentence=0;
    private:
        size_t generationRetries=0;
        bool lastThumb=true; //trick to force first redraw of gameScreen
        int16_t offsetX=0;
        int16_t offsetY=0;
        TaskHandle_t MapGeneratorHandle;
    protected:
        CanvasWidget * gameScreen = nullptr;
        CanvasWidget * layer0 = nullptr;
        CanvasWidget * layer1 = nullptr;
        CanvasWidget * layer2 = nullptr;

        const uint8_t tileW       = 16; // the typical scenery size
        const uint8_t tileH       = 16;
    public:
        DungeonGameApplication();
        virtual ~DungeonGameApplication();
        bool Tick();
};

#endif
