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

#ifndef __LUNOKIOT__DUNGEONS_GAME_APP__
#define __LUNOKIOT__DUNGEONS_GAME_APP__

#include "../../system/Application.hpp"
#include "../../UI/widgets/CanvasWidget.hpp"
#include "../../UI/widgets/CanvasZWidget.hpp"

class PlayerClass {
    private:
    protected:
        int16_t x=0;
        int16_t y=0;

    public:
        virtual bool Draw();
        void SetPos(int16_t x, int16_t y) {
            this->x = x;
            this->y = y;
        }
        PlayerClass() {};
        ~PlayerClass() {};
};

class ElfMalePlayer: public PlayerClass {
    public:
        bool Draw() { return false; }
};

// used by map generator
typedef struct {
    bool running=false;                 // is the service running?
    PlayerClass *player=nullptr;
    int16_t PlayerBeginX=0;
    int16_t PlayerBeginY=0;
    bool inProgress=false;    // in progress to be up
    bool valid=false;    
    uint8_t mapType=0;          // 0 at this time (only one procedural dungeon type map generator)

    /* big map */
    uint16_t width=220;          // default map width in tiles
    uint16_t height=140;         // default height
    uint16_t rooms=190;           // number of rooms
    uint8_t roomRadius=0;       // want circular corner? (lot of gitches)
    uint8_t minRoomSize=5;      // minimum room size
    uint8_t maxRoomSize=11;      // maximum
    uint8_t horizontalPaths=2;  // number of tracks left to right
    uint8_t verticalPaths=1;    // tracks up down
    uint16_t chestNumber=32;      // number of chests (empty/full/trap)
    uint16_t coinNumber=128;      // coins on level
    uint16_t cratesNumber=256;    // crates on level
    uint16_t columns=756;        // number of inner columns

    /* normal map
    uint16_t width=64;          // default map width in tiles
    uint16_t height=64;         // default height
    uint8_t rooms=16;           // number of rooms
    uint8_t roomRadius=0;       // want circular corner? (lot of gitches)
    uint8_t minRoomSize=5;      // minimum room size
    uint8_t maxRoomSize=7;      // maximum
    uint8_t horizontalPaths=2;  // number of tracks left to right
    uint8_t verticalPaths=2;    // tracks up down
    uint8_t chestNumber=8;      // number of chests (empty/full/trap)
    uint8_t coinNumber=32;      // coins on level
    uint8_t cratesNumber=28;    // crates on level
    uint8_t columns=48;
    */
    TFT_eSprite *floorMap=nullptr;
    TFT_eSprite *objectsMap=nullptr;
    TFT_eSprite *wallMap=nullptr;
    TFT_eSprite *topMap=nullptr;
    TFT_eSprite *darknessMap=nullptr;

} levelDescriptor;



class DungeonGameApplication: public LunokIoTApplication {
    public:
        const char *AppName() override { return "Dungeon game"; };
        //const float NormalScale=0.5; // 32x32 (unusable)
        //const float NormalScale=1.25; // 12x12
        //const float NormalScale=1.666; // 9x9
        //const float NormalScale=1.85; // 8x8
        const float NormalScale=2.5; // 6x6
        unsigned long renderTime=1000/2;
        unsigned long animationTimeout=0;

        bool zeroSet = false;
        float zeroDegX = 0;
        float zeroDegY = 0;
        float zeroDegZ = 0;

        //TFT_eSprite * lastShoot=nullptr;

        CanvasZWidget * gameScreen = nullptr;
        // double buffer
        TFT_eSprite * lastGameScreen = nullptr;
        // level layers
        CanvasZWidget * objectLayer = nullptr;
        CanvasZWidget * floorLayer = nullptr;
        CanvasZWidget * wallLayer = nullptr;
        CanvasZWidget * topLayer = nullptr;

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
        //static const size_t CNumTileProcess=225;     // maximum number of tiles processed simultaneously
 

/*
        float lavaBackgroundScale = 1.0;
        bool lavaDirection=false;
        int16_t lavaAngle=0;
        CanvasZWidget * lavaBackground = nullptr;
        //CanvasWidget * layer0 = nullptr;
        //CanvasWidget * layer1 = nullptr;
        //CanvasWidget * layer2 = nullptr;
        //CanvasWidget * layer3 = nullptr;
        //CanvasWidget * shadowLayer = nullptr;
*/
    public:
        const uint8_t tileW       = 16; // the scenery tile size
        const uint8_t tileH       = 16;
        DungeonGameApplication();
        virtual ~DungeonGameApplication();
        bool Tick();
};


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
    "meh... slow load",
    "can see frogs on map?",
    "where is my dragon?",
    "",
    "(phone rings loud)",
    "moma calling!?",
    "Yep?",
    "uh....",
    "aha...",
    "yep...",
    "...",
    "sure...",
    "mom... I'm on work!",
    "...",
    "er...",
    "(love you too...)",
    "yes!",
    "ya ya...",
    "yep...",
    "se...",
    "(phone hungs)",
    "...",
    "Oh sh*t!",
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
#endif
