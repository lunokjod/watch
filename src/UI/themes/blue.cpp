#include <Arduino.h>
#include <cstdio> 
#include "default.hpp"
#include "../UI.hpp"

struct TemplateColorPalette BlueThemeColorPalette = {
    .color0 = 0xd1dbe4,
    .color1 = 0xbac9d7,
    .color2 = 0xa3b7ca,
    .color3 = 0x8ca5bd,
    .color4 = 0x7593af,
    .color5 = 0x476f95,
    .color6 = 0x305d88,
    .color7 = 0x255481,
    .color8 = 0x1f4f7e,
    .color9 = 0x194a7a
};

struct TemplateThemeScheme BlueThemeScheme = {
    .background = BlueThemeColorPalette.color9,
    .min = BlueThemeColorPalette.color0,
    .shadow = BlueThemeColorPalette.color7,
    .background_alt = BlueThemeColorPalette.color8,
    .button = BlueThemeColorPalette.color6,
    .darken = BlueThemeColorPalette.color8,
    .boot_splash_foreground = BlueThemeColorPalette.color8,
    .text_alt = BlueThemeColorPalette.color2,
    .middle = BlueThemeColorPalette.color4,
    .max = BlueThemeColorPalette.color0,
    .mark = BlueThemeColorPalette.color7,
    .highlight = BlueThemeColorPalette.color3,
    .text = BlueThemeColorPalette.color0,
    .light = BlueThemeColorPalette.color3,
    .boot_splash_background = BlueThemeColorPalette.color0,
    .dark = BlueThemeColorPalette.color6,
    .high = 0xff0000,
    .clock_hands_second = BlueThemeColorPalette.color7,
    .medium = BlueThemeColorPalette.color8,
    .low = BlueThemeColorPalette.color9
};
