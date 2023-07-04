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

#include <Arduino.h>
#include <LilyGoWatch.h>
extern TTGOClass *ttgo; // ttgo lib
#include "LogView.hpp" // log capabilities

#include "Lamp.hpp"

LampApplication::LampApplication() { canvas->fillSprite(TFT_WHITE); }
bool LampApplication::Tick() {
    if ( ttgo->bl->isOn() ) { // is the screen ON? 
        if (  255 > ttgo->bl->getLevel() ) { ttgo->setBrightness(ttgo->bl->getLevel()+1); }
    }
    if ( touched ) { LaunchWatchface(); }
    return false;
}
LampApplication::~LampApplication() { SetUserBrightness(); }
