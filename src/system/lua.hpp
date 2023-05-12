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

#ifndef __LUNOKIOT__LUA__SUPPPORT___
#define __LUNOKIOT__LUA__SUPPPORT___

#include <Arduino.h>

const uint8_t LUACORE = 1;
const UBaseType_t LUAPRIORITY = tskIDLE_PRIORITY;
typedef std::function<void (const char* response, void *payload)> LuaCallback;

void LuaInit();
void LuaRun(const String LuaProgram, LuaCallback callback=nullptr, void *payload=nullptr);

#endif
