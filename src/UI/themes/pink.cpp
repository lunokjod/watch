#include <Arduino.h>
#include <cstdio> 
#include "../UI.hpp"


const uint32_t PinkThemeColors[] = {
    0xc095e4,0xdec1eb,0xfcedf2,0xfedfe3,0xffd1d4,0xffb7c5,0xffacc5,0xffa0c5
};

const TemplateColorPalette PinkThemeColorPalette = {
        .colors = PinkThemeColors,
        .size = sizeof(PinkThemeColors)/sizeof(uint32_t)
};

const struct TemplateThemeScheme PinkThemeScheme = {
    .background = PinkThemeColorPalette.colors[7],
    .min = PinkThemeColorPalette.colors[1],
    .shadow = PinkThemeColorPalette.colors[7],
    .background_alt = PinkThemeColorPalette.colors[0],
    .button = PinkThemeColorPalette.colors[6],
    .darken = PinkThemeColorPalette.colors[7],
    .boot_splash_foreground = PinkThemeColorPalette.colors[1],
    .text_alt = PinkThemeColorPalette.colors[2],
    .middle = PinkThemeColorPalette.colors[4],
    .max = PinkThemeColorPalette.colors[1],
    .mark = PinkThemeColorPalette.colors[7],
    .highlight = PinkThemeColorPalette.colors[3],
    .text = PinkThemeColorPalette.colors[1],
    .light = PinkThemeColorPalette.colors[3],
    .boot_splash_background = PinkThemeColorPalette.colors[7],
    .dark = PinkThemeColorPalette.colors[6],
    .high = 0xff0000,
    .clock_hands_second = PinkThemeColorPalette.colors[7],
    .medium = PinkThemeColorPalette.colors[5],
    .low = PinkThemeColorPalette.colors[3]
};
