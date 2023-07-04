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

#include "../UI/AppLuITemplate.hpp"
#include "LuaLauncher.hpp"
#include "LogView.hpp"
#include "../resources.hpp"
#include "../system/lua.hpp"
using namespace LuI;
const char * HelloworldLuaScript =
"\
log('Hello world from Lua script!')\n\
log('Fill screen with color')\n\
for i=1,3 do\n\
    local RandomBGColor = random(0,0xffff)\n\
    FillScreen(RandomBGColor)\n\
    delay(500);\n\
end\n\
log('Fill screen with color cycle')\n\
local color=0\n\
for x=0,240 do\n\
    DrawLine(x,0,x,240,RGBTft(color,color,color))\n\
    color=color+1\n\
end\n\
log('Fill screen with lines random')\n\
for i=1,100 do\n\
    local color = random(0,0xffff)\n\
    local x0 = random(0,240)\n\
    local y0 = random(0,240)\n\
    local x1 = random(0,240)\n\
    local y1 = random(0,240)\n\
    DrawLine(x0,y0,x1,y1,color)\n\
end\n\
log('Fill screen with circles random')\n\
for i=1,100 do\n\
    local color = random(0,0xffff)\n\
    local x0 = random(0,240)\n\
    local y0 = random(0,240)\n\
    local r = random(0,50)\n\
    DrawCircle(x0,y0,r,color)\n\
end\n\
log('See you!')\n\
";

LuaLauncher::LuaLauncher(const char * script) {
    LuaRun(script);
}
