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

#ifndef __LUNOKIOT__APPLICATION__BLEPLAYER__
#define __LUNOKIOT__APPLICATION__BLEPLAYER__
#include <Arduino_JSON.h>
#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class BLEPlayerApplication : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
        JSONVar lastPlayerInfoEntry;
        bool parsedInfoJSON=false;
        JSONVar lastPlayerStateEntry;
        bool parsedStateJSON=false;
        long difference=0;
        int dur=-1;
        int n=-1;
        int c=-1;
        int repeat=0;
        int shuffle=0;
        int position=-1;
        ButtonImageXBMWidget * playBtn = nullptr;
        ButtonImageXBMWidget * pauseBtn = nullptr;
        unsigned long nextDBRefresh=0;
        void DBRefresh();
        void TryGetCoverOfTite();
    public:
        SemaphoreHandle_t GetCoverLock = xSemaphoreCreateMutex();
        bool CoverObtained=false;
        char *lastTitle=nullptr;
        char *title=nullptr;
        char *receivedCoverBuffer = nullptr;
        const char * CoverURL = "http://www.omdbapi.com/?apikey=%s&t=%s";
        const char *AppName() override { return "BLE Player"; };
        BLEPlayerApplication();
        ~BLEPlayerApplication();
        bool Tick();
};

#endif
