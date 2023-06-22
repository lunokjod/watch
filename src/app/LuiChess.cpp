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
#include "../lunokiot_config.hpp"
#include "../UI/AppLuITemplate.hpp"
#include "LuiChess.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/Check.hpp"
#include "../UI/controls/IconMenu.hpp"
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/View3D/View3D.hpp"
#include "../UI/controls/Buffer.hpp"
#include "LogView.hpp"
#include "../UI/UI.hpp"
#include <LilyGoWatch.h>
#include "../UI/widgets/CanvasWidget.hpp"
using namespace LuI;
#include "../resources.hpp"

//#include "../system/Datasources/database.hpp"
#include "../system/Datasources/perceptron.hpp"

void ChessApplication::StartGame() {
    lAppLog("Start new game!\n");
    boardData->canvas->fillSprite(TFT_BLACK);
    // rook, knight, bishop, queen, king, bishop, knight, rook, 8x pawns

    // @TODO DISTINGUISH PLAYER BY COLOR
    lAppLog("Pushing pieces...\n");
    // rooks
    boardData->canvas->drawPixel(0,0,PIECE_ROOK_COLOR);
    boardData->canvas->drawPixel(0,7,PIECE_ROOK_COLOR);
    boardData->canvas->drawPixel(7,0,PIECE_ROOK_COLOR);
    boardData->canvas->drawPixel(7,7,PIECE_ROOK_COLOR);

    // knight
    boardData->canvas->drawPixel(1,0,PIECE_KNIGHT_COLOR);
    boardData->canvas->drawPixel(6,0,PIECE_KNIGHT_COLOR);
    boardData->canvas->drawPixel(6,7,PIECE_KNIGHT_COLOR);
    boardData->canvas->drawPixel(1,7,PIECE_KNIGHT_COLOR);

    // bishop
    boardData->canvas->drawPixel(2,0,PIECE_BISHOP_COLOR);
    boardData->canvas->drawPixel(5,0,PIECE_BISHOP_COLOR);
    boardData->canvas->drawPixel(2,7,PIECE_BISHOP_COLOR);
    boardData->canvas->drawPixel(5,7,PIECE_BISHOP_COLOR);

    // queen
    boardData->canvas->drawPixel(3,0,PIECE_QUEEN_COLOR);
    boardData->canvas->drawPixel(3,7,PIECE_QUEEN_COLOR);

    // king
    boardData->canvas->drawPixel(4,0,PIECE_KING_COLOR);
    boardData->canvas->drawPixel(4,7,PIECE_KING_COLOR);

    // pawns
    boardData->canvas->drawPixel(0,1,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(1,1,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(2,1,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(3,1,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(4,1,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(5,1,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(6,1,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(7,1,PIECE_PAWN_COLOR);

    boardData->canvas->drawPixel(0,6,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(1,6,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(2,6,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(3,6,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(4,6,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(5,6,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(6,6,PIECE_PAWN_COLOR);
    boardData->canvas->drawPixel(7,6,PIECE_PAWN_COLOR);
    //userBoard->dirty=true;
}

ChessApplication::ChessApplication() {
    directDraw=false;
    canvas->fillSprite(ThCol(background));
    Container * screen = new Container(LuI_Horizontal_Layout,2); 
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,2);
    // main view space
    Container * viewContainer = new Container(LuI_Horizontal_Layout,1);
    viewContainer->border=5;
    // add back button to dismiss
    Button *backButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
    backButton->border=0;
    backButton->tapCallback=[](void * obj){ LaunchWatchface(); }; // callback when tap
    // load icon in XBM format
    XBM * backButtonIcon = new XBM(img_backscreen_42_width,img_backscreen_42_height,img_backscreen_42_bits);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.3);
    boardData = new CanvasWidget(8,8);
    userBoard = new Image(TileSize*8,TileSize*8);
    StartGame();
    //userBoard->drawPushCallback = [&,this](IGNORE_PARAM) {
        lAppLog("Redraw board\n");
        TFT_eSprite * drawCanvas=userBoard->GetBuffer();
        //drawCanvas->fillSprite(ThCol(background));
        drawCanvas->fillSprite(ByteSwap(TFT_GREEN));
        drawCanvas->setTextFont(1);
        drawCanvas->setTextSize(3);
        drawCanvas->setTextDatum(TL_DATUM);
        uint8_t current=0;
        int32_t rY=0;
        bool whiteTile=true;
        for(int y=1;y<8;y++) {
            int32_t rX=0;
            for(int x=1;x<8;x++) {
                uint16_t color = TFT_BLACK;
                uint16_t textColor = TFT_WHITE;
                if ( whiteTile ) { color=TFT_WHITE; textColor=TFT_BLACK; }
                lAppLog("%u Floor Color 0x%04x %d %d\n",current,color,x-1,y-1);
                current++;
                drawCanvas->fillRect(rX,rY,TileSize,TileSize,color);
                drawCanvas->setTextColor(textColor);
                uint16_t currentDataColor = boardData->canvas->readPixel(x-1,y-1);
                if ( TFT_BLACK != currentDataColor ) {
                    lAppLog("  Piece Color 0x%04x %d %d\n",currentDataColor,x-1,y-1);
                    if (PIECE_PAWN_COLOR == currentDataColor ) { drawCanvas->drawString("P",rX,rY); }
                    else if ( PIECE_ROOK_COLOR == currentDataColor ) { drawCanvas->drawString("R",rX,rY); }
                    else if (PIECE_KNIGHT_COLOR == currentDataColor ) { drawCanvas->drawString("N",rX,rY); }
                    else if (PIECE_BISHOP_COLOR == currentDataColor ) { drawCanvas->drawString("B",rX,rY); }
                    else if (PIECE_QUEEN_COLOR == currentDataColor ) { drawCanvas->drawString("Q",rX,rY); }
                    else if (PIECE_KING_COLOR == currentDataColor ) { drawCanvas->drawString("K",rX,rY); }
                    else  { drawCanvas->drawString("O",rX,rY); }
                }
                whiteTile=(!whiteTile); // swap color on every column
                rX=TileSize*x;
            }
            whiteTile=(!whiteTile); // swap color on every line
            rY=TileSize*y;
        }
        drawCanvas->fillCircle(0,0,20,ByteSwap(TFT_RED));
    //};
    viewContainer->AddChild(userBoard);
    //userBoard->GetBuffer()->setPivot(20,5);
    //canvas->setPivot(canvas->width()/2,canvas->height()/2);
    //imageCanvas->setPivot(imageCanvas->width()/2,imageCanvas->height()/2);
    //userBoard->GetBuffer()->fillSprite(TFT_GREEN);
    screen->AddChild(viewContainer,1.65);
    screen->AddChild(bottomButtonContainer,0.35);

    AddChild(screen);
    size_t totalAllocations=0;
    brain=(Perceptron **)ps_calloc(MAX_PERCEPTRONS,sizeof(Perceptron *));
    totalAllocations+=MAX_PERCEPTRONS*sizeof(Perceptron *);
    for (int16_t y=0;y<MAX_PERCEPTRONS;y++) {
        //if ( nullptr == brain[y] ) {  }
        brain[y]=Perceptron_new(8*8, 0.2);
        totalAllocations+=sizeof(Perceptron);
    }
    totalAllocations+=sizeof(CanvasWidget);
    lAppLog("Allocating %u byte for %u perceptrons...\n",totalAllocations,MAX_PERCEPTRONS);
    //Tick();

    directDraw=true;
}

ChessApplication::~ChessApplication() {
    for (int16_t y=0;y<MAX_PERCEPTRONS;y++) {
        if ( nullptr != brain[y] ) { free(brain[y]); }
    }
    free(brain);
    delete boardData;
}

bool ChessApplication::Tick() {
    EventHandler();
    return false;
}
