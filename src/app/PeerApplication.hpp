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

#ifndef __LUNOKIOT__APPLICATION__PEER__
#define __LUNOKIOT__APPLICATION__PEER__

#include "../UI/AppTemplate.hpp"
#include <esp_now.h>
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/CanvasWidget.hpp"

class PeerApplication : public TemplateApplication {
    private:
        size_t selectedEmojiOffset = 0;
        unsigned long nextRefresh=0;
        //unsigned long nextTimestampSend=0;
        ActiveRect * iconPaletteArea;
        ButtonImageXBMWidget * sendEmojiBtn;
        ButtonImageXBMWidget * iconPaletteBtn;
        CanvasWidget * scrollZone;
        CanvasWidget * colorCanvas;
        CanvasWidget * emojiPaletteCanvas;
        bool emojiPaletteVisible=false;

    public:
        const char *AppName() override { return "Peer chat"; };
        PeerApplication();
        ~PeerApplication();
        static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
        static void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
        bool Tick();
};

#endif
