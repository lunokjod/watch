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
#include "lua.hpp"

#include "../app/LogView.hpp"
#include <LuaWrapper.h>

LuaWrapper lua;
static uint8_t RunningLuaScripts=0;

typedef struct {
    LuaCallback callback;
    void * payload;
    String * program;
} LuaCallbackData;

extern "C" {


    static int lua_wrapper_millis(lua_State *lua_state) {
        lua_pushnumber(lua_state, (lua_Number) millis());
        return 1;
    }
    static int myFunction(lua_State *lua_state) {
        Serial.println("Hi from my C function");
        return 0;
    }

    static int lua_wrapper_lwatch_print(lua_State *L) {
        // no mutex here... ¿why?
        Serial.printf("LUA: ");
        int n = lua_gettop(L);  /* number of arguments */
        int i;
        lua_getglobal(L, "tostring");
        for (i=1; i<=n; i++) {
            const char *s;
            size_t l;
            lua_pushvalue(L, -1);  /* function to be called */
            lua_pushvalue(L, i);   /* value to print */
            lua_call(L, 1, 1);
            s = lua_tolstring(L, -1, &l);  /* get result */
            if (s == NULL)
            return luaL_error(L, "'tostring' must return a string to 'print'");
            if (i>1) Serial.printf("\t");
            Serial.printf(s);
            lua_pop(L, 1);  /* pop result */
        }
        Serial.printf("\n");
        return 0;
    }
}

void LuaInit() {
    lua.Lua_register("print", &lua_wrapper_lwatch_print);
    lua.Lua_register("log", &lua_wrapper_lwatch_print);
    lua.Lua_register("millis", &lua_wrapper_millis);
    lua.Lua_register("myFunction", &myFunction);
}
void LuaRunTask(void *data) {
    LuaCallbackData * LuaTaskData = (LuaCallbackData*)data;
    const char * code = LuaTaskData->program->c_str();
    lLog("LUA (%u) Code:\n%s\nLUA (%u) Code End\n",RunningLuaScripts,code,RunningLuaScripts);
    const char * result = lua.Lua_dostring(LuaTaskData->program).c_str();
    if ( nullptr != LuaTaskData->callback ) {
        LuaCallback mustCall = LuaTaskData->callback;
        mustCall(result,LuaTaskData->payload);
    }
    lLog("LUA (%u) Result: '%s'\n",RunningLuaScripts,result);
    delete LuaTaskData->program;
    delete LuaTaskData;
    RunningLuaScripts--;
    vTaskDelete(NULL);
}

void LuaRun(const String LuaProgram, LuaCallback callback, void *payload) {
    RunningLuaScripts++;
    LuaCallbackData * mypkg = new LuaCallbackData();
    mypkg->callback = callback;
    mypkg->payload=payload;
    mypkg->program=new String(LuaProgram);
    xTaskCreatePinnedToCore(LuaRunTask, "luaScr", LUNOKIOT_TASK_STACK_SIZE, mypkg, LUAPRIORITY, NULL,LUACORE);
}
