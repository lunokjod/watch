#include <Arduino.h>
#include <cstdio> 
#include "default.hpp"
#include "../UI.hpp"
struct TemplateColorPalette DefaultThemeColorPalette = {
        .color0 = 0x212121,
        .color1 = 0x353e45,
        .color2 = 0x555f68,
        .color3 = 0x56818a,
        .color4 = 0xfcb61d,
        .color5 = 0xffffff,
        .color6 = 0x000000,
        .color7 = 0xff0000,
        .color8 = 0xffff00,
        .color9 = 0x00ff00
};

struct TemplateThemeScheme DefaultThemeScheme = {
        .background = DefaultThemeColorPalette.color0,
        .min = DefaultThemeColorPalette.color0,
        .shadow = DefaultThemeColorPalette.color0,
        .background_alt = DefaultThemeColorPalette.color2,
        .button = DefaultThemeColorPalette.color1,
        .darken = DefaultThemeColorPalette.color1,
        .boot_splash_foreground = DefaultThemeColorPalette.color1,
        .text_alt = DefaultThemeColorPalette.color2,
        .middle = DefaultThemeColorPalette.color2,
        .max = DefaultThemeColorPalette.color3,
        .mark = DefaultThemeColorPalette.color3,
        .highlight = DefaultThemeColorPalette.color4,
        .text = DefaultThemeColorPalette.color5,
        .light = DefaultThemeColorPalette.color5,
        .boot_splash_background = DefaultThemeColorPalette.color5,
        .dark = DefaultThemeColorPalette.color6,
        .high = DefaultThemeColorPalette.color7,
        .clock_hands_second = DefaultThemeColorPalette.color7,
        .medium = DefaultThemeColorPalette.color8,
        .low = DefaultThemeColorPalette.color9
};
