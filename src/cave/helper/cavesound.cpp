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

#include "config.h"

#include <glib.h>

#include "cave/helper/cavesound.hpp"

enum GdSoundFlag {
    GD_SP_LOOPED = 1 << 0,  ///< sound must be played looped (for example, amoeba sound)
    GD_SP_CLASSIC = 1 << 1, ///< this is a classic sound. non-classic sounds must have replacement or GD_S_NONE
    GD_SP_FORCE = 1 << 2,   ///< force restart - do not let sample play to the end, but do a restart if the same sample is requested.
    GD_SP_FAKE = 1 << 3,    ///< not a real sound, but a "macro" which will be changed to a real sound during playing.
};

struct SoundProperty {
    GdSound sound;          ///< To help checking the array
    const char *filename;   ///< File name to read the sample from.
    int flags;              ///< Flags, see GdSoundFlag
    int channel;            ///< Channel this sound is played on.
    int precedence;         ///< When multiple sounds are requested in a single frame, the sound with the greater precedence will be played.
    GdSound replace;        ///< If classic sounds are requested, this is the replacement.
};

static constexpr SoundProperty
sound_flags[] = {
    { GD_S_NONE, NULL, GD_SP_CLASSIC, 0, 0},

    // channel 1 sounds.
    // diamond collect sound has precedence over everything.
    // CHANNEL 1 SOUNDS ARE ALWAYS RESTARTED, so no need for GD_SP_FORCE flag.
    // Also, the precedence will only tell the selection of sound to be played in
    // a cave iteration cycle; and AFTER iterating the whole cave, the selected
    // sound will be played regardless of its precedence over the sound from the
    // previous iteration. As if channel 1 sounds were all GD_SP_FORCE-d.
    { GD_S_STONE, "stone.ogg", GD_SP_CLASSIC, 1, 10 },
    { GD_S_DIRT_BALL, "dirt_ball.ogg", 0, 1, 8 },    // sligthly lower precedence, as stones and diamonds should be "louder"
    { GD_S_NITRO, "nitro.ogg", 0, 1, 10 },
    { GD_S_FALLING_WALL, "falling_wall.ogg", 0, 1, 10, GD_S_STONE },
    { GD_S_EXPANDING_WALL, "expanding_wall.ogg", 0, 1, 10, GD_S_STONE },
    { GD_S_WALL_REAPPEAR, "wall_reappear.ogg", 0, 1, 9 },
    { GD_S_DIAMOND_RANDOM, NULL, GD_SP_CLASSIC | GD_SP_FAKE, 1, 10 },
    { GD_S_DIAMOND_1, "diamond_1.ogg", GD_SP_CLASSIC, 1, 10 },
    { GD_S_DIAMOND_2, "diamond_2.ogg", GD_SP_CLASSIC, 1, 10 },
    { GD_S_DIAMOND_3, "diamond_3.ogg", GD_SP_CLASSIC, 1, 10 },
    { GD_S_DIAMOND_4, "diamond_4.ogg", GD_SP_CLASSIC, 1, 10 },
    { GD_S_DIAMOND_5, "diamond_5.ogg", GD_SP_CLASSIC, 1, 10 },
    { GD_S_DIAMOND_6, "diamond_6.ogg", GD_SP_CLASSIC, 1, 10 },
    { GD_S_DIAMOND_7, "diamond_7.ogg", GD_SP_CLASSIC, 1, 10 },
    { GD_S_DIAMOND_8, "diamond_8.ogg", GD_SP_CLASSIC, 1, 10 },
    { GD_S_DIAMOND_COLLECT, "diamond_collect.ogg", GD_SP_CLASSIC, 1, 100 },          // collect sounds have higher precedence than falling sounds and the like.
    { GD_S_SKELETON_COLLECT, "skeleton_collect.ogg", 0, 1, 100, GD_S_DIAMOND_COLLECT },
    { GD_S_PNEUMATIC_COLLECT, "pneumatic_collect.ogg", 0, 1, 50, GD_S_DIAMOND_RANDOM },
    { GD_S_BOMB_COLLECT, "bomb_collect.ogg", 0, 1, 50, GD_S_DIAMOND_RANDOM },
    { GD_S_CLOCK_COLLECT, "clock_collect.ogg", GD_SP_CLASSIC, 1, 50 },
    { GD_S_SWEET_COLLECT, "sweet_collect.ogg", 0, 1, 50, GD_S_NONE },
    { GD_S_KEY_COLLECT, "key_collect.ogg", 0, 1, 50, GD_S_DIAMOND_RANDOM },
    { GD_S_DIAMOND_KEY_COLLECT, "diamond_key_collect.ogg", 0, 1, 50, GD_S_DIAMOND_RANDOM },
    { GD_S_SLIME, "slime.ogg", 0, 1, 5, GD_S_NONE },     // slime has lower precedence than diamond and stone falling sounds.
    { GD_S_LAVA, "lava.ogg", 0, 1, 5, GD_S_NONE },       // lava has low precedence, too.
    { GD_S_REPLICATOR, "replicator.ogg", 0, 1, 5, GD_S_NONE },
    { GD_S_ACID_SPREAD, "acid_spread.ogg", 0,  1, 3, GD_S_NONE },    // same for acid, even lower.
    { GD_S_BLADDER_MOVE, "bladder_move.ogg", 0, 1, 5, GD_S_NONE },   // same for bladder.
    { GD_S_BLADDER_CONVERT, "bladder_convert.ogg", 0, 1, 8, GD_S_NONE },
    { GD_S_BLADDER_SPENDER, "bladder_spender.ogg", 0, 1, 8, GD_S_NONE },
    { GD_S_BITER_EAT, "biter_eat.ogg", 0, 1, 3, GD_S_NONE },     // very low precedence. biters tend to produce too much sound.
    { GD_S_NUT, "nut.ogg", 0, 1, 8, GD_S_NONE },    // nut falling is relatively silent, so low precedence.
    { GD_S_NUT_CRACK, "nut_crack.ogg", 0, 1, 12, GD_S_NONE },   // higher precedence than a stone bouncing.

    // channel2 sounds.
    // Selection rule: will be played if higher precedence than the previously
    // played sound - or maybe if forced.
    { GD_S_DOOR_OPEN, "door_open.ogg", GD_SP_CLASSIC, 2, 10 },
    { GD_S_WALK_EARTH, "walk_earth.ogg", GD_SP_CLASSIC, 2, 10 },
    { GD_S_WALK_EMPTY, "walk_empty.ogg", GD_SP_CLASSIC, 2, 10 },
    { GD_S_STIRRING, "stirring.ogg", GD_SP_CLASSIC, 2, 10 },
    { GD_S_BOX_PUSH, "box_push.ogg", 0, 2, 10, GD_S_STONE },
    { GD_S_TELEPORTER, "teleporter.ogg", 0, 2, 10, GD_S_NONE },
    { GD_S_TIMEOUT_1, "timeout_1.ogg", GD_SP_CLASSIC, 2, 20 },   // timeout sounds have increasing precedence so they are always started
    { GD_S_TIMEOUT_2, "timeout_2.ogg", GD_SP_CLASSIC, 2, 21 },   // timeout sounds are examples which do not need "force restart" flag.
    { GD_S_TIMEOUT_3, "timeout_3.ogg", GD_SP_CLASSIC, 2, 22 },
    { GD_S_TIMEOUT_4, "timeout_4.ogg", GD_SP_CLASSIC, 2, 23 },
    { GD_S_TIMEOUT_5, "timeout_5.ogg", GD_SP_CLASSIC, 2, 24 },
    { GD_S_TIMEOUT_6, "timeout_6.ogg", GD_SP_CLASSIC, 2, 25 },
    { GD_S_TIMEOUT_7, "timeout_7.ogg", GD_SP_CLASSIC, 2, 26 },
    { GD_S_TIMEOUT_8, "timeout_8.ogg", GD_SP_CLASSIC, 2, 27 },
    { GD_S_TIMEOUT_9, "timeout_9.ogg", GD_SP_CLASSIC, 2, 28 },
    { GD_S_TIMEOUT, "timeout.ogg", GD_SP_FORCE, 2, 150, GD_S_NONE },
    { GD_S_EXPLOSION, "explosion.ogg", GD_SP_CLASSIC | GD_SP_FORCE, 2, 100 },
    { GD_S_BOMB_EXPLOSION, "bomb_explosion.ogg", GD_SP_FORCE, 2, 100, GD_S_EXPLOSION },
    { GD_S_GHOST_EXPLOSION, "ghost_explosion.ogg", GD_SP_FORCE, 2, 100, GD_S_EXPLOSION },
    { GD_S_VOODOO_EXPLOSION, "voodoo_explosion.ogg", GD_SP_FORCE, 2, 100, GD_S_EXPLOSION },
    { GD_S_NITRO_EXPLOSION, "nitro_explosion.ogg", GD_SP_FORCE, 2, 100, GD_S_EXPLOSION },
    { GD_S_BOMB_PLACE, "bomb_place.ogg", 0, 2, 10, GD_S_NONE },
    // precedence lower than timeout sounds, because the timeout sounds will be played
    // again very fast when the time counter goes down to zero.
    { GD_S_FINISHED, "finished.ogg", GD_SP_CLASSIC, 2, 15 },
    { GD_S_SWITCH_BITER, "switch_biter.ogg", 0, 2, 10, GD_S_NONE },
    { GD_S_SWITCH_CREATURES, "switch_creatures.ogg", 0, 2, 10, GD_S_NONE },
    { GD_S_SWITCH_GRAVITY, "switch_gravity.ogg", 0, 2, 10, GD_S_NONE },
    { GD_S_SWITCH_EXPANDING, "switch_expanding.ogg", 0, 2, 10, GD_S_NONE },
    { GD_S_SWITCH_CONVEYOR, "switch_conveyor.ogg", 0, 2, 10, GD_S_NONE },
    { GD_S_SWITCH_REPLICATOR, "switch_replicator.ogg", 0, 2, 10, GD_S_NONE },

    // channel 3 sounds.
    { GD_S_AMOEBA, "amoeba.ogg", GD_SP_CLASSIC | GD_SP_LOOPED, 3, 30 },
    { GD_S_MAGIC_WALL, "magic_wall.ogg", GD_SP_CLASSIC | GD_SP_LOOPED, 3, 30 },
    { GD_S_AMOEBA_MAGIC, "amoeba_and_magic.ogg", GD_SP_CLASSIC | GD_SP_LOOPED, 3, 30 },
    { GD_S_COVER, "cover.ogg", GD_SP_CLASSIC | GD_SP_LOOPED, 3, 100 },
    { GD_S_PNEUMATIC_HAMMER, "pneumatic.ogg", GD_SP_CLASSIC | GD_SP_LOOPED, 3, 50 },
    { GD_S_WATER, "water.ogg", GD_SP_LOOPED, 3, 20, GD_S_NONE },
    { GD_S_CRACK, "crack.ogg", GD_SP_CLASSIC, 3, 150 },
    { GD_S_GRAVITY_CHANGE, "gravity_change.ogg", 0, 3, 60, GD_S_NONE },

    // other sounds
    // the bonus life sound has nothing to do with the cave.
    // playing on channel 4.
    { GD_S_BONUS_LIFE, "bonus_life.ogg", 0, 4, 0, GD_S_NONE },
};

