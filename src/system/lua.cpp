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
#include "SystemEvents.hpp"
#include <LuaWrapper.h>
#include "../app/Shutdown.hpp"
#include <LilyGoWatch.h>
#include "Application.hpp"

LuaWrapper lua;
static uint32_t RunningLuaScripts=0;

typedef struct {
    uint32_t id;
    LuaCallback callback;
    void * payload;
    String * program;
} LuaCallbackData;

extern "C" {
    // get mandatory: uint16_t color = luaL_checkinteger(lua_state, 1);
    // get optional: long  floorVal = luaL_optnumber(lua_state, 1,0.0);
    // return value: lua_pushnumber(lua_state, (lua_Number) millis());
    /* BLOCK LOG
    xSemaphoreTake( lLogAsBlockSemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
    lRawLog("LUA (%u): <Debug> FillSprite(0x%04x)\n",RunningLuaScripts,color);
    xSemaphoreGive( lLogAsBlockSemaphore );
    */


    static int lua_wrapper_Debug(lua_State *lua_state) {
        lRawLog("[LUA] Free System Heap: %d\n",esp_get_free_heap_size());
        lRawLog("[LUA] Free Task Heap:   %d\n",uxTaskGetStackHighWaterMark(NULL));
        return 0;
    }
    static int lua_wrapper_Restart(lua_State *lua_state) {
        LaunchApplication(new ShutdownApplication(true));
        return 0;
    }
    static int lua_wrapper_Shutdown(lua_State *lua_state) {
        LaunchApplication(new ShutdownApplication());
        return 0;
    }
    static int lua_wrapper_LaunchWatchface(lua_State *lua_state) { LaunchWatchface(); return 0; }
    static int lua_wrapper_DoSleep(lua_State *lua_state) { ScreenSleep(); DoSleep(); return 0; }
    static int lua_wrapper_ScreenSleep(lua_State *lua_state) { ScreenSleep(); return 0; }
    static int lua_wrapper_ScreenWake(lua_State *lua_state) { ScreenWake(); return 0; }

    static int lua_wrapper_delay(lua_State *lua_state) {
        TickType_t a = luaL_checknumber(lua_state, 1);                        // get value from first param
        TickType_t nextCheck = xTaskGetTickCount();                     // get the current ticks
        xTaskDelayUntil( &nextCheck, (a / portTICK_PERIOD_MS) ); // wait desired time
        return 0;
    }
    static int lua_wrapper_millis(lua_State *lua_state) {
        lua_pushnumber(lua_state, (lua_Number) millis());
        return 1;
    }
    static int lua_wrapper_random(lua_State *lua_state) {
        long  floorVal = luaL_optnumber(lua_state, 1,0.0);
        long  maxVal = luaL_optnumber(lua_state, 2,1.0);
        lua_pushnumber(lua_state, (lua_Number) random(floorVal,maxVal));
        return 1;
    }

    static int lua_wrapper_tft_fillScreen(lua_State *lua_state) {
        uint16_t color = luaL_checkinteger(lua_state, 1);
        xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        //LunokIoTApplication * current = currentApplication;
        if ( nullptr != currentApplication ) {
            currentApplication->canvas->fillSprite(color);
            currentApplication->dirty=true;
        }
        UINextTimeout = millis()+UITimeout; // disable screen timeout
        xSemaphoreGive( UISemaphore );
        return 0;
    }

    static int lua_wrapper_tft_drawCircle(lua_State *lua_state) {
        int32_t x0 = luaL_checkinteger(lua_state, 1);
        int32_t y0 = luaL_checkinteger(lua_state, 2);
        int32_t r = luaL_checkinteger(lua_state, 3);
        uint32_t color = luaL_checkinteger(lua_state, 4);
        luaL_checktype(lua_state, 5, LUA_TBOOLEAN);
        bool fill = lua_toboolean(lua_state, 5);

        xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        //LunokIoTApplication * current = currentApplication;
        if ( nullptr != currentApplication ) {
            if ( fill ) {
                currentApplication->canvas->fillCircle(x0,y0,r,color);
            } else {
                currentApplication->canvas->drawCircle(x0,y0,r,color);
            }
            currentApplication->dirty=true;
        }
        UINextTimeout = millis()+UITimeout; // disable screen timeout
        xSemaphoreGive( UISemaphore );
        return 0;
    }
    
    static int lua_wrapper_tft_setTextColor(lua_State *lua_state) {
        uint32_t color = luaL_checkinteger(lua_state, 1);
        xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        //LunokIoTApplication * current = currentApplication;
        if ( nullptr != currentApplication ) {
            currentApplication->canvas->setTextColor(color);
        }
        xSemaphoreGive( UISemaphore );
        return 0;
    }

    static int lua_wrapper_tft_setTextSize(lua_State *lua_state) {
        uint32_t tsize = luaL_checkinteger(lua_state, 1);
        xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        //LunokIoTApplication * current = currentApplication;
        if ( nullptr != currentApplication ) {
            currentApplication->canvas->setTextSize(tsize);
        }
        xSemaphoreGive( UISemaphore );
        return 0;
    }
    
    static int lua_wrapper_tft_drawText(lua_State *lua_state) {
        int32_t x0 = luaL_checkinteger(lua_state, 1);
        int32_t y0 = luaL_checkinteger(lua_state, 2);
        const char * what = luaL_checkstring(lua_state, 3);
        xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        //LunokIoTApplication * current = currentApplication;
        if ( nullptr != currentApplication ) {
            /* @TODO
            currentApplication->canvas->setFreeFont(&FreeMonoBold9pt7b);
            currentApplication->canvas->setTextDatum(BC_DATUM);
            */
            currentApplication->canvas->drawString(what,x0,y0);
            currentApplication->dirty=true;
        }
        UINextTimeout = millis()+UITimeout; // disable screen timeout
        xSemaphoreGive( UISemaphore );
        return 0;
    }

    static int lua_wrapper_tft_drawRect(lua_State *lua_state) {
        int32_t x0 = luaL_checkinteger(lua_state, 1);
        int32_t y0 = luaL_checkinteger(lua_state, 2);
        int32_t x1 = luaL_checkinteger(lua_state, 3);
        int32_t y1 = luaL_checkinteger(lua_state, 4);
        uint32_t color = luaL_checkinteger(lua_state, 5);
        luaL_checktype(lua_state, 6, LUA_TBOOLEAN);
        bool fill = lua_toboolean(lua_state, 6);

        xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        //LunokIoTApplication * current = currentApplication;
        if ( nullptr != currentApplication ) {
            if ( fill ) {
                currentApplication->canvas->fillRect(x0,y0,x1,y1,color);
            } else {
                currentApplication->canvas->drawRect(x0,y0,x1,y1,color);
            }
            currentApplication->dirty=true;
        }
        UINextTimeout = millis()+UITimeout; // disable screen timeout
        xSemaphoreGive( UISemaphore );
        return 0;
    }

    static int lua_wrapper_tft_drawLine(lua_State *lua_state) {
        int32_t x0 = luaL_checkinteger(lua_state, 1);
        int32_t y0 = luaL_checkinteger(lua_state, 2);
        int32_t x1 = luaL_checkinteger(lua_state, 3);
        int32_t y1 = luaL_checkinteger(lua_state, 4);
        uint32_t color = luaL_checkinteger(lua_state, 5);

        xSemaphoreTake( UISemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        //LunokIoTApplication * current = currentApplication;
        if ( nullptr != currentApplication ) {
            currentApplication->canvas->drawLine(x0,y0,x1,y1,color);
            currentApplication->dirty=true;
        }
        UINextTimeout = millis()+UITimeout; // disable screen timeout
        xSemaphoreGive( UISemaphore );
        return 0;
    }

    static int lua_wrapper_tft_rgbTFTColor(lua_State *lua_state) {
        uint8_t colorR = luaL_checkinteger(lua_state, 1);
        uint8_t colorG = luaL_checkinteger(lua_state, 2);
        uint8_t colorB = luaL_checkinteger(lua_state, 3);
        uint16_t color = tft->color565(colorR,colorG,colorB);
        lua_pushnumber(lua_state, (lua_Number) color);
        return 1;
    }


    static int lua_wrapper_lwatch_print(lua_State *L) {
        // manual blocking of log to print in block
        xSemaphoreTake( lLogAsBlockSemaphore, LUNOKIOT_EVENT_MANDATORY_TIME_TICKS);
        lRawLog("LUA (%u)(output): ",RunningLuaScripts);
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
            if (s == NULL) {
                xSemaphoreGive( lLogAsBlockSemaphore );
                return luaL_error(L, "'tostring' must return a string to 'print'");
            }
            if (i>1) { lRawLog(" "); }
            lRawLog("%s",s);
            lua_pop(L, 1);  /* pop result */
        }
        lRawLog("\n");
        xSemaphoreGive( lLogAsBlockSemaphore );
        return 0;
    }
}

