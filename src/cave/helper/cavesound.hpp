/*
 * Copyright (c) 2007-2018, GDash Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:

 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef CAVESOUND_HPP_INCLUDED
#define CAVESOUND_HPP_INCLUDED

#include "config.h"

enum GdSound {
    GD_S_NONE,

    GD_S_STONE,
    GD_S_DIRT_BALL,
    GD_S_NITRO,
    GD_S_FALLING_WALL,
    GD_S_EXPANDING_WALL,
    GD_S_WALL_REAPPEAR,
    GD_S_DIAMOND_RANDOM,    /* randomly select a diamond sound */
    GD_S_DIAMOND_1,
    GD_S_DIAMOND_2,
    GD_S_DIAMOND_3,
    GD_S_DIAMOND_4,
    GD_S_DIAMOND_5,
    GD_S_DIAMOND_6,
    GD_S_DIAMOND_7,
    GD_S_DIAMOND_8,
    GD_S_DIAMOND_COLLECT,
    GD_S_SKELETON_COLLECT,
    GD_S_PNEUMATIC_COLLECT,
    GD_S_BOMB_COLLECT,
    GD_S_CLOCK_COLLECT,
    GD_S_SWEET_COLLECT,
    GD_S_KEY_COLLECT,
    GD_S_DIAMOND_KEY_COLLECT,
    GD_S_SLIME,
    GD_S_LAVA,
    GD_S_REPLICATOR,
    GD_S_ACID_SPREAD,
    GD_S_BLADDER_MOVE,
    GD_S_BLADDER_CONVERT,
    GD_S_BLADDER_SPENDER,
    GD_S_BITER_EAT,
    GD_S_NUT,
    GD_S_NUT_CRACK,

    GD_S_DOOR_OPEN,
    GD_S_WALK_EARTH,
    GD_S_WALK_EMPTY,
    GD_S_STIRRING,
    GD_S_BOX_PUSH,
    GD_S_TELEPORTER,
    GD_S_TIMEOUT_1,
    GD_S_TIMEOUT_2,
    GD_S_TIMEOUT_3,
    GD_S_TIMEOUT_4,
    GD_S_TIMEOUT_5,
    GD_S_TIMEOUT_6,
    GD_S_TIMEOUT_7,
    GD_S_TIMEOUT_8,
    GD_S_TIMEOUT_9,
    GD_S_TIMEOUT,
    GD_S_EXPLOSION,
    GD_S_BOMB_EXPLOSION,
    GD_S_GHOST_EXPLOSION,
    GD_S_VOODOO_EXPLOSION,
    GD_S_NITRO_EXPLOSION,
    GD_S_BOMB_PLACE,
    GD_S_FINISHED,
    GD_S_SWITCH_BITER,
    GD_S_SWITCH_CREATURES,
    GD_S_SWITCH_GRAVITY,
    GD_S_SWITCH_EXPANDING,
    GD_S_SWITCH_CONVEYOR,
    GD_S_SWITCH_REPLICATOR,

    GD_S_AMOEBA,            /* loop */
    GD_S_MAGIC_WALL,        /* loop */
    GD_S_AMOEBA_MAGIC,      /* loop */
    GD_S_COVER,             /* loop */
    GD_S_PNEUMATIC_HAMMER,  /* loop */
    GD_S_WATER,             /* loop */
    GD_S_CRACK,
    GD_S_GRAVITY_CHANGE,
    GD_S_BONUS_LIFE,

    GD_S_MAX,
};

struct SoundWithPos {
    GdSound sound;                          ///< The sound
    int dx, dy;                             ///< Position relative to the active player
    SoundWithPos() = default;
    SoundWithPos(GdSound sound, int dx, int dy): sound(sound), dx(dx), dy(dy) {}
};

const char *gd_sound_get_filename(GdSound sound);
bool gd_sound_is_looped(GdSound sound);
bool gd_sound_is_fake(GdSound sound);
bool gd_sound_is_classic(GdSound sound);
bool gd_sound_force_start(GdSound sound);
int gd_sound_get_channel(GdSound sound);
int gd_sound_get_precedence(GdSound sound);
GdSound gd_sound_classic_equivalent(GdSound sound);


#endif    /* CAVESOUND.H */

