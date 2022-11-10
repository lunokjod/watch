#ifndef __LUNOKIOT__THEME__DEFAULT__
#define __LUNOKIOT__THEME__DEFAULT__
#include <Arduino.h>
/*
 * Default lunokIoT Watch color palette
 */
class LunokIoTThemeColorPalette {
    public:
        // here all your colors, use union to generate an alias
        union {
            uint32_t color0 = 0x212121;
            uint32_t background;
            uint32_t min;
            uint32_t shadow;
        };
        union {
            uint32_t color1 = 0x353e45;
            uint32_t background_alt;
            uint32_t button;
            uint32_t darken;
            uint32_t boot_splash_foreground;
        };
        union {
            uint32_t color2 = 0x555f68;
            uint32_t text_alt;
            uint32_t middle;
        };
        union {
            uint32_t color3 = 0x56818a;
            uint32_t max;
            uint32_t mark;
        };
        union {
            uint32_t color4 = 0xfcb61d;
            uint32_t highlight;
        };
        union {
            uint32_t color5 = 0xffffff;
            uint32_t text;
            uint32_t light;
            uint32_t boot_splash_background;
        };
        union {
            uint32_t color6 = 0x000000;
            uint32_t dark;
        };
        union {
            uint32_t color7 = 0xff0000;
            uint32_t high;
            uint32_t clock_hands_second;
        };
        union {
            uint32_t color8 = 0xffff00;
            uint32_t medium;
        };
        union {
            uint32_t color9 = 0x00ff00;
            uint32_t low;
        };
    
};

#endif
