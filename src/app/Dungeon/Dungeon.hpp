#ifndef __LUNOKIOT__DUNGEONS_GAME_APP__
#define __LUNOKIOT__DUNGEONS_GAME_APP__

#include "../../system/Application.hpp"
#include "../../UI/widgets/CanvasWidget.hpp"

// used by map generator
typedef struct {
    bool inProgress=false;
    bool valid=false;
    uint8_t mapType=0;          // 0 at this time of code maturity :-( (only one procedural map generator type)
    uint16_t width=80;          // default map width
    uint16_t height=80;         // default height
    uint8_t rooms=21;            // number of rooms
    uint8_t roomRadius=0;       // want circular corner?
    uint8_t minRoomSize=4;      // minimum room size
    uint8_t maxRoomSize=12;     // maximum
    uint8_t horizontalPaths=3;  // number of tracks left to right
    uint8_t verticalPaths=3;    // tracks up down
    TFT_eSprite *floorMap=nullptr;
    TFT_eSprite *objectsMap=nullptr;
    TFT_eSprite *wallMap=nullptr;
    TFT_eSprite *topMap=nullptr;
    TFT_eSprite *darknessMap=nullptr;

} levelDescriptor;


const static char *LoadSentences[] = { 
    "generating world...",
    "gathering random...",
    "summoning demons...",
    "enslaving souls",
    "writing scroll of power",
    "cleaning healing bottles",
    "turning lead into gold"
    "what about zombies?",
    "adding mistery...",
    "where is my dragon?",
    "poisoning powerups",
    "meh... slow load",
    "moisting walls...",
    "found frogs on map",
    "be patient!",
    "",
    "...",
    "",
    "Look behind you,",
    "a Three-Headed",
    "Monkey!!!!",
    "",
    "still here?",
    "",
    "lol!!! x'D",
    "",
    "mi mind",
    "is going,",
    "Dave",
    "",
    "Klaatu barada nikto"
    "",
};

class DungeonGameApplication: public LunokIoTApplication {
    public:
        unsigned long waitGeneratorTimestamp=0;
        unsigned long waitGeneratorTimeout=0;
        unsigned long forceRedrawTimeout=0;
        unsigned long animationTimeout=0;
        levelDescriptor currentLevel;
        size_t currentSentence=0;
        bool dirty=false;
    private:
        size_t generationRetries=0;
        bool lastThumb=true; //trick to force first redraw of gameScreen
        int16_t offsetX=0;
        int16_t offsetY=0;
        TaskHandle_t MapGeneratorHandle=NULL;
    protected:
        CanvasWidget * gameScreen = nullptr;
        CanvasWidget * layer0 = nullptr;
        CanvasWidget * layer1 = nullptr;
        CanvasWidget * layer2 = nullptr;
        CanvasWidget * layer3 = nullptr;
        CanvasWidget * shadowLayer = nullptr;

        const uint8_t tileW       = 16; // the typical scenery size
        const uint8_t tileH       = 16;
    public:
        DungeonGameApplication();
        virtual ~DungeonGameApplication();
        bool Tick();
};

#endif
