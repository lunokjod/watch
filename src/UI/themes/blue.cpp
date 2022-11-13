#include <Arduino.h>
#include <cstdio> 
#include "default.hpp"
#include "../UI.hpp"

static uint32_t BlueThemeColors[] = {
    0xd1dbe4,0xbac9d7,0xa3b7ca,0x8ca5bd,0x7593af,
    0x476f95,0x305d88,0x255481,0x1f4f7e,0x194a7a
};

struct TemplateColorPalette BlueThemeColorPalette = {
        .colors = BlueThemeColors,
        .size = sizeof(BlueThemeColors)/sizeof(uint32_t)-1
};

struct TemplateThemeScheme BlueThemeScheme = {
    .background = BlueThemeColorPalette.colors[9],
    .min = BlueThemeColorPalette.colors[0],
    .shadow = BlueThemeColorPalette.colors[7],
    .background_alt = BlueThemeColorPalette.colors[8],
    .button = BlueThemeColorPalette.colors[6],
    .darken = BlueThemeColorPalette.colors[8],
    .boot_splash_foreground = BlueThemeColorPalette.colors[8],
    .text_alt = BlueThemeColorPalette.colors[2],
    .middle = BlueThemeColorPalette.colors[4],
    .max = BlueThemeColorPalette.colors[0],
    .mark = BlueThemeColorPalette.colors[7],
    .highlight = BlueThemeColorPalette.colors[3],
    .text = BlueThemeColorPalette.colors[0],
    .light = BlueThemeColorPalette.colors[3],
    .boot_splash_background = BlueThemeColorPalette.colors[0],
    .dark = BlueThemeColorPalette.colors[6],
    .high = 0xff0000,
    .clock_hands_second = BlueThemeColorPalette.colors[7],
    .medium = BlueThemeColorPalette.colors[8],
    .low = BlueThemeColorPalette.colors[9]
};
