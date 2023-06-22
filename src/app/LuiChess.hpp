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

#ifndef __LUNOKIOT__APPLICATION__GAME_CHESS__
#define __LUNOKIOT__APPLICATION__GAME_CHESS__
#include "../UI/UI.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/controls/Image.hpp"

#include "../UI/AppLuITemplate.hpp"
#include "../system/Datasources/perceptron.hpp"

class ChessApplication : public TemplateLuIApplication {
    public:
        const uint16_t PIECE_PAWN_COLOR=TFT_WHITE;
        const uint16_t PIECE_ROOK_COLOR=TFT_RED;
        const uint16_t PIECE_KNIGHT_COLOR=TFT_GREEN;
        const uint16_t PIECE_BISHOP_COLOR=TFT_BLUE;
        const uint16_t PIECE_QUEEN_COLOR=TFT_YELLOW;
        const uint16_t PIECE_KING_COLOR=TFT_CYAN;
        CanvasWidget * boardData=nullptr;
        LuI::Image * userBoard=nullptr;
        const uint8_t TileSize=20;
        // input layer / hidden layer / output layer
        // input: 8x8 to see whole board
        // hidden: arbitrary reduction to 32 possibilities
        // output: piece selection (16), piece direction (2^3) = 0~6, piece step (2^3) = 0~8 
        // rook, knight, bishop, queen, king, bishop, knight, rook, 8x pawns 
        // https://en.wikipedia.org/wiki/Algebraic_notation_(chess)
        const size_t MAX_PERCEPTRONS = (8*8)+32+(16+3+3);
        Perceptron **brain; // all of them
        void StartGame();
        ~ChessApplication();
        ChessApplication();
        const char *AppName() override { return "Chess"; };
        bool Tick() override;
};

#endif