/* at program start, do some checks. */
namespace {
    template <int i>
    struct SoundPropertyOk {
        static_assert(sound_flags[i].sound == GdSound(i));
        static_assert((sound_flags[i].flags & GD_SP_CLASSIC) || (sound_flags[sound_flags[i].replace].flags & GD_SP_CLASSIC));
        static_assert(i == GD_S_NONE || (sound_flags[i].channel >= 1 && sound_flags[i].channel <= 4));
        enum { value = SoundPropertyOk<i+1>::value };
    };

    template <>
    struct SoundPropertyOk<GD_S_MAX> {
        enum { value = true };
    };

    static_assert(SoundPropertyOk<1>::value);
}

/*
 *   some sound things
 *
 */

/// Get the filename of a sound, in which the sample should be stored.
/// @param sound The sound identifier.
/// @return The base name of the file, without a path.
const char *gd_sound_get_filename(GdSound sound) {
    return sound_flags[sound].filename;
}

/// Returns true, if the sound is looped.
bool gd_sound_is_looped(GdSound sound) {
    return (sound_flags[sound].flags & GD_SP_LOOPED) != 0;
}

/// Returns true, if the sound is a "macro".
bool gd_sound_is_fake(GdSound sound) {
    return (sound_flags[sound].flags & GD_SP_FAKE) != 0;
}

/// Returns true, if the sound is a classic sound.
bool gd_sound_is_classic(GdSound sound) {
    return (sound_flags[sound].flags & GD_SP_CLASSIC) != 0;
}

/// Returns true, if the sound is always restarted, when playing "again".
/// Forcing also means that the sound is to be started instead of the
/// one currently playing, even if it has a lower precedence.
bool gd_sound_force_start(GdSound sound) {
    return (sound_flags[sound].flags & GD_SP_FORCE) != 0;
}

/// Gives the classic equivalent of the sound.
/// If it is already classic, the function returns with the same value.
/// If not, it is changed to a classic replacement.
GdSound gd_sound_classic_equivalent(GdSound sound) {
    if (gd_sound_is_classic(sound))
        return sound;
    return sound_flags[sound].replace;
}

/// Get the channel this sound is to be played on.
int gd_sound_get_channel(GdSound sound) {
    return sound_flags[sound].channel;
}

/// Get the precedence of the sound.
/// Sounds with higher precedence should be played, if more sounds are requested at once.
int gd_sound_get_precedence(GdSound sound) {
    return sound_flags[sound].precedence;
}
