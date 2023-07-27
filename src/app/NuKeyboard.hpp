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

#ifndef __LUNOKIOT__NUKEYBOARD_APP__
#define __LUNOKIOT__NUKEYBOARD_APP__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/CanvasZWidget.hpp"
#include "../system/Datasources/perceptron.hpp"

class NuKeyboardApplication: public TemplateApplication {
    private:
        CanvasZWidget * handDrawCanvas=nullptr; 
        unsigned long nextRefresh=0;
        unsigned long nextCleanup=-1;
        const int32_t TouchRadius=18;
        //const float ScaleImage=0.025;  // 240x240 = 6x6
        const float ScaleImage=0.0334;  // 8x8
        const size_t PerceptronMatrixSize=7;
        static const size_t FrontLayer=16;
        static const size_t InternalLayer=32;
        static const size_t OutLayer=1;
        Perceptron *frontLayer[FrontLayer];
        Perceptron *hiddenLayer[InternalLayer];
        Perceptron *outLayer[OutLayer];
    public:
        void Train();
        void Query();
        const char *AppName() override { return "Nu keyboard"; };
        NuKeyboardApplication();
        ~NuKeyboardApplication();
        bool Tick();
};

#endif