// this is the defult callback when isn't set on LuaRun
void LuaEndDefaultCallback(const char* response, void *payload) {
    lLog("LUA (%u) End callback: no action\n",RunningLuaScripts);
};

void LuaInit() {
    // log and stdout functions
    lua.Lua_register("print", &lua_wrapper_lwatch_print);
    lua.Lua_register("log", &lua_wrapper_lwatch_print);

    // arduino general
    lua.Lua_register("millis", &lua_wrapper_millis);
    lua.Lua_register("delay", &lua_wrapper_delay);
    lua.Lua_register("random", &lua_wrapper_random);

    // system (lwatch) calls
    lua.Lua_register("LaunchWatchface", &lua_wrapper_LaunchWatchface);
    lua.Lua_register("ScreenSleep", &lua_wrapper_ScreenSleep);
    lua.Lua_register("ScreenWake", &lua_wrapper_ScreenWake);
    lua.Lua_register("DoSleep", &lua_wrapper_DoSleep);
    lua.Lua_register("Shutdown", &lua_wrapper_Shutdown);
    lua.Lua_register("Restart", &lua_wrapper_Restart);
    lua.Lua_register("Debug", &lua_wrapper_Debug);
    // draw primitives
    lua.Lua_register("FillScreen", &lua_wrapper_tft_fillScreen);
    lua.Lua_register("DrawLine", &lua_wrapper_tft_drawLine);
    lua.Lua_register("DrawCircle", &lua_wrapper_tft_drawCircle);
    lua.Lua_register("DrawRect", &lua_wrapper_tft_drawRect);
    lua.Lua_register("SetTextColor", &lua_wrapper_tft_setTextColor);
    lua.Lua_register("SetTextSize", &lua_wrapper_tft_setTextSize);
    lua.Lua_register("DrawText", &lua_wrapper_tft_drawText);
    lua.Lua_register("RGBTft", &lua_wrapper_tft_rgbTFTColor);
    // @TODO UI calls
    // @TODO GUI calls

}
void LuaRunTask(void *data) {
    LuaCallbackData * LuaTaskData = (LuaCallbackData*)data;
    const char * code = LuaTaskData->program->c_str();
    lLog("LUA (%u) -- Script dump:\n\n%s\nLUA (%u) -- Script dump end\n",LuaTaskData->id,code,LuaTaskData->id);
    const char * result = lua.Lua_dostring(LuaTaskData->program).c_str();
    lLog("LUA (%u) Script end: result: '%s'\n",LuaTaskData->id,result);

    //lLog("LUA (%u) Callback: %p result: %p\n",LuaTaskData->id,LuaTaskData->callback,result);
    if ( nullptr != LuaTaskData->callback ) {
        LuaCallback mustCall = LuaTaskData->callback;
        mustCall(result,LuaTaskData->payload);
    }
    delete LuaTaskData->program;
    delete LuaTaskData;
    RunningLuaScripts--;
    vTaskDelete(NULL);
}

void LuaRun(const String LuaProgram, LuaCallback callback, void *payload) {
    RunningLuaScripts++;
    LuaCallbackData * mypkg = new LuaCallbackData();
    if ( nullptr == callback ) { mypkg->callback = LuaEndDefaultCallback; }
    else { mypkg->callback = callback; }
    mypkg->payload=payload;
    mypkg->id = RunningLuaScripts;
    mypkg->program=new String(LuaProgram);
    xTaskCreatePinnedToCore(LuaRunTask, "luaScr", LUNOKIOT_TASK_STACK_SIZE, mypkg, LUAPRIORITY, NULL,LUACORE);
}
