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


#include "../static/Dungeon/wall_side_top_left.c"
#include "../static/Dungeon/wall_side_top_right.c"

#include "../static/Dungeon/wall_banner_green.c"
#include "../static/Dungeon/wall_banner_blue.c"
#include "../static/Dungeon/wall_banner_red.c"
#include "../static/Dungeon/wall_banner_yellow.c"
#include "../static/Dungeon/wall_goo.c"
#include "../static/Dungeon/wall_goo_base.c"

#include "../static/Dungeon/crate.c"
#include "../static/Dungeon/skull.c"
#include "../static/Dungeon/wall_coulmn_base.c"
#include "../static/Dungeon/wall_column_mid.c"
#include "../static/Dungeon/wall_column_top.c"

#include "../static/Dungeon/wall_fountain_basin_blue_anim_f0.c"
#include "../static/Dungeon/wall_fountain_basin_blue_anim_f1.c"
#include "../static/Dungeon/wall_fountain_basin_blue_anim_f2.c"
#include "../static/Dungeon/wall_fountain_mid_blue_anim_f0.c"
#include "../static/Dungeon/wall_fountain_mid_blue_anim_f1.c"
#include "../static/Dungeon/wall_fountain_mid_blue_anim_f2.c"

#include "../static/Dungeon/wall_fountain_basin_red_anim_f0.c"
#include "../static/Dungeon/wall_fountain_basin_red_anim_f1.c"
#include "../static/Dungeon/wall_fountain_basin_red_anim_f2.c"
#include "../static/Dungeon/wall_fountain_mid_red_anim_f0.c"
#include "../static/Dungeon/wall_fountain_mid_red_anim_f1.c"
#include "../static/Dungeon/wall_fountain_mid_red_anim_f2.c"

#include "../static/Dungeon/wall_fountain_top.c"
#include "../static/Dungeon/coulmn_base.c"
#include "../static/Dungeon/column_mid.c"
#include "../static/Dungeon/column_top.c"
#include "../static/Dungeon/wall_corner_top_right.c"
#include "../static/Dungeon/wall_corner_top_left.c"

#include "../static/Dungeon/coin_anim_f3.c"
#include "../static/Dungeon/coin_anim_f2.c"
#include "../static/Dungeon/coin_anim_f1.c"
#include "../static/Dungeon/coin_anim_f0.c"

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
    wall_inner_corner_l_top_rigth.pixel_data, // 19
    wall_inner_corner_l_top_left.pixel_data,
    wall_corner_bottom_right.pixel_data, // 21
    wall_corner_bottom_left.pixel_data,
    wall_side_mid_left.pixel_data,  // 23
    wall_side_mid_right.pixel_data,
    wall_corner_right.pixel_data, // 25
    wall_corner_left.pixel_data,
    wall_side_top_right.pixel_data, // 27
    wall_side_top_left.pixel_data,

    floor_spikes_anim_f0.pixel_data, // 29
    floor_spikes_anim_f1.pixel_data,
    floor_spikes_anim_f2.pixel_data,
    floor_spikes_anim_f3.pixel_data,

    chest_empty_open_anim_f0.pixel_data, // 33
    chest_empty_open_anim_f1.pixel_data,
    chest_empty_open_anim_f2.pixel_data,
    chest_full_open_anim_f0.pixel_data, // 36
    chest_full_open_anim_f1.pixel_data,
    chest_full_open_anim_f2.pixel_data,
    chest_mimic_open_anim_f0.pixel_data, // 39
    chest_mimic_open_anim_f1.pixel_data,
    chest_mimic_open_anim_f2.pixel_data, // 41

    wall_goo.pixel_data,

    wall_banner_green.pixel_data, // 43
    wall_banner_blue.pixel_data,
    wall_banner_red.pixel_data, // 45
    wall_banner_yellow.pixel_data,
    crate.pixel_data,
    skull.pixel_data, // 48
    wall_coulmn_base.pixel_data, 
    wall_column_mid.pixel_data,
    wall_column_top.pixel_data, // 51

    wall_fountain_basin_blue_anim_f0.pixel_data, // 52
    wall_fountain_basin_blue_anim_f1.pixel_data,
    wall_fountain_basin_blue_anim_f2.pixel_data,

    wall_fountain_mid_blue_anim_f0.pixel_data,  // 55
    wall_fountain_mid_blue_anim_f1.pixel_data,
    wall_fountain_mid_blue_anim_f2.pixel_data,

    wall_fountain_basin_red_anim_f0.pixel_data, // 58
    wall_fountain_basin_red_anim_f1.pixel_data,
    wall_fountain_basin_red_anim_f2.pixel_data,

    wall_fountain_mid_red_anim_f0.pixel_data,
    wall_fountain_mid_red_anim_f1.pixel_data,
    wall_fountain_mid_red_anim_f2.pixel_data, // 63

    wall_fountain_top.pixel_data, // 64

    coulmn_base.pixel_data, // 65
    column_mid.pixel_data,
    wall_goo_base.pixel_data, //67
    column_top.pixel_data, 

    wall_corner_top_right.pixel_data, // 69
    wall_corner_top_left.pixel_data,  // 70

    coin_anim_f0.pixel_data, // 71 
    coin_anim_f1.pixel_data,
    coin_anim_f2.pixel_data,
    coin_anim_f3.pixel_data,    // 74

};
const uint32_t DungeonTileSetsSize = sizeof(DungeonTileSets)/sizeof(DungeonTileSets[0]);

#include "../static/Dungeon/elf_m_idle_anim_f0.c"
#include "../static/Dungeon/elf_m_idle_anim_f1.c"
#include "../static/Dungeon/elf_m_idle_anim_f2.c"
#include "../static/Dungeon/elf_m_idle_anim_f3.c"

const unsigned char * DungeonPeopleSets[] = {
    elf_m_idle_anim_f0.pixel_data,
    elf_m_idle_anim_f1.pixel_data,
    elf_m_idle_anim_f2.pixel_data,
    elf_m_idle_anim_f3.pixel_data,
};
const uint32_t DungeonPeopleSetsSize = sizeof(DungeonPeopleSets)/sizeof(DungeonPeopleSets[0]);

