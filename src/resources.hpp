#ifndef __LUNOKIOT__RESOURCE_FILES__
#define __LUNOKIOT__RESOURCE_FILES__

#define lunokiot_logo_32_width 38
#define lunokiot_logo_32_height 32
//const uint8_t * lunokiot_logo_32_bits;
extern const unsigned char lunokiot_logo_32_bits[];

#define img_activity_walking_width 64
#define img_activity_walking_height 64
extern const unsigned char img_activity_walking_bits[];

#define img_activity_running_width 64
#define img_activity_running_height 64
extern const unsigned char img_activity_running_bits[];

#define img_activity_stay_width 64
#define img_activity_stay_height 64
extern const unsigned char img_activity_stay_bits[];

#define img_backscreen_24_width 24
#define img_backscreen_24_height 24
//extern const unsigned char * img_backscreen_24_bits;
extern const unsigned char img_backscreen_24_bits[];

#define img_lunokiot_logo_width 128
#define img_lunokiot_logo_height 109
extern const unsigned char img_lunokiot_logo_bits[1744];

#define img_back_32_width 32
#define img_back_32_height 26
extern const unsigned char img_back_32_bits[];

#define img_trash_32_width 26
#define img_trash_32_height 32
extern const unsigned char img_trash_32_bits[];

#define img_log_32_width 32
#define img_log_32_height 32
extern const unsigned char img_log_32_bits[];

#define img_power_64_width 38
#define img_power_64_height 64
//unsigned char img_power_64_bits[];
extern const unsigned char img_power_64_bits[];

#define img_play_48_width 48
#define img_play_48_height 48
extern const unsigned char img_play_48_bits[];

#define img_pause_48_width 48
#define img_pause_48_height 48
extern const unsigned char img_pause_48_bits[];

#define img_back_16_width 18
#define img_back_16_height 18
extern const unsigned char img_back_16_bits[];

#define img_mainmenu_back_width 120
#define img_mainmenu_back_height 97
extern const unsigned char img_mainmenu_back_bits[];

#define img_mainmenu_debug_width 120
#define img_mainmenu_debug_height 118
extern const unsigned char img_mainmenu_debug_bits[];

#define img_mainmenu_bright_width 120
#define img_mainmenu_bright_height 120
extern const unsigned char img_mainmenu_bright_bits[];

#define img_mainmenu_notifications_width 120
#define img_mainmenu_notifications_height 113
extern const unsigned char img_mainmenu_notifications_bits[];

#define img_mainmenu_steps_width 89
#define img_mainmenu_steps_height 120
extern const unsigned char img_mainmenu_steps_bits[];

#define img_mainmenu_battery_width 66
#define img_mainmenu_battery_height 120
extern const unsigned char img_mainmenu_battery_bits[];

#define img_mainmenu_stopwatch_width 102
#define img_mainmenu_stopwatch_height 120
extern const unsigned char img_mainmenu_stopwatch_bits[];

#define img_mainmenu_options_width 118
#define img_mainmenu_options_height 120
extern const unsigned char img_mainmenu_options_bits[];

#define img_mainmenu_calendar_width 120
#define img_mainmenu_calendar_height 120
extern const unsigned char img_mainmenu_calendar_bits[];

#define img_mainmenu_calculator_width 95
#define img_mainmenu_calculator_height 120
extern const unsigned char img_mainmenu_calculator_bits[];

#define img_mainmenu_about_width 115
#define img_mainmenu_about_height 120
extern const unsigned char img_mainmenu_about_bits[];

#define img_mainmenu_lamp_width 120
#define img_mainmenu_lamp_height 120
extern const unsigned char img_mainmenu_lamp_bits[];

#define img_xbm_32_width 28
#define img_xbm_32_height 32
extern const unsigned char img_xbm_32_bits[];


typedef struct {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  const unsigned char *pixel_data;
} GimpImage;

extern const GimpImage img_battery_empty_160;
extern const GimpImage testImageBackground;
extern const GimpImage img_camera_18;
extern const GimpImage img_landscape_18;
extern const GimpImage img_lightbulb_18;
extern const GimpImage img_wireframe_18;
extern const GimpImage img_alpha_18;
extern const GimpImage img_wirecolor;
extern const GimpImage img_flatcolor_18;
extern const GimpImage img_fullrender_18;
extern const GimpImage img_wirecolor_18;
extern const GimpImage img_landscape_200;
extern const GimpImage img_house_32;
extern const GimpImage img_icon_telegram_48;
extern const GimpImage img_icon_mail_48;
extern const GimpImage img_poweroff_fullscreen;

#include "../src/UI/controls/View3D/Common.hpp"
extern const LuI::Mesh3DDescriptor CubeMesh;
extern const LuI::Mesh3DDescriptor BackButtonMesh;
#endif