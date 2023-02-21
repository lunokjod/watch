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
#include <cstdio> 
#include "../UI.hpp"

const uint32_t DefaultThemeColors[] = { 0x212121,0x353e45,0x555f68,0x56818a,0xfcb61d,
                                        0xffffff,0x000000,0xff0000,0xffff00,0x00ff00 };
// why this? seems redundant... 
// tip: need iterate colors for show theme colors
const struct TemplateColorPalette DefaultThemeColorPalette = {
        .colors = DefaultThemeColors,
        .size = sizeof(DefaultThemeColors)/sizeof(uint32_t)
};

const struct TemplateThemeScheme DefaultThemeScheme = {
        .background = DefaultThemeColorPalette.colors[0],
        .min = DefaultThemeColorPalette.colors[0],
        .shadow = DefaultThemeColorPalette.colors[0],
        .background_alt = DefaultThemeColorPalette.colors[2],
        .button = DefaultThemeColorPalette.colors[1],
        .darken = DefaultThemeColorPalette.colors[1],
        .boot_splash_foreground = DefaultThemeColorPalette.colors[1],
        .text_alt = DefaultThemeColorPalette.colors[2],
        .middle = DefaultThemeColorPalette.colors[2],
        .max = DefaultThemeColorPalette.colors[3],
        .mark = DefaultThemeColorPalette.colors[3],
        .highlight = DefaultThemeColorPalette.colors[4],
        .text = DefaultThemeColorPalette.colors[5],
        .light = DefaultThemeColorPalette.colors[5],
        .boot_splash_background = DefaultThemeColorPalette.colors[5],
        .dark = DefaultThemeColorPalette.colors[6],
        .high = DefaultThemeColorPalette.colors[7],
        .clock_hands_second = DefaultThemeColorPalette.colors[7],
        .medium = DefaultThemeColorPalette.colors[8],
        .low = DefaultThemeColorPalette.colors[9]
};
