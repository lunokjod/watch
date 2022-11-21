#include <Arduino.h>

// all tiles
#include "../static/Dungeon/floor_1.c"
#include "../static/Dungeon/floor_2.c"
#include "../static/Dungeon/floor_3.c"
#include "../static/Dungeon/floor_4.c"
#include "../static/Dungeon/floor_5.c"
#include "../static/Dungeon/floor_6.c"
#include "../static/Dungeon/floor_7.c"
#include "../static/Dungeon/floor_8.c"
#include "../static/Dungeon/floor_ladder.c"
#include "../static/Dungeon/floor_ladder2.c"
#include "../static/Dungeon/hole.c"

#include "../static/Dungeon/wall_right.c"
#include "../static/Dungeon/wall_mid.c"
#include "../static/Dungeon/wall_left.c"
#include "../static/Dungeon/wall_hole_1.c"
#include "../static/Dungeon/wall_hole_2.c"

#include "../static/Dungeon/floor_spikes_anim_f0.c"
#include "../static/Dungeon/floor_spikes_anim_f1.c"
#include "../static/Dungeon/floor_spikes_anim_f2.c"
#include "../static/Dungeon/floor_spikes_anim_f3.c"

#include "../static/Dungeon/chest_empty_open_anim_f0.c"
#include "../static/Dungeon/chest_empty_open_anim_f1.c"
#include "../static/Dungeon/chest_empty_open_anim_f2.c"
#include "../static/Dungeon/chest_full_open_anim_f0.c"
#include "../static/Dungeon/chest_full_open_anim_f1.c"
#include "../static/Dungeon/chest_full_open_anim_f2.c"
#include "../static/Dungeon/chest_mimic_open_anim_f0.c"
#include "../static/Dungeon/chest_mimic_open_anim_f1.c"
#include "../static/Dungeon/chest_mimic_open_anim_f2.c"

#include "../static/Dungeon/wall_side_front_right.c"
#include "../static/Dungeon/wall_side_front_left.c"

#include "../static/Dungeon/wall_top_mid.c"
#include "../static/Dungeon/wall_inner_corner_l_top_rigth.c"
#include "../static/Dungeon/wall_inner_corner_l_top_left.c"
#include "../static/Dungeon/wall_corner_bottom_right.c"
#include "../static/Dungeon/wall_corner_bottom_left.c"
#include "../static/Dungeon/wall_corner_right.c"
#include "../static/Dungeon/wall_corner_left.c"

#include "../static/Dungeon/wall_side_mid_left.c"
#include "../static/Dungeon/wall_side_mid_right.c"

#include "../static/Dungeon/crate.c"
#include "../static/Dungeon/skull.c"


const unsigned char * DungeonTileSets[] = {
    floor_1.pixel_data,
    floor_2.pixel_data,
    floor_3.pixel_data,
    floor_4.pixel_data,
    floor_5.pixel_data,
    floor_6.pixel_data,
    floor_7.pixel_data,
    floor_8.pixel_data,
    floor_ladder.pixel_data, // 8
    floor_ladder2.pixel_data,
    hole.pixel_data, // 10

    wall_right.pixel_data,
    wall_mid.pixel_data, // 12
    wall_left.pixel_data,
    wall_hole_1.pixel_data, // 14
    wall_hole_2.pixel_data,
    wall_side_front_right.pixel_data,
    wall_side_front_left.pixel_data, // 17
    wall_top_mid.pixel_data,
    wall_inner_corner_l_top_rigth.pixel_data,
    wall_inner_corner_l_top_left.pixel_data,
    wall_corner_bottom_right.pixel_data, // 21
    wall_corner_bottom_left.pixel_data,
    wall_side_mid_left.pixel_data,  // 23
    wall_side_mid_right.pixel_data,
    wall_corner_right.pixel_data, // 25
    wall_corner_left.pixel_data,

    floor_spikes_anim_f0.pixel_data, // 25
    floor_spikes_anim_f1.pixel_data,
    floor_spikes_anim_f2.pixel_data,
    floor_spikes_anim_f3.pixel_data,

    chest_empty_open_anim_f0.pixel_data,
    chest_empty_open_anim_f1.pixel_data, // 30
    chest_empty_open_anim_f2.pixel_data,
    chest_full_open_anim_f0.pixel_data,
    chest_full_open_anim_f1.pixel_data,
    chest_full_open_anim_f2.pixel_data,
    chest_mimic_open_anim_f0.pixel_data,
    chest_mimic_open_anim_f1.pixel_data,
    chest_mimic_open_anim_f2.pixel_data, // 37

    crate.pixel_data,
    skull.pixel_data, // 39
};
