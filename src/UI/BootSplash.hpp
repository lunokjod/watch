#ifndef __LUNOKIOT__BOOT_SCREEN__
#define __LUNOKIOT__BOOT_SCREEN__

#include <Arduino.h>
#include <LilyGoWatch.h>

#include "../static/img_splash.c"

void DrawSplash();
void LaunchSplash();

void DrawSplashStatus(const char *textBanner);
#endif
