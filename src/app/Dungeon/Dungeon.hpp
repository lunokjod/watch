#ifndef __LUNOKIOT__DUNGEONS_GAME_APP__
#define __LUNOKIOT__DUNGEONS_GAME_APP__

#include "../../system/Application.hpp"
#include "../../UI/widgets/CanvasWidget.hpp"
#include "../../UI/widgets/CanvasZWidget.hpp"

// used by map generator
typedef struct {
    bool running=false;         // is the service running?
    int16_t PlayerBeginX=0;
    int16_t PlayerBeginY=0;
    bool inProgress=false;    // in progress to be up
    bool valid=false;    
    uint8_t mapType=0;          // 0 at this time of code maturity :-( (only one procedural map generator type)
    uint16_t width=15;          // default map width
    uint16_t height=15;         // default height
    uint8_t rooms=2;            // number of rooms
    uint8_t roomRadius=0;       // want circular corner?
    uint8_t minRoomSize=4;      // minimum room size
    uint8_t maxRoomSize=6;     // maximum
    uint8_t horizontalPaths=1;  // number of tracks left to right
    uint8_t verticalPaths=1;    // tracks up down
    TFT_eSprite *floorMap=nullptr;
    TFT_eSprite *objectsMap=nullptr;
    TFT_eSprite *wallMap=nullptr;
    TFT_eSprite *topMap=nullptr;
    TFT_eSprite *darknessMap=nullptr;

} levelDescriptor;


const static char *LoadSentences[] = { 
    "loading...",
    "this can take a while",
    "gathering random...",
    "summoning demons...",
    "enslaving souls",
    "hiding spells...",
    "oah! a secret book?",
    "feeding cockroaches",
    "killing kitties...",
    "electing floor colors",
    "writing scroll of power",
    "cleaning healing bottles",
    "turning lead into gold",
    "poisoning powerups",
    "moisting walls...",
    "(phone rings loud)",
    "moma calling!?",
    "Yep?",
    "uh....",
    "mmm",
    "sure...",
    "mom, I'm on work!",
    "er... yes",
    "see you!",
    "(phone hungs)",
    "meh... slow load",
    "mmm a tabern...",
    "can see frogs on map?",
    "where is my dragon?",
    "",
    "cogito, ergo sum",
    "be patient!",
    "",
    "...",
    "",
    "Look behind you,","a Three-Headed","Monkey!!!!",
    "",
    "still here?",
    "",
    "lol!!! x'D",
    "",
    "mi mind","is going,","Dave",
    "",
    "Klaatu barada nikto",
    "",
    "oh kmon!!!",
    "ALL YOUR","BASE ARE","BELONG TO US",
    "Or be a Teapot!",
};

class DungeonGameApplication: public LunokIoTApplication {
    public:
        const float DragScale=1.0;
        const float HalfScale=1.0;
        const float NormalScale=1.0;
        unsigned long renderTime=1000/3;
        unsigned long animationTimeout=0;

        TFT_eSprite * lastShoot=nullptr;
        CanvasZWidget * gameScreen = nullptr;
        CanvasZWidget * composerLayer = nullptr;
        TFT_eSprite * dragScreenCopy=nullptr;

        bool loadedSpawnPoint=false;
        unsigned long waitGeneratorTimestamp=0;
        unsigned long waitGeneratorTimeout=0;
        levelDescriptor currentLevel;
        size_t currentSentence=0;
        bool dirty=true;
        void Redraw();
        void ManageSplashScreen();
    private:
        size_t generationRetries=0;
        bool lastThumb=false; //trick to force first redraw of gameScreen
        int16_t offsetX=0;
        int16_t offsetY=0;
        TaskHandle_t MapGeneratorHandle=NULL;
    protected:
        void NoticesBanner(const char *what);       // send messages to splash
        size_t PoToCXMap=0;                         // points to last processed coordinate X of map 
        size_t PoToCYMap=0;                         // points to last processed coordinate Y of map 
        static const size_t CNumTileProcess=6;     // maximum number of tiles processed simultaneously
 


        float lavaBackgroundScale = 1.0;
        bool lavaDirection=false;
        int16_t lavaAngle=0;
        CanvasZWidget * lavaBackground = nullptr;
        //CanvasWidget * layer0 = nullptr;
        //CanvasWidget * layer1 = nullptr;
        //CanvasWidget * layer2 = nullptr;
        //CanvasWidget * layer3 = nullptr;
        //CanvasWidget * shadowLayer = nullptr;

    public:
        const uint8_t tileW       = 16; // the typical scenery size
        const uint8_t tileH       = 16;
        DungeonGameApplication();
        virtual ~DungeonGameApplication();
        bool Tick();
};

#endif
