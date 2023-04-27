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

#ifndef __LUNOKIOT__UI__ACTIVE__RECTAREA___
#define  __LUNOKIOT__UI__ACTIVE__RECTAREA___
/*
 * UI Class ActiveRect
 * 
 * Declare invisible rectangular area on screen (used to handle touch events)
 * Note: can be used as circular point using InRadius
 * 
 */
#include "../UI.hpp"

class ActiveRect {
    protected:
        bool lastInteraction = false;
        bool pushed=false;
    public:
        int16_t x;
        int16_t y;
        int16_t h;
        int16_t w;

        int16_t GetX() { return x; }
        void SetX(int16_t x) { this->x = x; }
        int16_t GetY() { return y; }
        void SetY(int16_t y) { this->y = y; }
        int16_t GetH() { return h; }
        void SetH(int16_t h) { this->h = h; }
        int16_t GetW() { return w; }
        void SetW(int16_t w) { this->w = w; }
        void SetEnabled(bool enabled) { this->enabled = enabled; }
        bool GetEnabled() { return enabled; }
        bool enabled = true;
        // info
        unsigned long currentTapTime = 0;
        uint32_t taskStackSize;

        // events
        ActiveRect(int16_t x=0,int16_t y=0, int16_t h=0, int16_t w=0,
                                        UICallback tapCallback=nullptr);
        bool Interact(bool touch, int16_t tx,int16_t ty, bool trigger=true); // t for "tap"

        // callbacks
        UICallback tapActivityCallback;
        void * paramCallback=nullptr;

        // tools
        void Trigger();
        static void _LaunchCallbackTask(void * callbackData);
        static bool InRect(int16_t px, int16_t py, int16_t x,int16_t y,int16_t h,int16_t w);
        static bool InRadius(int16_t px, int16_t py, int16_t x,int16_t y,int16_t r);
};

#endif
