# Applications list:

## Set Date/Time
 * this app allow set time/date

![menu settime](../../doc/img_menu_settime.png)
![settime time](../../doc/img_settime_0.png)
![settime date](../../doc/img_settime_1.png)

## Timezone
 * this app allow set system timezone

![menu timezone](../../doc/img_menu_timezone.png)
![timezone0](../../doc/img_timezone0.png)

## Provisioning
 * this app allow connect watch to internet via privisioning with a official ESP32 provisionning app from espressif
   * Android: https://play.google.com/store/apps/details?id=com.espressif.provsoftap 
 * Notes: at this time, only WiFi provisioning is availiable

![menu provisioning](../../doc/mainmenu_provisioning.png)
![provisioning main](../../doc/img_provisioning0.png)
![provisioning QR](../../doc/img_provisioning1.png)

## Screen orientation
 * Rotate whole screen

![menu orientaton](../../doc/img_menu_screenrotation.png)
![screen orientaton](../../doc/img_screenrotation0.png)

## Battery
 * Show battery health

![menu battery](../../doc/img_menu_battery.png)
![battery0](../../doc/battery.png)


## Settings
 * Enable disable features

![menu battery](../../doc/img_menu_settings.png)
![battery0](../../doc/img_settings0.png)

## Brightness
 * Set default screen brightness

![menu bright](../../doc/img_mainmenu_brightness.png)
![bright0](../../doc/img_bright0.png)


## Steps
 * Week activity stats 

![menu steps](../../doc/img_mainmenu_steps.png)
![steps0](../../doc/img_steps0.png)
![steps1](../../doc/img_steps1.png)

* @TODO 

# Develop new application:

 * Duplicate files named:
 ```src/app/ApplicationBase.hpp``` & ```src/app/ApplicationBase.cpp``` set name at your discretion (recomended add "Appliction" at end)

 * Edit both files and change all references to "ApplicationBase" and change to "YourDesiredNameApplication"

 * your app is ready!
 
 ## Add to mainmenu (edit app/MainMenu.cpp)

 Include here #include "SomethingApplication.hpp" file on it

On the AllApps definition goes all MainMenu entries:

```{"PUT YOUR APP NAME HERE",img_mainmenu_bright_bits, img_mainmenu_bright_height, img_mainmenu_bright_width, []() { LaunchApplication(new MYAPPLICATION()); } },```

and it's all... icons can be made with gimp (xbm format) img_mainmenu_bright_bits,img_mainmenu_bright_height and img_mainmenu_bright_width are inside xbm file

## How application works:

 * Applications on lWatch are objects descendants from TemplateApplication or LunokIoTApplication

 ***LunokIoTApplication*** is the base application (use it if don't want "back" button on your application)

 ***TemplateApplication*** is a LunokIoTApplication with back button (recommended)
