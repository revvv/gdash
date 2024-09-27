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

#include <cmath>

#include "cave/caverendered.hpp"
#include "cave/elementproperties.hpp"
#include "settings.hpp"


// for gravity and other routines.
// these arrays contain the rotated directions.
// ccw eighth: counter-clockwise, 1/8 turn (45 degrees)
// cw fourth: clockwise, 1/4 turn (90 degrees)
static GdDirection const ccw_eighth[] = { MV_STILL, MV_UP_LEFT, MV_UP, MV_UP_RIGHT, MV_RIGHT, MV_DOWN_RIGHT, MV_DOWN, MV_DOWN_LEFT };
static GdDirection const ccw_fourth[] = { MV_STILL, MV_LEFT, MV_UP_LEFT, MV_UP, MV_UP_RIGHT, MV_RIGHT, MV_DOWN_RIGHT, MV_DOWN, MV_DOWN_LEFT, MV_LEFT };
static GdDirection const cw_eighth[] = { MV_STILL, MV_UP_RIGHT, MV_RIGHT, MV_DOWN_RIGHT, MV_DOWN, MV_DOWN_LEFT, MV_LEFT, MV_UP_LEFT, MV_UP };
static GdDirection const cw_fourth[] = { MV_STILL, MV_RIGHT, MV_DOWN_RIGHT, MV_DOWN, MV_DOWN_LEFT, MV_LEFT, MV_UP_LEFT, MV_UP, MV_UP_RIGHT };
// 180 degrees turn of a direction
static GdDirection const opposite[] = { MV_STILL, MV_DOWN, MV_DOWN_LEFT, MV_LEFT, MV_UP_LEFT, MV_UP, MV_UP_RIGHT, MV_RIGHT, MV_DOWN_RIGHT };
// doubling a direction (e.g. right=1,0   2x right=2,0
static GdDirection const twice[] = { MV_STILL, MV_UP_2, MV_UP_RIGHT_2, MV_RIGHT_2, MV_DOWN_RIGHT_2, MV_DOWN_2, MV_DOWN_LEFT_2, MV_LEFT_2, MV_UP_LEFT_2 };


void CaveRendered::add_particle_set(int x, int y, GdElementEnum particletype) {
    if (!gd_particle_effects)
        return;

    /* movements and sizes can depend on gravity. */
    /* when stones & diamonds are falling, particle effects are perpendicular
     * to the direction of gravity (eg. falling vertically, and some extends
     * horizonally), so gx and gy are deliberately swapped in some expressions below. */
    double gx = gd_dx[gravity], gy = gd_dy[gravity], agx = fabs(gx), agy = fabs(gy);
    switch (particletype) {
        case O_DIRT:
            particles.push_back(ParticleSet(75, 0.1, 0.15, x + 0.5, y + 0.5, 0.5, 0.5, 0, 0, 1, 1, dirt_particle_color));
            break;
        case O_DIRT2:
            particles.push_back(ParticleSet(75, 0.1, 0.15, x + 0.5, y + 0.5, 0.5, 0.5, 0, 0, 1, 1, dirt_2_particle_color));
            break;
        case O_STONE_F:
            particles.push_back(ParticleSet(75, 0.1, 0.15,
                                            x + 0.5 + 0.5 * gx, y + 0.5 + 0.5 * gy, 0.25 + 0.25 * agy, 0.25 + 0.25 * agx,
                                            0.5 * gx, 0.5 * gy, 1 + agy, 1 + agx, stone_particle_color));
            break;
        case O_MEGA_STONE_F:
            particles.push_back(ParticleSet(75, 0.1, 0.15,
                                            x + 0.5 + 0.5 * gx, y + 0.5 + 0.5 * gy, 0.25 + 0.25 * agy, 0.25 + 0.25 * agx,
                                            0.5 * gx, 0.5 * gy, 1 + agy, 1 + agx, mega_stone_particle_color));
            break;
        case O_DIAMOND_F:
            /* falling diamond */
            particles.push_back(ParticleSet(15, 0.03, 0.5,
                                            x + 0.5 + 0.5 * gx, y + 0.5 + 0.5 * gy, 0.25, 0.25,
                                            0, 0, 2, 2, diamond_particle_color));
            break;
        case O_DIAMOND:
            /* collecting diamond */
            particles.push_back(ParticleSet(8, 0.03, 0.5,
                                            x + 0.5, y + 0.5, 0.25, 0.25,
                                            0, 0, 2, 2, diamond_particle_color));
            break;
        case O_EXPLODE_1:
            /* for explosions, the original place of the particles is a 2x2 cave cell area, but they
             * expand rapidly. */
            particles.push_back(ParticleSet(300, 0.05, 0.5, x + 0.5, y + 0.5, 1.0, 1.0, 0, 0, 4, 4, explosion_particle_color));
            break;
        case O_PRE_DIA_1:
            particles.push_back(ParticleSet(300, 0.05, 0.5, x + 0.5, y + 0.5, 1.0, 1.0, 0, 0, 4, 4, diamond_particle_color));
            break;
        case O_MAGIC_WALL:
            // a magic wall creates particles in every frame. so add only very few particles!
            // rather they should be bright like stars
            particles.push_back(ParticleSet(3, 0.01, 0.75, x + 0.5, y + 0.5, 0.5, 0.5, 0, 0, 1 + 2 * agx, 1 + 2 * agy, magic_wall_particle_color));
            break;
        case O_EXPANDING_WALL:
            particles.push_back(ParticleSet(75, 0.1, 0.15, x + 0.5, y + 0.5, 0.5, 0.5, 0, 0, 1, 1, expanding_wall_particle_color));
            break;
        case O_EXPANDING_STEEL_WALL:
            particles.push_back(ParticleSet(75, 0.1, 0.15, x + 0.5, y + 0.5, 0.5, 0.5, 0, 0, 1, 1, expanding_steel_wall_particle_color));
            break;
        case O_LAVA:
            // this should look like it's boiling
            particles.push_back(ParticleSet(10, 0.01, 0.5, x + 0.5, y + 0.5, 0.5, 0.5, 0, 0, 2, 2, lava_particle_color));
            break;
        case O_ROCKET_1:
            particles.push_back(ParticleSet(100, 0.03, 0.25, x + 0.9, y + 0.5, 0.5, 0.2, -4, 0.2, 3, 0.2, explosion_particle_color));
            break;
        case O_ROCKET_2:
            particles.push_back(ParticleSet(100, 0.03, 0.25, x + 0.5, y + 0.1, 0.2, 0.5, 0.2, 4, 0.2, 3, explosion_particle_color));
            break;
        case O_ROCKET_3:
            particles.push_back(ParticleSet(100, 0.03, 0.25, x + 0.1, y + 0.5, 0.5, 0.2, 4, 0.2, 3, 0.2, explosion_particle_color));
            break;
        case O_ROCKET_4:
            particles.push_back(ParticleSet(100, 0.03, 0.25, x + 0.5, y + 0.9, 0.2, 0.5, 0.2, -4, 0.2, 3, explosion_particle_color));
            break;
        default:
            break;
    }
}


/// Remembers to play the sound in the given cave.
/// The sound1, sound2 and sound3 variables will be handled by a game implementation.
/// This function only remembers to play them. It also checks the precedences.
void CaveRendered::sound_play(GdSound sound, int x, int y) {
    switch (sound) {
        case GD_S_NONE:
            return;
            break;
        case GD_S_WATER:
            if (!water_sound) return;
            break;
        case GD_S_AMOEBA:
            if (!amoeba_sound) return;
            break;
        case GD_S_MAGIC_WALL:
            if (!magic_wall_sound) return;
            break;
        case GD_S_STONE:
            if (!stone_sound) return;
            break;
        case GD_S_DIAMOND_RANDOM:
            if (!diamond_sound) return;
            break;
        case GD_S_NUT:
            if (!nut_sound) return;
            break;
        case GD_S_NITRO:
            if (!nitro_sound) return;
            break;
        case GD_S_FALLING_WALL:
            if (!falling_wall_sound) return;
            break;
        case GD_S_EXPANDING_WALL:
            if (!expanding_wall_sound) return;
            break;
        case GD_S_BLADDER_SPENDER:
            if (!bladder_spender_sound) return;
            break;
        case GD_S_BLADDER_CONVERT:
            if (!bladder_convert_sound) return;
            break;
        case GD_S_SLIME:
            if (!slime_sound) return;
            break;
        case GD_S_LAVA:
            if (!lava_sound) return;
            break;
        case GD_S_ACID_SPREAD:
            if (!acid_spread_sound) return;
            break;
        case GD_S_BLADDER_MOVE:
            if (!bladder_sound) return;
            break;
        case GD_S_BITER_EAT:
            if (!biter_sound) return;
            break;
        case GD_S_NUT_CRACK:
            if (!nut_sound) return;
            break;

        default:
            break;
    }

    SoundWithPos *s;
    switch (gd_sound_get_channel(sound)) {
        case 1:
            s = &sound1;
            break;
        case 2:
            s = &sound2;
            break;
        case 3:
            s = &sound3;
            break;
        default:
            g_assert_not_reached();
            return;
    }

    /* amoeba and magic wall _together_ make a different, mixed, "gritty" sound. */
    /* so if we find a mixing situation (old=amoeba & new=magic or vice versa), do it. */
    /* also, if we were already mixing, "continue" the mixing, but still check the
     * distances of sounds below. */
    if ((s->sound == GD_S_AMOEBA && sound == GD_S_MAGIC_WALL)
            || (s->sound == GD_S_MAGIC_WALL && sound == GD_S_AMOEBA)
            || (s->sound == GD_S_AMOEBA_MAGIC))
        sound = GD_S_AMOEBA_MAGIC;

    /* sound coordinates relative to player */
    int dx = x - player_x, dy = y - player_y;
    /* if higher precedence, or same precedence but closer than the previous one, play. */
    if (gd_sound_get_precedence(sound) >= gd_sound_get_precedence(s->sound)
            || (gd_sound_get_precedence(sound) == gd_sound_get_precedence(s->sound)
                && (dx * dx + dy * dy) < (s->dx * s->dx + s->dy * s->dy))) {
        *s = SoundWithPos(sound, dx, dy);
    }
}


/// play sound of a given element.
void CaveRendered::play_effect_of_element(GdElementEnum element, int x, int y, GdDirectionEnum dir, bool particles) {
    x += gd_dx[dir];
    y += gd_dy[dir];
    /* stone and diamond fall sounds. */
    switch (element) {
        case O_WATER:
            sound_play(GD_S_WATER, x, y);
            break;
        case O_AMOEBA:
            sound_play(GD_S_AMOEBA, x, y);
            break;
        case O_MAGIC_WALL:
            sound_play(GD_S_MAGIC_WALL, x, y);
            if (particles)
                add_particle_set(x, y, O_MAGIC_WALL);
            break;
        case O_STONE:
        case O_STONE_F:
        case O_FLYING_STONE:
        case O_FLYING_STONE_F:
        case O_WAITING_STONE:
        case O_CHASING_STONE:
            sound_play(GD_S_STONE, x, y);
            if (particles)
                add_particle_set(x, y, O_STONE_F);
            break;

        case O_MEGA_STONE:
        case O_MEGA_STONE_F:
            sound_play(GD_S_STONE, x, y);
            if (particles)
                add_particle_set(x, y, O_MEGA_STONE_F);
            break;

        case O_DIAMOND:
        case O_DIAMOND_F:
        case O_FLYING_DIAMOND:
        case O_FLYING_DIAMOND_F:
            sound_play(GD_S_DIAMOND_RANDOM, x, y);
            if (particles)
                add_particle_set(x, y, O_DIAMOND_F);
            break;

        case O_NUT:
        case O_NUT_F:
            sound_play(GD_S_NUT, x, y);
            break;

        case O_NITRO_PACK:
        case O_NITRO_PACK_F:
            sound_play(GD_S_NITRO, x, y);
            break;

        case O_FALLING_WALL:
        case O_FALLING_WALL_F:
            sound_play(GD_S_FALLING_WALL, x, y);
            break;

        case O_H_EXPANDING_WALL:
        case O_V_EXPANDING_WALL:
        case O_EXPANDING_WALL:
            sound_play(GD_S_EXPANDING_WALL, x, y);
            if (particles)
                add_particle_set(x, y, O_EXPANDING_WALL);
            break;

        case O_H_EXPANDING_STEEL_WALL:
        case O_V_EXPANDING_STEEL_WALL:
        case O_EXPANDING_STEEL_WALL:
            sound_play(GD_S_EXPANDING_WALL, x, y);
            if (particles)
                add_particle_set(x, y, O_EXPANDING_STEEL_WALL);
            break;

        case O_BLADDER_SPENDER:
            sound_play(GD_S_BLADDER_SPENDER, x, y);
            break;

        case O_PRE_CLOCK_1:
            sound_play(GD_S_BLADDER_CONVERT, x, y);
            break;

        case O_SLIME:
            sound_play(GD_S_SLIME, x, y);
            break;

        case O_LAVA:
            sound_play(GD_S_LAVA, x, y);
            break;

        case O_ACID:
            sound_play(GD_S_ACID_SPREAD, x, y);
            break;

        case O_BLADDER:
            sound_play(GD_S_BLADDER_MOVE, x, y);
            break;

        case O_BITER_1:
        case O_BITER_2:
        case O_BITER_3:
        case O_BITER_4:
            sound_play(GD_S_BITER_EAT, x, y);
            break;

        case O_DIRT:
            sound_play(GD_S_WALK_EARTH, x, y);
            if (particles)
                add_particle_set(x, y, O_DIRT);
            break;

        case O_DIRT2:
            sound_play(GD_S_WALK_EARTH, x, y);
            if (particles)
                add_particle_set(x, y, O_DIRT2);
            break;

        case O_DIRT_BALL:
        case O_DIRT_BALL_F:
        case O_DIRT_LOOSE:
        case O_DIRT_LOOSE_F:
            sound_play(GD_S_DIRT_BALL, x, y);
            if (particles)
                add_particle_set(x, y, O_DIRT2);
            break;

        default:
            /* do nothing. */
            break;
    }
}


void CaveRendered::play_eat_sound_of_element(GdElementEnum element, int x, int y) {
    switch (element) {
        case O_DIAMOND_KEY:
            sound_play(GD_S_DIAMOND_KEY_COLLECT, x, y);
            break;
        case O_KEY_1:
            sound_play(GD_S_KEY_COLLECT, x, y);
            break;
        case O_KEY_2:
            sound_play(GD_S_KEY_COLLECT, x, y);
            break;
        case O_KEY_3:
            sound_play(GD_S_KEY_COLLECT, x, y);
            break;
        case O_DOOR_1:
            sound_play(GD_S_DOOR_OPEN, x, y);
            break;
        case O_DOOR_2:
            sound_play(GD_S_DOOR_OPEN, x, y);
            break;
        case O_DOOR_3:
            sound_play(GD_S_DOOR_OPEN, x, y);
            break;
        case O_CREATURE_SWITCH:
            sound_play(GD_S_SWITCH_CREATURES, x, y);
            break;
        case O_EXPANDING_WALL_SWITCH:
            sound_play(GD_S_SWITCH_EXPANDING, x, y);
            break;
        case O_BITER_SWITCH:        /* biter change delay */
            sound_play(GD_S_SWITCH_BITER, x, y);
            break;
        case O_REPLICATOR_SWITCH:   /* replicator on/off switch */
            sound_play(GD_S_SWITCH_REPLICATOR, x, y);
            break;
        case O_CONVEYOR_SWITCH: /* conveyor belts on/off */
            sound_play(GD_S_SWITCH_CONVEYOR, x, y);
            break;
        case O_CONVEYOR_DIR_SWITCH: /* conveyor belts switch direction */
            sound_play(GD_S_SWITCH_CONVEYOR, x, y);
            break;
        case O_DIRT:
        case O_DIRT_SLOPED_UP_RIGHT:
        case O_DIRT_SLOPED_UP_LEFT:
        case O_DIRT_SLOPED_DOWN_LEFT:
        case O_DIRT_SLOPED_DOWN_RIGHT:
            sound_play(GD_S_WALK_EARTH, x, y);
            break;
        case O_DIRT2:
        case O_DIRT_BALL:
        case O_DIRT_LOOSE:
            sound_play(GD_S_WALK_EARTH, x, y);
            break;
        case O_STEEL_EATABLE:
        case O_BRICK_EATABLE:
            sound_play(GD_S_WALK_EARTH, x, y);
            break;
        case O_SPACE:
        case O_LAVA:
            sound_play(GD_S_WALK_EMPTY, x, y);
            break;
        case O_SWEET:
            sound_play(GD_S_SWEET_COLLECT, x, y);
            break;
        case O_PNEUMATIC_HAMMER:
            sound_play(GD_S_PNEUMATIC_COLLECT, x, y);
            break;
        case O_CLOCK:
            sound_play(GD_S_CLOCK_COLLECT, x, y);
            break;
        case O_DIAMOND:
        case O_FLYING_DIAMOND:
            sound_play(GD_S_DIAMOND_COLLECT, x, y);
            break;
        case O_SKELETON:
            sound_play(GD_S_SKELETON_COLLECT, x, y);
            break;
        default:
            /* no effect */
            break;
    }
}


void CaveRendered::play_eat_particle_of_element(GdElementEnum element, int x, int y) {
    switch (element) {
        case O_DIRT:
        case O_DIRT_SLOPED_UP_RIGHT:
        case O_DIRT_SLOPED_UP_LEFT:
        case O_DIRT_SLOPED_DOWN_LEFT:
        case O_DIRT_SLOPED_DOWN_RIGHT:
            add_particle_set(x, y, O_DIRT);
            break;
        case O_DIRT2:
        case O_DIRT_BALL:
        case O_DIRT_LOOSE:
            add_particle_set(x, y, O_DIRT2);
            break;
        case O_DIAMOND:
        case O_FLYING_DIAMOND:
            add_particle_set(x, y, O_DIAMOND);
            break;
        case O_STEEL_EATABLE:
        case O_BRICK_EATABLE:
            break;
        default:
            /* no effect */
            break;
    }
}


/// sets timeout sound.
void CaveRendered::set_seconds_sound() {
    /* this is an integer division, so 0 seconds can be 0.5 seconds... */
    /* also, when this reaches 8, the player still has 8.9999 seconds. so the sound is played at almost t=9s. */
    switch (time / timing_factor) {
        case 8:
            sound_play(GD_S_TIMEOUT_1, player_x, player_y);
            break;
        case 7:
            sound_play(GD_S_TIMEOUT_2, player_x, player_y);
            break;
        case 6:
            sound_play(GD_S_TIMEOUT_3, player_x, player_y);
            break;
        case 5:
            sound_play(GD_S_TIMEOUT_4, player_x, player_y);
            break;
        case 4:
            sound_play(GD_S_TIMEOUT_5, player_x, player_y);
            break;
        case 3:
            sound_play(GD_S_TIMEOUT_6, player_x, player_y);
            break;
        case 2:
            sound_play(GD_S_TIMEOUT_7, player_x, player_y);
            break;
        case 1:
            sound_play(GD_S_TIMEOUT_8, player_x, player_y);
            break;
        case 0:
            sound_play(GD_S_TIMEOUT_9, player_x, player_y);
            break;
    }
}


/// Returns the element at x,y.
inline GdElementEnum CaveRendered::get(int x, int y) const {
    return map(x, y);
}

/// Returns the element at (x,y)+dir.
inline GdElementEnum CaveRendered::get(int x, int y, GdDirectionEnum dir) const {
    return get(x + gd_dx[dir], y + gd_dy[dir]);
}

/// Returns true, if element at (x,y)+dir explodes if hit by a stone (for example, a firefly).
inline bool CaveRendered::explodes_by_hit(int x, int y, GdDirectionEnum dir) const {
    return (gd_element_properties[get(x, y, dir)].flags & P_EXPLODES_BY_HIT) != 0;
}

/// returns true, if the element is not explodable (for example the steel wall).
inline bool CaveRendered::non_explodable(int x, int y) const {
    return (gd_element_properties[get(x, y)].flags & P_NON_EXPLODABLE) != 0;
}

/// returns true, if the element at (x,y)+dir can be eaten by the amoeba (dirt, space)
inline bool CaveRendered::amoeba_eats(int x, int y, GdDirectionEnum dir) const {
    return (gd_element_properties[get(x, y, dir)].flags & P_AMOEBA_CONSUMES) != 0;
}

/// Returns true if the element is sloped, so stones and diamonds roll down on it.
/// For example a stone or brick wall.
/// Some elements can be sloped in specific directions only; for example a wall
/// like /|  is sloped from the up to the left.
/// @param x The x coordinate
/// @param y The y coordinate
/// @param dir The coordinate to move from (x,y), e.g. element at (x,y)+dir is checked.
/// @param slop The direction in which the element should be sloped.
bool CaveRendered::sloped(int x, int y, GdDirectionEnum dir, GdDirectionEnum slop) const {
    switch (slop) {
        case MV_LEFT:
            return (gd_element_properties[get(x, y, dir)].flags & P_SLOPED_LEFT) != 0;
        case MV_RIGHT:
            return (gd_element_properties[get(x, y, dir)].flags & P_SLOPED_RIGHT) != 0;
        case MV_UP:
            return (gd_element_properties[get(x, y, dir)].flags & P_SLOPED_UP) != 0;
        case MV_DOWN:
            return (gd_element_properties[get(x, y, dir)].flags & P_SLOPED_DOWN) != 0;
        default:
            break;
    }

    return false;
}

/// returns true if the element is sloped for bladder movement (brick=yes, diamond=no, for example)
inline bool CaveRendered::sloped_for_bladder(int x, int y, GdDirectionEnum dir) const {
    return (gd_element_properties[get(x, y, dir)].flags & P_BLADDER_SLOPED) != 0;
}

/// returns true if the element at (x,y)+dir can blow up a fly by touching it.
inline bool CaveRendered::blows_up_flies(int x, int y, GdDirectionEnum dir) const {
    return (gd_element_properties[get(x, y, dir)].flags & P_BLOWS_UP_FLIES) != 0;
}

/// returns true if the element is a counter-clockwise creature
inline bool CaveRendered::rotates_ccw(int x, int y) const {
    return (gd_element_properties[get(x, y)].flags & P_CCW) != 0;
}

/// returns true if the element is a player (normal player, player glued, player with bomb)
bool CaveRendered::is_player(int x, int y) const {
    return (gd_element_properties[get(x, y)].flags & P_PLAYER) != 0;
}

/// returns true if the element at (x,y)+dir is a player (normal player, player glued, player with bomb)
bool CaveRendered::is_player(int x, int y, GdDirectionEnum dir) const {
    return (gd_element_properties[get(x, y, dir)].flags & P_PLAYER) != 0;
}

/// returns true if the element at (x,y)+dir can be hammered.
inline bool CaveRendered::can_be_hammered(int x, int y, GdDirectionEnum dir) const {
    return (gd_element_properties[get(x, y, dir)].flags & P_CAN_BE_HAMMERED) != 0;
}

/// Returns true if the element at (x,y)+dir can be pushed.
/// @todo should be inlined.
bool CaveRendered::can_be_pushed(int x, int y, GdDirectionEnum dir) const {
    return (gd_element_properties[get(x, y, dir)].flags & P_CAN_BE_PUSHED) != 0;
}

/// returns true if the element at (x,y) is the first animation stage of an explosion
inline bool CaveRendered::is_first_stage_of_explosion(int x, int y) const {
    return (gd_element_properties[get(x, y)].flags & P_EXPLOSION_FIRST_STAGE) != 0;
}

/// returns true if the element sits on and is moved by the conveyor belt
inline bool CaveRendered::moved_by_conveyor_top(int x, int y, GdDirectionEnum dir) const {
    return (gd_element_properties[get(x, y, dir)].flags & P_MOVED_BY_CONVEYOR_TOP) != 0;
}

/// returns true if the elements floats upwards, and is conveyed by the conveyor belt which is OVER it
inline bool CaveRendered::moved_by_conveyor_bottom(int x, int y, GdDirectionEnum dir) const {
    return (gd_element_properties[get(x, y, dir)].flags & P_MOVED_BY_CONVEYOR_BOTTOM) != 0;
}

/// returns true if the element is a scanned one (needed by the engine)
inline bool CaveRendered::is_scanned(int x, int y) const {
    return is_scanned_element(get(x, y));
}

/// returns true if the element is a scanned one (needed by the engine)
inline bool CaveRendered::is_scanned(int x, int y, GdDirectionEnum dir) const {
    return is_scanned_element(get(x + gd_dx[dir], y + gd_dy[dir]));
}

/// Returns true if neighboring element is "e", or equivalent to "e".
/// Dirt is treated in a special way; eg. if is_like_element(O_DIRT) is
/// asked, an an O_DIRT2 is there, true is returned.
/// Also, lava is special; if is_like_element(O_SPACE) is asked, and
/// an O_LAVA is there, true is returned. This way any movement
/// is allowed by any creature and player into lava.
/// @param x The x coordinate on the map
/// @param y The y coordinate on the map
/// @param dir The direction to add to (x,y) and check the element at
/// @param e The element to compare (x,y)+dir to
/// @return True, if they are equivalent
bool CaveRendered::is_like_element(int x, int y, GdDirectionEnum dir, GdElementEnum e) const {
    int examined = get(x, y, dir);

    /* if it is a dirt-like, change to dirt, so equality will evaluate to true */
    if (gd_element_properties[examined].flags & P_DIRT)
        examined = O_DIRT;
    if (gd_element_properties[e].flags & P_DIRT)
        e = O_DIRT;
    /* if the element on the map is a lava, it should be like space */
    if (examined == O_LAVA)
        examined = O_SPACE;
    /* now they are "normalized", compare and return */
    return e == examined;
}

/// Returns true if the neighboring element (x,y)+dir is a dirt or lava.
/// This is a shorthand function to check easily if there is space somewhere.
/// Any movement which is possible into space, must also be possible into lava.
/// Therefore 'if (map(x,y)==O_SPACE)' must not be used!
/// Lava absorbs everything going into it. Everything.
/// But it does not "pull" elements; only the things disappear which
/// _do_ go directly into it. So if the player steps into the lava,
/// he will die. If a dragonfly flies over it, it will not.
/// This behavior is implemented in the is_like_space and the store
/// functions. is_like_space returns true for the lava, too. The store
/// function ignores any store requests into the lava.
/// The player_eat_element function will also behave for lava as it does for space.
inline bool CaveRendered::is_like_space(int x, int y, GdDirectionEnum dir) const {
    int e = get(x, y, dir);
    return e == O_SPACE || e == O_LAVA;
}

/// Returns true, if element at (x,y)+dir is like dirt.
/// All dirt types must be equivalent; for example when allowing the player to
/// place a bomb in dirt, or when a nitro pack is bouncing on a piece of dirt
/// (without exploding).
/// Therefore 'if (map(x,y)==O_DIRT)' must not be used!
inline bool CaveRendered::is_like_dirt(int x, int y, GdDirectionEnum dir) const {
    return (gd_element_properties[get(x, y, dir)].flags & P_DIRT) != 0;
}


/// Store an element at a given position; lava absorbs everything.
/// If there is a lava originally at the given position, sound is played, and
/// the map is NOT changed.
/// The element given is changed to its "scanned" state, if there is such.
inline void CaveRendered::store(int x, int y, GdElementEnum element, bool disable_particle) {
    GdElementEnum &cell = map(x, y);
    if (cell == O_LAVA) {
        play_effect_of_element(O_LAVA, x, y);
        return;
    }
    cell = scanned_pair(element);
}


/// Store an element to (x,y)+dir.
inline void CaveRendered::store(int x, int y, GdDirectionEnum dir, GdElementEnum element) {
    store(x + gd_dx[dir], y + gd_dy[dir], element);
}

/// Store the element to (x,y)+dir, and store a space to (x,y).
inline void CaveRendered::move(int x, int y, GdDirectionEnum dir, GdElementEnum element) {
    store(x, y, dir, element);
    store(x, y, O_SPACE);
}

/// increment a cave element; can be used for elements which are one after the other, for example bladder1, bladder2, bladder3...
/// @todo to be removed
inline void CaveRendered::next(int x, int y) {
    map(x, y) = GdElementEnum(map(x, y) + 1);
}

/// Remove th scanned "bit" from an element.
/// To be called only for scanned elements!!!
inline void CaveRendered::unscan(int x, int y) {
    if (is_scanned(x, y))
        map(x, y) = gd_element_properties[get(x, y)].pair;
}


/// Change the cell at (x,y) to a given explosion type.
/// Used by 3x3 explosion functions.
/// Take care of non explodable elements.
/// Take care of other special cases, like a voodoo dying,
/// and a nitro pack explosion triggered.
void CaveRendered::cell_explode(int x, int y, GdElementEnum explode_to) {
    if (non_explodable(x, y))
        return;

    if (voodoo_any_hurt_kills_player && get(x, y) == O_VOODOO)
        voodoo_touched = true;

    if (get(x, y) == O_VOODOO && !voodoo_disappear_in_explosion)
        /* voodoo turns into a time penalty */
        store(x, y, O_TIME_PENALTY);
    else if (get(x, y) == O_NITRO_PACK || get(x, y) == O_NITRO_PACK_F)
        /* nitro pack inside an explosion - it is now triggered */
        store(x, y, O_NITRO_PACK_EXPLODE);
    else
        /* for everything else. disable particle effects, as the explosion already generates */
        store(x, y, explode_to, true /* disable particle effects */);
}

/// A creature at (x,y) explodes to a 3x3 square.
void CaveRendered::creature_explode(int x, int y, GdElementEnum explode_to) {
    /* the processing of an explosion took pretty much time: processing 3x3=9 elements */
    ckdelay_current += 1200;
    sound_play(GD_S_EXPLOSION, x, y);

    for (int yy = y - 1; yy <= y + 1; yy++)
        for (int xx = x - 1; xx <= x + 1; xx++)
            cell_explode(xx, yy, explode_to);
}

/// A nitro pack at (x,y) explodes to a 3x3 square.
void CaveRendered::nitro_explode(int x, int y) {
    /* the processing of an explosion took pretty much time: processing 3x3=9 elements */
    ckdelay_current += 1200;
    sound_play(GD_S_NITRO_EXPLOSION, x, y);

    for (int yy = y - 1; yy <= y + 1; yy++)
        for (int xx = x - 1; xx <= x + 1; xx++)
            cell_explode(xx, yy, O_NITRO_EXPL_1);
    /* the current cell is explicitly changed into a nitro expl, as cell_explode changes it to a triggered nitro pack */
    store(x, y, O_NITRO_EXPL_1);
}

/// A voodoo explodes, leaving a 3x3 steel and a time penalty behind.
void CaveRendered::voodoo_explode(int x, int y) {
    if (voodoo_any_hurt_kills_player)
        voodoo_touched = true;

    /* the processing of an explosion took pretty much time: processing 3x3=9 elements */
    ckdelay_current += 1000;
    sound_play(GD_S_VOODOO_EXPLOSION, x, y);

    /* voodoo explodes to 3x3 steel */
    for (int yy = y - 1; yy <= y + 1; yy++)
        for (int xx = x - 1; xx <= x + 1; xx++)
            store(xx, yy, O_PRE_STEEL_1);
    /* middle is a time penalty (which will be turned into a gravestone) */
    store(x, y, O_TIME_PENALTY);
}

/// Explode cell at (x,y), but skip voodooo.
/// A bomb does not explode the voodoo, neither does the ghost.
/// This function checks these, and stores the new element given or not.
/// Destroying the voodoo is also controlled by the voodoo_disappear_in_explosion flag.
void CaveRendered::cell_explode_skip_voodoo(int x, int y, GdElementEnum expl) {
    if (non_explodable(x, y))
        return;
    /* bomb does not explode voodoo */
    if (!voodoo_disappear_in_explosion && get(x, y) == O_VOODOO)
        return;
    if (voodoo_any_hurt_kills_player && get(x, y) == O_VOODOO)
        voodoo_touched = true;
    store(x, y, expl);
}

/// An x shaped ghost explosion.
void CaveRendered::ghost_explode(int x, int y) {
    /* the processing of an explosion took pretty much time: processing 5 elements */
    ckdelay_current += 650;
    sound_play(GD_S_GHOST_EXPLOSION, x, y);

    cell_explode_skip_voodoo(x, y, O_GHOST_EXPL_1);
    cell_explode_skip_voodoo(x - 1, y - 1, O_GHOST_EXPL_1);
    cell_explode_skip_voodoo(x + 1, y + 1, O_GHOST_EXPL_1);
    cell_explode_skip_voodoo(x - 1, y + 1, O_GHOST_EXPL_1);
    cell_explode_skip_voodoo(x + 1, y - 1, O_GHOST_EXPL_1);
}

/// A + shaped bomb explosion.
void CaveRendered::bomb_explode(int x, int y) {
    /* the processing of an explosion took pretty much time: processing 5 elements */
    ckdelay_current += 650;
    sound_play(GD_S_BOMB_EXPLOSION, x, y);

    cell_explode_skip_voodoo(x, y, O_BOMB_EXPL_1);
    cell_explode_skip_voodoo(x - 1, y, O_BOMB_EXPL_1);
    cell_explode_skip_voodoo(x + 1, y, O_BOMB_EXPL_1);
    cell_explode_skip_voodoo(x, y + 1, O_BOMB_EXPL_1);
    cell_explode_skip_voodoo(x, y - 1, O_BOMB_EXPL_1);
}

/// Explode the thing at (x,y).
/// Checks the element, and selects the correct exploding type accordingly.
void CaveRendered::explode(int x, int y) {
    /* if this remains false throughout the switch() below, particles will be added
     * in the bottom line of the function. if there are particles already added
     * somewhere, this is set to true. */
    bool particles_added = false;

    switch (get(x, y)) {
        case O_GHOST:
            ghost_explode(x, y);
            break;

        case O_BOMB_TICK_7:
            bomb_explode(x, y);
            break;

        case O_VOODOO:
            voodoo_explode(x, y);
            break;

        case O_NITRO_PACK:
        case O_NITRO_PACK_F:
        case O_NITRO_PACK_EXPLODE:
            nitro_explode(x, y);
            break;

        case O_AMOEBA_2:
            creature_explode(x, y, O_AMOEBA_2_EXPL_1);
            break;

        case O_FALLING_WALL_F:
            creature_explode(x, y, O_EXPLODE_1);
            break;

        case O_ROCKET_1:
        case O_ROCKET_2:
        case O_ROCKET_3:
        case O_ROCKET_4:
            creature_explode(x, y, O_EXPLODE_1);
            break;

        case O_BUTTER_1:
        case O_BUTTER_2:
        case O_BUTTER_3:
        case O_BUTTER_4:
            add_particle_set(x, y, O_PRE_DIA_1);
            particles_added = true;
            creature_explode(x, y, butterfly_explode_to);
            break;

        case O_ALT_BUTTER_1:
        case O_ALT_BUTTER_2:
        case O_ALT_BUTTER_3:
        case O_ALT_BUTTER_4:
            add_particle_set(x, y, O_PRE_DIA_1);
            particles_added = true;
            creature_explode(x, y, alt_butterfly_explode_to);
            break;

        case O_FIREFLY_1:
        case O_FIREFLY_2:
        case O_FIREFLY_3:
        case O_FIREFLY_4:
            creature_explode(x, y, firefly_explode_to);
            break;

        case O_ALT_FIREFLY_1:
        case O_ALT_FIREFLY_2:
        case O_ALT_FIREFLY_3:
        case O_ALT_FIREFLY_4:
            creature_explode(x, y, alt_firefly_explode_to);
            break;

        case O_PLAYER:
        case O_PLAYER_BOMB:
        case O_PLAYER_GLUED:
        case O_PLAYER_STIRRING:
        case O_PLAYER_ROCKET_LAUNCHER:
        case O_PLAYER_PNEUMATIC_LEFT:
        case O_PLAYER_PNEUMATIC_RIGHT:
            creature_explode(x, y, O_EXPLODE_1);
            break;

        case O_STONEFLY_1:
        case O_STONEFLY_2:
        case O_STONEFLY_3:
        case O_STONEFLY_4:
            creature_explode(x, y, stonefly_explode_to);
            break;

        case O_DRAGONFLY_1:
        case O_DRAGONFLY_2:
        case O_DRAGONFLY_3:
        case O_DRAGONFLY_4:
            creature_explode(x, y, dragonfly_explode_to);
            break;

        default:
            g_assert_not_reached();
            break;
    }

    if (!particles_added)
        add_particle_set(x, y, O_EXPLODE_1);
}

/// Explode the element at (x,y)+dir.
/// A simple wrapper for the explode(int x, int y) function without
/// the dir parameter.
void CaveRendered::explode(int x, int y, GdDirectionEnum dir) {
    explode(x + gd_dx[dir], y + gd_dy[dir]);
}


/// The player eats or activates the given element.
/// This function does all things that should happen when the
/// player eats something - increments score, plays sound etc.
/// This function is also used to activate switches, and to collect
/// keys.
/// It returns the remaining element, which is usually space;
/// might be some other thing. (example: DIRT for CLOCK)
/// This does NOT take snap_element into consideration.
/// @param element Element to eat
/// @param x The coordinate of player
/// @param y The coordinate of player
/// @param dir The direction the player is moving
/// @return remaining element
GdElementEnum CaveRendered::player_eat_element(GdElementEnum element, int x, int y, GdDirectionEnum dir) {
    GdElementEnum was_element = element;
    x += gd_dx[dir];
    y += gd_dy[dir];

    switch (element) {
        case O_DIAMOND_KEY:
            diamond_key_collected = true;
            element = O_SPACE;
            break;

            /* KEYS AND DOORS */
        case O_KEY_1:
            ++key1;
            element = O_SPACE;
            break;
        case O_KEY_2:
            ++key2;
            element = O_SPACE;
            break;
        case O_KEY_3:
            ++key3;
            element = O_SPACE;
            break;
        case O_DOOR_1:
            if (key1 == 0)
                element = O_NONE;
            else {
                --key1;
                element = O_SPACE;
            }
            break;
        case O_DOOR_2:
            if (key2 == 0)
                element = O_NONE;
            else {
                --key2;
                element = O_SPACE;
            }
            break;
        case O_DOOR_3:
            if (key3 == 0)
                element = O_NONE;
            else {
                --key3;
                element = O_SPACE;
            }
            break;
            /* SWITCHES */
        case O_CREATURE_SWITCH:     /* creatures change direction. */
            creatures_backwards = !creatures_backwards;
            break;
        case O_EXPANDING_WALL_SWITCH:       /* expanding wall change direction. */
            expanding_wall_changed = !expanding_wall_changed;
            break;
        case O_BITER_SWITCH:        /* biter change delay */
            biter_delay_frame++;
            if (biter_delay_frame == 4)
                biter_delay_frame = 0;
            break;
        case O_REPLICATOR_SWITCH:   /* replicator on/off switch */
            replicators_active = !replicators_active;
            break;
        case O_CONVEYOR_SWITCH: /* conveyor belts on/off */
            conveyor_belts_active = !conveyor_belts_active;
            break;
        case O_CONVEYOR_DIR_SWITCH: /* conveyor belts switch direction */
            conveyor_belts_direction_changed = !conveyor_belts_direction_changed;
            break;
        case O_GRAVITY_SWITCH:
            if (gravity_switch_active && (dir == MV_LEFT || dir == MV_RIGHT || dir == MV_UP || dir == MV_DOWN)) {
                sound_play(GD_S_SWITCH_GRAVITY, x, y);
                gravity_will_change = gravity_change_time * timing_factor;
                gravity_next_direction = dir;
                gravity_switch_active = false;
            } else
                element = O_NONE;
            break;

            /* USUAL STUFF */
        case O_DIRT:
        case O_DIRT2:
        case O_DIRT_SLOPED_UP_RIGHT:
        case O_DIRT_SLOPED_UP_LEFT:
        case O_DIRT_SLOPED_DOWN_LEFT:
        case O_DIRT_SLOPED_DOWN_RIGHT:
        case O_DIRT_BALL:
        case O_DIRT_LOOSE:
        case O_STEEL_EATABLE:
        case O_BRICK_EATABLE:
            element = O_SPACE;
            break;

        case O_SPACE:
        case O_LAVA:    /* player goes into lava, as if it was space */
            element = O_SPACE;
            break;

        case O_SWEET:
            sweet_eaten = true;
            element = O_SPACE;
            break;

        case O_PNEUMATIC_HAMMER:
            got_pneumatic_hammer = true;
            element = O_SPACE;
            break;

        case O_CLOCK:
            /* bonus time */
            time += time_bonus * timing_factor;
            if (time > max_time * timing_factor)
                time -= max_time * timing_factor;
            /* no space, rather a dirt remains there... */
            element = O_DIRT;
            break;

        case O_DIAMOND:
        case O_FLYING_DIAMOND:
            score += diamond_value;
            ++diamonds_collected;
            if (diamonds_needed == diamonds_collected) {
                gate_open = true;
                diamond_value = extra_diamond_value; /* extra is worth more points. */
                gate_open_flash = 1;
                sound_play(GD_S_CRACK, x, y);
            }
            element = O_SPACE;
            break;
        case O_SKELETON:
            ++skeletons_collected;
            for (int i = 0; i < skeletons_worth_diamonds; i++)
                player_eat_element(O_DIAMOND, x, y, MV_STILL);  /* as if player got a diamond */
            element = O_SPACE;
            break;
        case O_OUTBOX:
        case O_INVIS_OUTBOX:
            player_state = GD_PL_EXITED;  /* player now exits the cave! */
            element = O_SPACE;
            break;

        default:
            /* non-eatable, does nothing */
            element = O_NONE;
            break;
    }

    if (element != O_NONE) {
        play_eat_sound_of_element(was_element, x, y);
        play_eat_particle_of_element(was_element, x, y);
    }
    return element;
}

/**
   Process a crazy dream-style teleporter.
   Called from cave_iterate, for a player or a player_bomb.
   Player is standing at px, py, and trying to move in the direction player_move,
   where there is a teleporter.
   We check the whole cave, from (px+1,py), till we get back to (px,py) (by wrapping
   around). The first teleporter we find, and which is suitable, will be the destination.
   @param px The coordinate of the player which tries to move into the teleporter, x.
   @param py The coordinate of the player, y.
   @param player_move The direction he is moving into.
   @return True, if the player is teleported, false, if no suitable teleporter found.
 */
bool CaveRendered::do_teleporter(int px, int py, GdDirectionEnum player_move) {
    int tx = px;
    int ty = py;
    bool teleported = false;
    do {
        /* jump to next element; wrap around columns and rows. */
        tx++;
        if (tx >= w) {
            tx = 0;
            ty++;
            if (ty >= h)
                ty = 0;
        }
        /* if we found a teleporter... */
        if (get(tx, ty) == O_TELEPORTER && is_like_space(tx, ty, player_move)) {
            store(tx, ty, player_move, get(px, py));    /* new player appears near teleporter found */
            store(px, py, O_SPACE); /* current player disappears */
            sound_play(GD_S_TELEPORTER, tx, ty);
            teleported = true;  /* success */
        }
    } while (!teleported && (tx != px || ty != py)); /* loop until we get back to original coordinates */
    return teleported;
}

/**
    Try to push an element.
    Also does move the specified _element_, if possible.
    Up to the caller to move the _player_itself_, as the movement might be a snap, not a real movement.
    @return  true if the push is possible.
*/
bool CaveRendered::do_push(int x, int y, GdDirectionEnum player_move, bool player_fire) {
    GdElementEnum what = get(x, y, player_move);
    GdDirection grav_compat;    /* gravity for falling wall, bladder, ... */
    if (gravity_affects_all)
        grav_compat = gravity;
    else
        grav_compat = MV_DOWN;
    bool result = false;

    /* do a switch on what element is being pushed to determine probability. */
    switch (what) {
        case O_WAITING_STONE:
        case O_STONE:
        case O_NITRO_PACK:
        case O_CHASING_STONE:
        case O_MEGA_STONE:
        case O_FLYING_STONE:
        case O_NUT:
            /* pushing some kind of stone or nut*/
            /* directions possible: 90degrees cw or ccw to current gravity. */
            /* only push if player dir is orthogonal to gravity, ie. gravity down, pushing left&right possible */
            if (player_move == ccw_fourth[gravity] || player_move == cw_fourth[gravity]) {
                int prob;

                prob = 0;
                /* different probabilities for different elements. */
                /* remember that probabilities are 1/million, stored as integers! */
                switch (what) {
                    case O_WAITING_STONE:
                        prob = 1000000; /* waiting stones are light, can always push */
                        break;
                    case O_CHASING_STONE:
                        if (sweet_eaten) /* chasing can be pushed if player is turbo */
                            prob = 1000000; /* with p=1, always push */
                        break;
                    case O_MEGA_STONE:
                        if (mega_stones_pushable_with_sweet && sweet_eaten) /* mega may(!) be pushed if player is turbo */
                            prob = 1000000; /* p=1, always push */
                        break;
                    case O_STONE:
                    case O_NUT:
                    case O_FLYING_STONE:
                    case O_NITRO_PACK:
                        if (sweet_eaten)
                            prob = pushing_stone_prob_sweet; /* probability with sweet */
                        else
                            prob = pushing_stone_prob;  /* probability without sweet. */
                        break;
                    default:
                        g_assert_not_reached();
                        break;
                }

                if (is_like_space(x, y, twice[player_move]) && random.rand_int_range(0, 1000000) < prob) {
                    /* if decided that he is able to push, */
                    play_effect_of_element(what, x + gd_dx[player_move], y + gd_dy[player_move]);
                    /* if pushed a stone, it "bounces". all other elements are simply pushed. */
                    if (what == O_STONE)
                        store(x, y, twice[player_move], stone_bouncing_effect);
                    else
                        store(x, y, twice[player_move], what);
                    result = true;
                }
            }
            break;

        case O_BLADDER:
        case O_BLADDER_1:
        case O_BLADDER_2:
        case O_BLADDER_3:
        case O_BLADDER_4:
        case O_BLADDER_5:
        case O_BLADDER_6:
        case O_BLADDER_7:
        case O_BLADDER_8:
            /* pushing a bladder. keep in mind that after pushing, we always get an O_BLADDER,
             * not an O_BLADDER_x. */
            /* there is no "delayed" state of a bladder, so we use store_dir_no_scanned! */

            /* first check: we cannot push a bladder "up" */
            if (player_move != opposite[grav_compat]) {
                /* pushing a bladder "down". p=player, o=bladder, 1, 2, 3=directions to check. */
                /* player moving in the direction of gravity. */
                /*  p   p  g  */
                /* 2o3  |  |  */
                /*  1   v  v  */
                if (player_move == grav_compat) {
                    if (is_like_space(x, y, twice[player_move])) /* pushing bladder down */
                        store(x, y, twice[player_move], O_BLADDER), result = true;
                    else if (is_like_space(x, y, cw_eighth[grav_compat]))    /* if no space to push down, maybe left (down-left to player) */
                        /* left is "down, turned right (cw)" */
                        store(x, y, cw_eighth[grav_compat], O_BLADDER), result = true;
                    else if (is_like_space(x, y, ccw_eighth[grav_compat]))   /* if not, maybe right (down-right to player) */
                        store(x, y, ccw_eighth[grav_compat], O_BLADDER), result = true;
                }
                /* pushing a bladder "left". p=player, o=bladder, 1, 2, 3=directions to check. */
                /*  3        g */
                /* 1op  <-p  | */
                /*  2        v */
                else if (player_move == cw_fourth[grav_compat]) {
                    if (is_like_space(x, y, twice[cw_fourth[grav_compat]]))  /* pushing it left */
                        store(x, y, twice[cw_fourth[grav_compat]], O_BLADDER), result = true;
                    else if (is_like_space(x, y, cw_eighth[grav_compat]))    /* maybe down, and player will move left */
                        store(x, y, cw_eighth[grav_compat], O_BLADDER), result = true;
                    else if (is_like_space(x, y, cw_eighth[player_move]))    /* maybe up, and player will move left */
                        store(x, y, cw_eighth[player_move], O_BLADDER), result = true;
                }
                /* pushing a bladder "right". p=player, o=bladder, 1, 2, 3=directions to check. */
                /*  3        g */
                /* po1  p-<  | */
                /*  2        v */
                else if (player_move == ccw_fourth[grav_compat]) {
                    if (is_like_space(x, y, twice[player_move])) /* pushing it right */
                        store(x, y, twice[player_move], O_BLADDER), result = true;
                    else if (is_like_space(x, y, ccw_eighth[grav_compat]))   /* maybe down, and player will move right */
                        store(x, y, ccw_eighth[grav_compat], O_BLADDER), result = true;
                    else if (is_like_space(x, y, ccw_eighth[player_move]))   /* maybe up, and player will move right */
                        store(x, y, ccw_eighth[player_move], O_BLADDER), result = true;
                }

                if (result)
                    play_effect_of_element(O_BLADDER, x, y);
            }
            break;

        case O_BOX:
            /* a box is only pushed with the fire pressed */
            if (player_fire) {
                /* but always with 100% probability */
                switch (player_move) {
                    case MV_LEFT:
                    case MV_RIGHT:
                    case MV_UP:
                    case MV_DOWN:
                        /* pushing in some dir, two steps in that dir - is there space? */
                        if (is_like_space(x, y, twice[player_move])) {
                            /* yes, so push. */
                            store(x, y, twice[player_move], O_BOX);
                            result = true;
                            sound_play(GD_S_BOX_PUSH, x, y);
                        }
                        break;
                    default:
                        /* push in no other directions possible */
                        break;
                }
            }
            break;

            /* pushing of other elements not possible */
        default:
            break;
    }

    return result;
}

/// clear these to no sound; and they will be set during iteration.
void CaveRendered::clear_sounds() {
    sound1 = SoundWithPos(GD_S_NONE, 0, 0);
    sound2 = SoundWithPos(GD_S_NONE, 0, 0);
    sound3 = SoundWithPos(GD_S_NONE, 0, 0);
}

/// Try to make an element start falling.
/// When an element starts to fall, there is no particle effect. Only when bouncing and when rolling
/// down sloped things. But sound there is.
/// @param x The x coordinate of the element.
/// @param y The y coordinate of the element.
/// @param falling_direction The direction to start "falling" to.
///         Down (=gravity) for a stone, Up (=opposite of gravity) for a flying stone, for example.
/// @param falling_element The falling pair of the element (O_STONE -> O_STONE_F)
void CaveRendered::do_start_fall(int x, int y, GdDirectionEnum falling_direction, GdElementEnum falling_element) {
    if (gravity_disabled)
        return;

    if (is_like_space(x, y, falling_direction)) {    /* beginning to fall */
        play_effect_of_element(get(x, y), x, y, MV_STILL, false /* no particles */);
        move(x, y, falling_direction, falling_element);
    }
    /* check if it is on a sloped element, and it can roll. */
    /* for example, sloped wall looks like: */
    /*  /| */
    /* /_| */
    /* this is tagged as sloped up&left. */
    /* first check if the stone or diamond is coming from "up" (ie. opposite of gravity) */
    /* then check the direction to roll (left or right) */
    /* this way, gravity can also be pointing right, and the above slope will work as one would expect */
    else if (sloped(x, y, falling_direction, opposite[falling_direction])) {    /* rolling down, if sitting on a sloped object  */
        if (sloped(x, y, falling_direction, cw_fourth[falling_direction]) && is_like_space(x, y, cw_fourth[falling_direction]) && is_like_space(x, y, cw_eighth[falling_direction])) {
            /* rolling left? - keep in mind that ccw_fourth rotates gravity ccw, so here we use cw_fourth */
            play_effect_of_element(get(x, y), x, y, MV_STILL, false /* no particles */);
            move(x, y, cw_fourth[falling_direction], falling_element);
        } else if (sloped(x, y, falling_direction, ccw_fourth[falling_direction]) && is_like_space(x, y, ccw_fourth[falling_direction]) && is_like_space(x, y, ccw_eighth[falling_direction])) {
            /* rolling right? */
            play_effect_of_element(get(x, y), x, y, MV_STILL, false /* no particles */);
            move(x, y, ccw_fourth[falling_direction], falling_element);
        }
    }
}

/// When the element at (x,y) is falling in the direction fall_dir,
/// check if it crushes a voodoo below it. If yes, explode the voodoo,
/// and return true. Otherwise return false.
/// @return true if voodoo crushed.
bool CaveRendered::do_fall_try_crush_voodoo(int x, int y, GdDirectionEnum fall_dir) {
    if (get(x, y, fall_dir) == O_VOODOO && voodoo_dies_by_stone) {
        /* this is a 1stB-style vodo. explodes by stone, collects diamonds */
        explode(x, y, fall_dir);
        return true;
    } else
        return false;
}

/// When the element at (x,y) is falling in the direction fall_dir,
/// check if the voodoo below it can eat it. If yes, the voodoo eats it.
/// @return true if successful, false if voodoo does not eat the element.
bool CaveRendered::do_fall_try_eat_voodoo(int x, int y, GdDirectionEnum fall_dir) {
    if (get(x, y, fall_dir) == O_VOODOO && voodoo_collects_diamonds) {
        /* this is a 1stB-style voodoo. explodes by stone, collects diamonds */
        player_eat_element(O_DIAMOND, x, y, fall_dir);  /* as if player got diamond */
        store(x, y, O_SPACE);   /* diamond disappears */
        return true;
    } else
        return false;
}

/// Element at (x,y) is falling. Try to crack nut under it.
/// If successful, nut is cracked, and the element is bounced (stops moving).
/// @param fall_dir The direction the element is falling in.
/// @param bouncing The element which it is converted to, if it has cracked a nut.
/// @return True, if nut is cracked.
bool CaveRendered::do_fall_try_crack_nut(int x, int y, GdDirectionEnum fall_dir, GdElementEnum bouncing) {
    if (get(x, y, fall_dir) == O_NUT || get(x, y, fall_dir) == O_NUT_F) {
        /* stones */
        store(x, y, bouncing);
        store(x, y, fall_dir, nut_turns_to_when_crushed);
        sound_play(GD_S_NUT_CRACK, x, y);
        return true;
    } else
        return false;
}

/// For a falling element, try if a magic wall is under it.
/// If yes, process element using the magic wall, and return true.
/// @param fall_dir The direction the element is falling to.
/// @param magic The element a magic wall turns it to.
/// @return If The element is processed by the magic wall.
bool CaveRendered::do_fall_try_magic(int x, int y, GdDirectionEnum fall_dir, GdElementEnum magic) {
    if (get(x, y, fall_dir) == O_MAGIC_WALL) {
        play_effect_of_element(O_DIAMOND, x, y, MV_STILL, false);   /* always play diamond sound, but with no particle effect */
        if (magic_wall_state == GD_MW_DORMANT)
            magic_wall_state = GD_MW_ACTIVE;
        if (magic_wall_state == GD_MW_ACTIVE && is_like_space(x, y, twice[fall_dir])) {
            /* if magic wall active and place underneath, it turns element into anything the effect says to do. */
            store(x, y, twice[fall_dir], magic);
        }
        store(x, y, O_SPACE);   /* active or non-active or anything, element falling in will always disappear */
        if (magic_wall_breakscan && amoeba_state == GD_AM_AWAKE)
            convert_amoeba_this_frame = true;
        return true;
    } else
        return false;
}

/// For a falling element, test if an explodable element is under it; if yes, explode it, and return yes.
/// @return True, if element at (x,y)+fall_dir is exploded.
bool CaveRendered::do_fall_try_crush(int x, int y, GdDirectionEnum fall_dir) {
    if (explodes_by_hit(x, y, fall_dir)) {
        explode(x, y, fall_dir);
        return true;
    } else
        return false;
}

/**
 * For a falling element, try if a sloped element is under it.
 * Move element if possible, or bounce element.
 * If there are two directions possible for the element to roll to, left is preferred.
 * If no rolling is possible, it is converted to a bouncing element.
 * So this always "does something" with the element, and this should be the last
 * function to call when checking what happens to a falling element.
 */
void CaveRendered::do_fall_roll_or_stop(int x, int y, GdDirectionEnum fall_dir, GdElementEnum bouncing) {
    if (is_like_space(x, y, fall_dir))   { /* falling further */
        move(x, y, fall_dir, get(x, y));
        return;
    }
    /* check if it is on a sloped element, and it can roll. */
    /* for example, sloped wall looks like: */
    /*  /| */
    /* /_| */
    /* this is tagged as sloped up&left. */
    /* first check if the stone or diamond is coming from "up" (ie. opposite of gravity) */
    /* then check the direction to roll (left or right) */
    /* this way, gravity can also be pointing right, and the above slope will work as one would expect */
    if (sloped(x, y, fall_dir, opposite[fall_dir])) {   /* sloped element, falling to left or right */
        if (sloped(x, y, fall_dir, cw_fourth[fall_dir]) && is_like_space(x, y, cw_eighth[fall_dir]) && is_like_space(x, y, cw_fourth[fall_dir])) {
            play_effect_of_element(get(x, y), x, y);
            move(x, y, cw_fourth[fall_dir], get(x, y)); /* try to roll left first - cw_fourth is because direction is down!!! left is clockwise to that */
        } else if (sloped(x, y, fall_dir, ccw_fourth[fall_dir]) && is_like_space(x, y, ccw_eighth[fall_dir]) && is_like_space(x, y, ccw_fourth[fall_dir])) {
            play_effect_of_element(get(x, y), x, y);
            move(x, y, ccw_fourth[fall_dir], get(x, y));    /* if not, try to roll right */
        } else {
            /* cannot roll in any direction, so it stops */
            play_effect_of_element(get(x, y), x, y);
            store(x, y, bouncing);
        }
        return;
    }

    /* any other element, stops */
    play_effect_of_element(get(x, y), x, y);
    store(x, y, bouncing);
    return;
}


/// Process a cave - one iteration.
/// @param player_move The direction the player moves to.
/// @param player_fire True, if the fire button is pressed.
/// @param suicide True, if the suicide button is pressed.
/// @return A new GdDirectionEnum, which might be changed to not have diagonal movements. This is to make stored replays neater.
GdDirectionEnum CaveRendered::iterate(GdDirectionEnum player_move, bool player_fire, bool suicide) {
    int ymin, ymax; /* for border scan */
    bool amoeba_found_enclosed, amoeba_2_found_enclosed;    /* amoeba found to be enclosed. if not, this is cleared */
    int amoeba_count, amoeba_2_count;       /* counting the number of amoebas. after scan, check if too much */
    bool inbox_toggle;
    bool start_signal;
    GdDirection grav_compat;    /* gravity for falling wall, bladder, ... */
    /* directions for o_something_1, 2, 3 and 4 (creatures) */
    static GdDirection const creature_dir[] = { MV_LEFT, MV_UP, MV_RIGHT, MV_DOWN };
    static GdDirection const creature_chdir[] = { MV_RIGHT, MV_DOWN, MV_LEFT, MV_UP };
    int time_decrement_sec;
    GdElement biter_try[] = { O_DIRT, biter_eat, O_SPACE, O_STONE }; /* biters eating elements preference, they try to go in this order */

    clear_sounds();

    if (gravity_affects_all)
        grav_compat = gravity;
    else
        grav_compat = MV_DOWN;

    /* if diagonal movements not allowed, */
    /* horizontal movements have precedence. [BROADRIBB] */
    if (!diagonal_movements) {
        switch (player_move) {
            case MV_UP_RIGHT:
            case MV_DOWN_RIGHT:
                player_move = MV_RIGHT;
                break;
            case MV_UP_LEFT:
            case MV_DOWN_LEFT:
                player_move = MV_LEFT;
                break;
            default:
                /* no correction needed */
                break;
        }
    }

    /* increment this. if the scan routine comes across player, clears it (sets to zero). */
    if (player_seen_ago < 100)
        ++player_seen_ago;

    if (pneumatic_hammer_active_delay > 0)
        --pneumatic_hammer_active_delay;

    /* inboxes and outboxes flash with the rhythm of the game, not the display.
     * also, a player can be born only from an open, not from a steel-wall-like inbox. */
    inbox_flash_toggle = !inbox_flash_toggle;
    inbox_toggle = inbox_flash_toggle;

    if (gate_open_flash > 0)
        --gate_open_flash;

    /* score collected this frame */
    score = 0;

    /* to implement buggy bd1 amoeba+magic wall behaviour */
    convert_amoeba_this_frame = false;

    /* suicide only kills the active player */
    /* player_x, player_y was set by the previous iterate routine, or the cave setup. */
    /* we must check if there is a player or not - he may have exploded or something like that */
    if (suicide && player_state == GD_PL_LIVING && is_player(player_x, player_y))
        store(player_x, player_y, O_EXPLODE_1);

    /* check for walls reappearing */
    if (hammered_walls_reappear) {
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++) {
                /* timer for the cell > 0? */
                if (hammered_reappear(x, y) > 0) {
                    /* decrease timer */
                    hammered_reappear(x, y)--;
                    /* check if it became zero */
                    if (hammered_reappear(x, y) == 0) {
                        store(x, y, O_BRICK);
                        sound_play(GD_S_WALL_REAPPEAR, x, y);
                    }
                }
            }
    }

    /* variables to check during the scan */
    amoeba_found_enclosed = true;   /* will be set to false if any of the amoeba is found free. */
    amoeba_2_found_enclosed = true;
    amoeba_count = 0;
    amoeba_2_count = 0;
    ckdelay_current = 0;
    time_decrement_sec = 0;

    /* check whether to scan the first and last line */
    if (border_scan_first_and_last) {
        ymin = 0;
        ymax = h - 1;
    } else {
        ymin = 1;
        ymax = h - 2;
    }
    /* the cave scan routine */
    for (int y = ymin; y <= ymax; y++)
        for (int x = 0; x < w; x++) {
            /* if we find a scanned element, change it to the normal one, and that's all. */
            /* this is required, for example for chasing stones, which have moved, always passing slime! */
            if (is_scanned(x, y)) {
                unscan(x, y);
                continue;
            }

            /* add the ckdelay correction value for every element seen. */
            ckdelay_current += gd_element_properties[get(x, y)].ckdelay;

            switch (get(x, y)) {
                    /*
                     *  P L A Y E R S
                     */
                case O_PLAYER:
                    if (kill_player) {
                        explode(x, y);
                        break;
                    }
                    player_seen_ago = 0;
                    /* bd4 intermission caves have many players. so if one of them has exited,
                     * do not change the flag anymore. so this if () is needed */
                    if (player_state != GD_PL_EXITED)
                        player_state = GD_PL_LIVING;

                    /* check for pneumatic hammer things */
                    /* 1) press fire, 2) have pneumatic hammer 4) space on left or right for hammer 5) stand on something */
                    if (player_fire && got_pneumatic_hammer && is_like_space(x, y, player_move)
                            && !is_like_space(x, y, MV_DOWN)) {
                        if (player_move == MV_LEFT && can_be_hammered(x, y, MV_DOWN_LEFT)) {
                            pneumatic_hammer_active_delay = pneumatic_hammer_frame;
                            store(x, y, MV_LEFT, O_PNEUMATIC_ACTIVE_LEFT);
                            store(x, y, O_PLAYER_PNEUMATIC_LEFT);
                            break;  /* finished. */
                        }
                        if (player_move == MV_RIGHT && can_be_hammered(x, y, MV_DOWN_RIGHT)) {
                            pneumatic_hammer_active_delay = pneumatic_hammer_frame;
                            store(x, y, MV_RIGHT, O_PNEUMATIC_ACTIVE_RIGHT);
                            store(x, y, O_PLAYER_PNEUMATIC_RIGHT);
                            break;  /* finished. */
                        }
                    }

                    if (player_move != MV_STILL) {
                        /* only do every check if he is not moving */
                        GdElementEnum what = get(x, y, player_move);
                        GdElementEnum remains = O_NONE; /* o_none in this variable will mean that there is no change. */
                        bool push;

                        /* if we are 'eating' a teleporter, and the function returns true (teleporting worked), break here */
                        if (what == O_TELEPORTER && do_teleporter(x, y, player_move))
                            break;

                        /* try to push element; if successful, break  */
                        push = do_push(x, y, player_move, player_fire);
                        if (push)
                            remains = O_SPACE;
                        else
                            switch (what) {
                                case O_BOMB:
                                    /* if its a bomb, remember he now has one. */
                                    /* we do not change the "remains" and "what" variables, so that part of the code will be ineffective */
                                    sound_play(GD_S_BOMB_COLLECT, x, y);
                                    store(x, y, player_move, O_SPACE);
                                    if (player_fire)
                                        store(x, y, O_PLAYER_BOMB);
                                    else
                                        move(x, y, player_move, O_PLAYER_BOMB);
                                    break;

                                case O_ROCKET_LAUNCHER:
                                    /* if its a rocket launcher, remember he now has one. */
                                    /* we do not change the "remains" and "what" variables, so that part of the code will be ineffective */
                                    sound_play(GD_S_BOMB_COLLECT, x, y);
                                    store(x, y, player_move, O_SPACE);
                                    if (player_fire)
                                        store(x, y, O_PLAYER_ROCKET_LAUNCHER);
                                    else
                                        move(x, y, player_move, O_PLAYER_ROCKET_LAUNCHER);
                                    break;

                                case O_POT:
                                    /* we do not change the "remains" and "what" variables, so that part of the code will be ineffective */
                                    if (!player_fire && !gravity_switch_active && skeletons_collected >= skeletons_needed_for_pot) {
                                        skeletons_collected -= skeletons_needed_for_pot;
                                        move(x, y, player_move, O_PLAYER_STIRRING);
                                        gravity_disabled = true;
                                    }
                                    break;

                                default:
                                    /* get element - process others. if cannot get, player_eat_element will return the same */
                                    remains = player_eat_element(what, x, y, player_move);
                                    break;
                            }

                        /* if anything changed, apply the change. */
                        if (remains != O_NONE) {
                            /* if snapping anything and we have snapping explosions set. but these is not true for pushing. */
                            if (remains == O_SPACE && player_fire && !push)
                                remains = snap_element;
                            if (remains != O_SPACE || player_fire)
                                /* if any other element than space, player cannot move. also if pressing fire, will not move. */
                                store(x, y, player_move, remains);
                            else
                                /* if space remains there, the player moves. */
                                move(x, y, player_move, O_PLAYER);
                        }

                    }
                    break;

                case O_PLAYER_BOMB:
                    /* much simpler; cannot snap-push stones */
                    if (kill_player) {
                        explode(x, y);
                        break;
                    }
                    player_seen_ago = 0;
                    /* bd4 intermission caves have many players. so if one of them has exited,
                     * do not change the flag anymore. so this if () is needed */
                    if (player_state != GD_PL_EXITED)
                        player_state = GD_PL_LIVING;

                    if (player_move != MV_STILL) {  /* if the player does not move, nothing to do */
                        GdElementEnum what = get(x, y, player_move);
                        GdElementEnum remains = O_NONE;

                        if (player_fire) {
                            /* placing a bomb into empty space or dirt */
                            if (is_like_space(x, y, player_move) || is_like_dirt(x, y, player_move)) {
                                store(x, y, player_move, O_BOMB_TICK_1);
                                /* placed bomb, he is normal player again */
                                store(x, y, O_PLAYER);
                                sound_play(GD_S_BOMB_PLACE, x, y);
                            }
                            break;
                        }

                        /* pushing and collecting */
                        /* if we are 'eating' a teleporter, and the function returns true (teleporting worked), break here */
                        if (what == O_TELEPORTER && do_teleporter(x, y, player_move))
                            break;

                        if (do_push(x, y, player_move, false))  /* player fire is false... */
                            remains = O_SPACE;
                        else {
                            /* get element. if cannot get, player_eat_element will return the same */
                            remains = player_eat_element(what, x, y, player_move);
                        }

                        /* if something changed, OR there is space, move. */
                        if (remains != O_NONE) {
                            /* if anything changed, apply the change. */
                            move(x, y, player_move, O_PLAYER_BOMB);
                        }
                    }
                    break;

                case O_PLAYER_ROCKET_LAUNCHER:
                    /* much simpler; cannot snap-push stones */
                    if (kill_player) {
                        explode(x, y);
                        break;
                    }
                    player_seen_ago = 0;
                    /* bd4 intermission caves have many players. so if one of them has exited,
                     * do not change the flag anymore. so this if () is needed */
                    if (player_state != GD_PL_EXITED)
                        player_state = GD_PL_LIVING;

                    /* firing a rocket? */
                    if (player_move != MV_STILL) {    /* if the player does not move, nothing to do */
                        GdElementEnum what = get(x, y, player_move);
                        GdElementEnum remains = O_NONE;

                        /* to fire a rocket, diagonal movement should not be allowed. so either x or y must be zero */
                        /* placing a bomb into empty space */
                        if (player_fire) {
                            if (is_like_space(x, y, player_move)) {
                                switch (player_move) {
                                    case MV_RIGHT:
                                        store(x, y, player_move, O_ROCKET_1);
                                        if (!infinite_rockets)
                                            store(x, y, O_PLAYER);
                                        break;
                                    case MV_UP:
                                        store(x, y, player_move, O_ROCKET_2);
                                        if (!infinite_rockets)
                                            store(x, y, O_PLAYER);
                                        break;
                                    case MV_LEFT:
                                        store(x, y, player_move, O_ROCKET_3);
                                        if (!infinite_rockets)
                                            store(x, y, O_PLAYER);
                                        break;
                                    case MV_DOWN:
                                        store(x, y, player_move, O_ROCKET_4);
                                        if (!infinite_rockets)
                                            store(x, y, O_PLAYER);
                                        break;
                                    default:
                                        /* cannot fire in other directions */
                                        break;
                                }
                                sound_play(GD_S_BOMB_PLACE, x, y);
                            }
                            /* a player with rocket launcher cannot snap elements, so stop here */
                            break;
                        }

                        /* pushing and collecting */
                        /* if we are 'eating' a teleporter, and the function returns true (teleporting worked), break here */
                        if (what == O_TELEPORTER && do_teleporter(x, y, player_move))
                            break;

                        if (do_push(x, y, player_move, false))  /* player fire is false... */
                            remains = O_SPACE;
                        else {
                            /* get element. if cannot get, player_eat_element will return the same */
                            remains = player_eat_element(what, x, y, player_move);
                        }

                        /* if something changed, OR there is space, move. */
                        if (remains != O_NONE) {
                            /* if anything changed, apply the change. */
                            move(x, y, player_move, O_PLAYER_ROCKET_LAUNCHER);
                        }
                    }
                    break;

                case O_PLAYER_STIRRING:
                    if (kill_player) {
                        explode(x, y);
                        break;
                    }
                    sound_play(GD_S_STIRRING, x, y); /* stirring sound */
                    player_seen_ago = 0;
                    /* bd4 intermission caves have many players. so if one of them has exited,
                     * do not change the flag anymore. so this if () is needed */
                    if (player_state != GD_PL_EXITED)
                        player_state = GD_PL_LIVING;
                    if (player_fire) {
                        /* player "exits" stirring the pot by pressing fire */
                        gravity_disabled = false;
                        store(x, y, O_PLAYER);
                        gravity_switch_active = true;
                    }
                    break;

                    /* player holding pneumatic hammer */
                case O_PLAYER_PNEUMATIC_LEFT:
                case O_PLAYER_PNEUMATIC_RIGHT:
                    /* usual player stuff */
                    if (kill_player) {
                        explode(x, y);
                        break;
                    }
                    player_seen_ago = 0;
                    if (player_state != GD_PL_EXITED)
                        player_state = GD_PL_LIVING;
                    if (pneumatic_hammer_active_delay == 0) /* if hammering time is up, becomes a normal player again. */
                        store(x, y, O_PLAYER);
                    break;

                    /* the active pneumatic hammer itself */
                case O_PNEUMATIC_ACTIVE_RIGHT:
                case O_PNEUMATIC_ACTIVE_LEFT:
                    if (pneumatic_hammer_active_delay > 0)
                        sound_play(GD_S_PNEUMATIC_HAMMER, x, y);
                    if (pneumatic_hammer_active_delay == 0) {
                        GdElementEnum new_elem;

                        store(x, y, O_SPACE);   /* pneumatic hammer element disappears */
                        /* which is the new element which appears after that one is hammered? */
                        new_elem = gd_element_get_hammered(get(x, y, MV_DOWN));
                        /* if there is a new element, display it */
                        /* O_NONE might be returned, for example if the element being hammered explodes during hammering (by a nearby explosion) */
                        if (new_elem != O_NONE) {
                            store(x, y, MV_DOWN, new_elem);

                            /* and if walls reappear, remember it in array */
                            /* y+1 is down */
                            if (hammered_walls_reappear)
                                hammered_reappear(x, (y + 1) % h) = hammered_wall_reappear_frame;
                        }
                    }
                    break;


                    /*
                     *  S T O N E S,   D I A M O N D S
                     */
                case O_STONE:   /* standing stone */
                    do_start_fall(x, y, gravity, stone_falling_effect);
                    break;

                case O_MEGA_STONE:  /* standing mega_stone */
                    do_start_fall(x, y, gravity, O_MEGA_STONE_F);
                    break;

                case O_DIAMOND: /* standing diamond */
                    do_start_fall(x, y, gravity, diamond_falling_effect);
                    break;

                case O_NUT: /* standing nut */
                    do_start_fall(x, y, gravity, O_NUT_F);
                    break;

                case O_DIRT_BALL:   /* standing dirt ball */
                    do_start_fall(x, y, gravity, O_DIRT_BALL_F);
                    break;

                case O_DIRT_LOOSE:  /* standing loose dirt */
                    do_start_fall(x, y, gravity, O_DIRT_LOOSE_F);
                    break;

                case O_FLYING_STONE:    /* standing stone */
                    do_start_fall(x, y, opposite[gravity], O_FLYING_STONE_F);
                    break;

                case O_FLYING_DIAMOND:  /* standing diamond */
                    do_start_fall(x, y, opposite[gravity], O_FLYING_DIAMOND_F);
                    break;

                    /*
                     *  F A L L I N G    E L E M E N T S,    F L Y I N G   S T O N E S,   D I A M O N D S
                     */
                case O_DIRT_BALL_F: /* falling dirt ball */
                    if (!gravity_disabled)
                        do_fall_roll_or_stop(x, y, gravity, O_DIRT_BALL);
                    break;

                case O_DIRT_LOOSE_F:    /* falling loose dirt */
                    if (!gravity_disabled)
                        do_fall_roll_or_stop(x, y, gravity, O_DIRT_LOOSE);
                    break;

                case O_STONE_F: /* falling stone */
                    if (!gravity_disabled) {
                        if (do_fall_try_crush_voodoo(x, y, gravity)) break;
                        if (do_fall_try_crack_nut(x, y, gravity, stone_bouncing_effect)) break;
                        if (do_fall_try_magic(x, y, gravity, magic_stone_to)) break;
                        if (do_fall_try_crush(x, y, gravity)) break;
                        do_fall_roll_or_stop(x, y, gravity, stone_bouncing_effect);
                    }
                    break;

                case O_MEGA_STONE_F:    /* falling mega */
                    if (!gravity_disabled) {
                        if (do_fall_try_crush_voodoo(x, y, gravity)) break;
                        if (do_fall_try_crack_nut(x, y, gravity, O_MEGA_STONE)) break;
                        if (do_fall_try_magic(x, y, gravity, magic_mega_stone_to)) break;
                        if (do_fall_try_crush(x, y, gravity)) break;
                        do_fall_roll_or_stop(x, y, gravity, O_MEGA_STONE);
                    }
                    break;

                case O_DIAMOND_F:   /* falling diamond */
                    if (!gravity_disabled) {
                        if (do_fall_try_eat_voodoo(x, y, gravity)) break;
                        if (do_fall_try_magic(x, y, gravity, magic_diamond_to)) break;
                        if (do_fall_try_crush(x, y, gravity)) break;
                        do_fall_roll_or_stop(x, y, gravity, diamond_bouncing_effect);
                    }
                    break;

                case O_NUT_F:   /* falling nut */
                    if (!gravity_disabled) {
                        if (do_fall_try_magic(x, y, gravity, magic_nut_to)) break;
                        if (do_fall_try_crush(x, y, gravity)) break;
                        do_fall_roll_or_stop(x, y, gravity, O_NUT);
                    }
                    break;

                case O_FLYING_STONE_F:  /* falling stone */
                    if (!gravity_disabled) {
                        GdDirectionEnum fall_dir = opposite[gravity];

                        if (do_fall_try_crush_voodoo(x, y, fall_dir)) break;
                        if (do_fall_try_crack_nut(x, y, fall_dir, O_FLYING_STONE)) break;
                        if (do_fall_try_magic(x, y, fall_dir, magic_flying_stone_to)) break;
                        if (do_fall_try_crush(x, y, fall_dir)) break;
                        do_fall_roll_or_stop(x, y, fall_dir, O_FLYING_STONE);
                    }
                    break;

                case O_FLYING_DIAMOND_F:    /* falling diamond */
                    if (!gravity_disabled) {
                        GdDirectionEnum fall_dir = opposite[gravity];

                        if (do_fall_try_eat_voodoo(x, y, fall_dir)) break;
                        if (do_fall_try_magic(x, y, fall_dir, magic_flying_diamond_to)) break;
                        if (do_fall_try_crush(x, y, fall_dir)) break;
                        do_fall_roll_or_stop(x, y, fall_dir, O_FLYING_DIAMOND);
                    }
                    break;


                    /*
                     * N I T R O    P A C K
                     */
                case O_NITRO_PACK:  /* standing nitro pack */
                    do_start_fall(x, y, gravity, O_NITRO_PACK_F);
                    break;

                case O_NITRO_PACK_F:    /* falling nitro pack */
                    if (!gravity_disabled) {
                        if (is_like_space(x, y, gravity))    /* if space, falling further */
                            move(x, y, gravity, O_NITRO_PACK_F);
                        else if (do_fall_try_magic(x, y, gravity, magic_nitro_pack_to)) {
                            /* try magic wall; if true, function did the work */
                        } else if (is_like_dirt(x, y, gravity)) {
                            /* falling on a dirt, it does NOT explode - just stops at its place. */
                            store(x, y, O_NITRO_PACK);
                            play_effect_of_element(O_NITRO_PACK, x, y);
                        } else
                            /* falling on any other element it explodes */
                            explode(x, y);
                    }
                    break;

                case O_NITRO_PACK_EXPLODE:  /* a triggered nitro pack */
                    explode(x, y);
                    break;


                    /*
                     *  C R E A T U R E S
                     */

                case O_COW_1:
                case O_COW_2:
                case O_COW_3:
                case O_COW_4:
                    /* if cannot move in any direction, becomes an enclosed cow */
                    if (!is_like_space(x, y, MV_UP) && !is_like_space(x, y, MV_DOWN)
                            && !is_like_space(x, y, MV_LEFT) && !is_like_space(x, y, MV_RIGHT))
                        store(x, y, O_COW_ENCLOSED_1);
                    else {
                        /* THIS IS THE CREATURE MOVE thing copied. */
                        GdDirection const *creature_move;
                        bool ccw = rotates_ccw(x, y); /* check if default is counterclockwise */
                        GdElementEnum base; /* base element number (which is like O_***_1) */
                        int dir, dirn, dirp;    /* direction */

                        base = O_COW_1;

                        dir = get(x, y) - base; /* facing where */
                        creature_move = creatures_backwards ? creature_chdir : creature_dir;

                        /* now change direction if backwards */
                        if (creatures_backwards)
                            ccw = !ccw;

                        if (ccw) {
                            dirn = (dir + 3) & 3; /* fast turn */
                            dirp = (dir + 1) & 3; /* slow turn */
                        } else {
                            dirn = (dir + 1) & 3; /* fast turn */
                            dirp = (dir + 3) & 3; /* slow turn */
                        }

                        if (is_like_space(x, y, creature_move[dirn]))
                            move(x, y, creature_move[dirn], (GdElementEnum)(base + dirn));  /* turn and move to preferred dir */
                        else if (is_like_space(x, y, creature_move[dir]))
                            move(x, y, creature_move[dir], (GdElementEnum)(base + dir)); /* go on */
                        else
                            store(x, y, GdElementEnum(base + dirp)); /* turn in place if nothing else possible */
                    }
                    break;
                    /* enclosed cows wait some time before turning to a skeleton */
                case O_COW_ENCLOSED_1:
                case O_COW_ENCLOSED_2:
                case O_COW_ENCLOSED_3:
                case O_COW_ENCLOSED_4:
                case O_COW_ENCLOSED_5:
                case O_COW_ENCLOSED_6:
                    if (is_like_space(x, y, MV_UP) || is_like_space(x, y, MV_LEFT) || is_like_space(x, y, MV_RIGHT) || is_like_space(x, y, MV_DOWN))
                        store(x, y, O_COW_1);
                    else
                        next(x, y);
                    break;
                case O_COW_ENCLOSED_7:
                    if (is_like_space(x, y, MV_UP) || is_like_space(x, y, MV_LEFT) || is_like_space(x, y, MV_RIGHT) || is_like_space(x, y, MV_DOWN))
                        store(x, y, O_COW_1);
                    else
                        store(x, y, O_SKELETON);
                    break;

                case O_FIREFLY_1:
                case O_FIREFLY_2:
                case O_FIREFLY_3:
                case O_FIREFLY_4:
                case O_ALT_FIREFLY_1:
                case O_ALT_FIREFLY_2:
                case O_ALT_FIREFLY_3:
                case O_ALT_FIREFLY_4:
                case O_BUTTER_1:
                case O_BUTTER_2:
                case O_BUTTER_3:
                case O_BUTTER_4:
                case O_ALT_BUTTER_1:
                case O_ALT_BUTTER_2:
                case O_ALT_BUTTER_3:
                case O_ALT_BUTTER_4:
                case O_STONEFLY_1:
                case O_STONEFLY_2:
                case O_STONEFLY_3:
                case O_STONEFLY_4:
                    /* check if touches a voodoo */
                    if (get(x, y, MV_LEFT) == O_VOODOO || get(x, y, MV_RIGHT) == O_VOODOO || get(x, y, MV_UP) == O_VOODOO || get(x, y, MV_DOWN) == O_VOODOO)
                        voodoo_touched = true;
                    /* check if touches something bad and should explode (includes voodoo by the flags) */
                    if (blows_up_flies(x, y, MV_DOWN) || blows_up_flies(x, y, MV_UP)
                            || blows_up_flies(x, y, MV_LEFT) || blows_up_flies(x, y, MV_RIGHT))
                        explode(x, y);
                    /* otherwise move */
                    else {
                        GdDirection const *creature_move;
                        bool ccw = rotates_ccw(x, y); /* check if default is counterclockwise */
                        GdElementEnum base;         /* base element number (which is like O_***_1) */
                        int dir, dirn, dirp;        /* direction */

                        if (get(x, y) >= O_FIREFLY_1 && get(x, y) <= O_FIREFLY_4)
                            base = O_FIREFLY_1;
                        else if (get(x, y) >= O_BUTTER_1 && get(x, y) <= O_BUTTER_4)
                            base = O_BUTTER_1;
                        else if (get(x, y) >= O_STONEFLY_1 && get(x, y) <= O_STONEFLY_4)
                            base = O_STONEFLY_1;
                        else if (get(x, y) >= O_ALT_FIREFLY_1 && get(x, y) <= O_ALT_FIREFLY_4)
                            base = O_ALT_FIREFLY_1;
                        else if (get(x, y) >= O_ALT_BUTTER_1 && get(x, y) <= O_ALT_BUTTER_4)
                            base = O_ALT_BUTTER_1;
                        else
                            g_assert_not_reached();

                        dir = get(x, y) - base; /* facing where */
                        creature_move = creatures_backwards ? creature_chdir : creature_dir;

                        /* now change direction if backwards */
                        if (creatures_backwards)
                            ccw = !ccw;

                        if (ccw) {
                            dirn = (dir + 3) & 3; /* fast turn */
                            dirp = (dir + 1) & 3; /* slow turn */
                        } else {
                            dirn = (dir + 1) & 3; /* fast turn */
                            dirp = (dir + 3) & 3; /* slow turn */
                        }

                        if (is_like_space(x, y, creature_move[dirn]))
                            move(x, y, creature_move[dirn], (GdElementEnum)(base + dirn));  /* turn and move to preferred dir */
                        else if (is_like_space(x, y, creature_move[dir]))
                            move(x, y, creature_move[dir], (GdElementEnum)(base + dir)); /* go on */
                        else
                            store(x, y, GdElementEnum(base + dirp)); /* turn in place if nothing else possible */
                    }
                    break;

                case O_WAITING_STONE:
                    if (is_like_space(x, y, grav_compat)) {  /* beginning to fall */
                        /* it wakes up. */
                        move(x, y, grav_compat, O_CHASING_STONE);
                    } else if (sloped(x, y, grav_compat, opposite[grav_compat])) {  /* rolling down a brick wall or a stone */
                        if (sloped(x, y, grav_compat, cw_fourth[grav_compat]) && is_like_space(x, y, cw_fourth[grav_compat]) && is_like_space(x, y, cw_eighth[grav_compat])) {
                            /* maybe rolling left - see case O_STONE to understand why we use cw_fourth here */
                            move(x, y, cw_fourth[grav_compat], O_WAITING_STONE);
                        } else if (sloped(x, y, grav_compat, ccw_fourth[grav_compat]) && is_like_space(x, y, ccw_fourth[grav_compat]) && is_like_space(x, y, ccw_eighth[grav_compat])) {
                            /* or maybe right */
                            move(x, y, ccw_fourth[grav_compat], O_WAITING_STONE);
                        }
                    }
                    break;

                case O_CHASING_STONE: {
                    int px = player_x_mem[0];
                    int py = player_y_mem[0];
                    bool horizontal = random.rand_boolean();
                    bool dont_move = false;
                    int i = 3;

                    /* try to move... */
                    while (1) {
                        if (horizontal) {   /*********************************/
                            /* check for a horizontal movement */
                            if (px == x) {
                                /* if coordinates are the same */
                                i -= 1;
                                horizontal = !horizontal;
                                if (i == 2)
                                    continue;
                            } else {
                                if (px > x && is_like_space(x, y, MV_RIGHT)) {
                                    move(x, y, MV_RIGHT, O_CHASING_STONE);
                                    dont_move = true;
                                    break;
                                } else if (px < x && is_like_space(x, y, MV_LEFT)) {
                                    move(x, y, MV_LEFT, O_CHASING_STONE);
                                    dont_move = true;
                                    break;
                                } else {
                                    i -= 2;
                                    if (i == 1) {
                                        horizontal = !horizontal;
                                        continue;
                                    }
                                }
                            }
                        } else {    /********************************/
                            /* check for a vertical movement */
                            if (py == y) {
                                /* if coordinates are the same */
                                i -= 1;
                                horizontal = !horizontal;
                                if (i == 2)
                                    continue;
                            } else {
                                if (py > y && is_like_space(x, y, MV_DOWN)) {
                                    move(x, y, MV_DOWN, O_CHASING_STONE);
                                    dont_move = true;
                                    break;
                                } else if (py < y && is_like_space(x, y, MV_UP)) {
                                    move(x, y, MV_UP, O_CHASING_STONE);
                                    dont_move = true;
                                    break;
                                } else {
                                    i -= 2;
                                    if (i == 1) {
                                        horizontal = !horizontal;
                                        continue;
                                    }
                                }
                            }
                        }
                        if (i != 0)
                            dont_move = true;
                        break;
                    }

                    /* if we should move in both directions, but can not move in any, stop. */
                    if (!dont_move) {
                        if (horizontal) {   /* check for horizontal */
                            if (x >= px) {
                                if (is_like_space(x, y, MV_UP) && is_like_space(x, y, MV_UP_LEFT))
                                    move(x, y, MV_UP, O_CHASING_STONE);
                                else if (is_like_space(x, y, MV_DOWN) && is_like_space(x, y, MV_DOWN_LEFT))
                                    move(x, y, MV_DOWN, O_CHASING_STONE);
                            } else {
                                if (is_like_space(x, y, MV_UP) && is_like_space(x, y, MV_UP_RIGHT))
                                    move(x, y, MV_UP, O_CHASING_STONE);
                                else if (is_like_space(x, y, MV_DOWN) && is_like_space(x, y, MV_DOWN_RIGHT))
                                    move(x, y, MV_DOWN, O_CHASING_STONE);
                            }
                        } else {    /* check for vertical */
                            if (y >= py) {
                                if (is_like_space(x, y, MV_LEFT) && is_like_space(x, y, MV_UP_LEFT))
                                    move(x, y, MV_LEFT, O_CHASING_STONE);
                                else if (is_like_space(x, y, MV_RIGHT) && is_like_space(x, y, MV_UP_RIGHT))
                                    move(x, y, MV_RIGHT, O_CHASING_STONE);
                            } else {
                                if (is_like_space(x, y, MV_LEFT) && is_like_space(x, y, MV_DOWN_LEFT))
                                    move(x, y, MV_LEFT, O_CHASING_STONE);
                                else if (is_like_space(x, y, MV_RIGHT) && is_like_space(x, y, MV_DOWN_RIGHT))
                                    move(x, y, MV_RIGHT, O_CHASING_STONE);
                            }
                        }
                    }
                }
                break;

                case O_REPLICATOR:
                    if (replicators_wait_frame == 0 && replicators_active && !gravity_disabled) {
                        /* only replicate, if space is under it. */
                        /* do not replicate players! */
                        /* also obeys gravity settings. */
                        /* only replicate element if it is not a scanned one */
                        if (is_like_space(x, y, gravity) && !is_player(x, y, opposite[gravity]) && !is_scanned(x, y, opposite[gravity])) {
                            store(x, y, gravity, get(x, y, opposite[gravity]));
                            sound_play(GD_S_REPLICATOR, x, y);
                        }
                    }
                    break;

                case O_BITER_1:
                case O_BITER_2:
                case O_BITER_3:
                case O_BITER_4:
                    if (biters_wait_frame == 0) {
                        static GdDirectionEnum biter_move[] = { MV_UP, MV_RIGHT, MV_DOWN, MV_LEFT };
                        int dir = get(x, y) - O_BITER_1; /* direction, last two bits 0..3 */
                        int dirn = (dir + 3) & 3;
                        int dirp = (dir + 1) & 3;
                        GdElementEnum made_sound_of = O_NONE;

                        unsigned int i;
                        for (i = 0; i < G_N_ELEMENTS(biter_try); i++) {
                            if (is_like_element(x, y, biter_move[dir], biter_try[i])) {
                                GdDirectionEnum mdir = biter_move[dir];
                                play_eat_particle_of_element(get(x, y, mdir), x + gd_dx[mdir], y + gd_dy[mdir]);
                                move(x, y, mdir, GdElementEnum(O_BITER_1 + dir));
                                if (biter_try[i] != O_SPACE)
                                    made_sound_of = O_BITER_1;  /* sound of a biter eating */
                                break;
                            } else if (is_like_element(x, y, biter_move[dirn], biter_try[i])) {
                                GdDirectionEnum mdir = biter_move[dirn];
                                play_eat_particle_of_element(get(x, y, mdir), x + gd_dx[mdir], y + gd_dy[mdir]);
                                move(x, y, mdir, GdElementEnum(O_BITER_1 + dirn));
                                if (biter_try[i] != O_SPACE)
                                    made_sound_of = O_BITER_1;  /* sound of a biter eating */
                                break;
                            } else if (is_like_element(x, y, biter_move[dirp], biter_try[i])) {
                                GdDirectionEnum mdir = biter_move[dirp];
                                play_eat_particle_of_element(get(x, y, mdir), x + gd_dx[mdir], y + gd_dy[mdir]);
                                move(x, y, mdir, GdElementEnum(O_BITER_1 + dirp));
                                if (biter_try[i] != O_SPACE)
                                    made_sound_of = O_BITER_1;  /* sound of a biter eating */
                                break;
                            }
                        }
                        if (i == G_N_ELEMENTS(biter_try)) {
                            /* i=number of elements in array: could not move, so just turn */
                            store(x, y, GdElementEnum(O_BITER_1 + dirp));
                        } else if (biter_try[i] == O_STONE) {
                            /* if there was a stone there, where we moved... do not eat stones, just throw them back */
                            store(x, y, O_STONE);
                            made_sound_of = O_STONE;
                        }

                        /* if biter did move, we had sound. play it. */
                        if (made_sound_of != O_NONE)
                            play_effect_of_element(made_sound_of, x, y);
                    }
                    break;

                case O_DRAGONFLY_1:
                case O_DRAGONFLY_2:
                case O_DRAGONFLY_3:
                case O_DRAGONFLY_4:
                    /* check if touches a voodoo */
                    if (get(x, y, MV_LEFT) == O_VOODOO || get(x, y, MV_RIGHT) == O_VOODOO || get(x, y, MV_UP) == O_VOODOO || get(x, y, MV_DOWN) == O_VOODOO)
                        voodoo_touched = true;
                    /* check if touches something bad and should explode (includes voodoo by the flags) */
                    if (blows_up_flies(x, y, MV_DOWN) || blows_up_flies(x, y, MV_UP)
                            || blows_up_flies(x, y, MV_LEFT) || blows_up_flies(x, y, MV_RIGHT))
                        explode(x, y);
                    /* otherwise move */
                    else {
                        GdDirection const *creature_move;
                        bool ccw = rotates_ccw(x, y); /* check if default is counterclockwise */
                        GdElementEnum base = O_DRAGONFLY_1; /* base element number (which is like O_***_1) */
                        int dir, dirn;  /* direction */

                        dir = get(x, y) - base; /* facing where */
                        creature_move = creatures_backwards ? creature_chdir : creature_dir;

                        /* now change direction if backwards */
                        if (creatures_backwards)
                            ccw = !ccw;

                        if (ccw)
                            dirn = (dir + 3) & 3; /* fast turn */
                        else
                            dirn = (dir + 1) & 3; /* fast turn */

                        /* if can move forward, does so. */
                        if (is_like_space(x, y, creature_move[dir]))
                            move(x, y, creature_move[dir], GdElementEnum(base + dir));
                        else
                            /* otherwise turns 90 degrees in place. */
                            store(x, y, GdElementEnum(base + dirn));
                    }
                    break;


                case O_BLADDER:
                    store(x, y, O_BLADDER_1);
                    break;

                case O_BLADDER_1:
                case O_BLADDER_2:
                case O_BLADDER_3:
                case O_BLADDER_4:
                case O_BLADDER_5:
                case O_BLADDER_6:
                case O_BLADDER_7:
                case O_BLADDER_8:
                    /* bladder with any delay state: try to convert to clock. */
                    if (is_like_element(x, y, opposite[grav_compat], bladder_converts_by)
                            || is_like_element(x, y, cw_fourth[grav_compat], bladder_converts_by)
                            || is_like_element(x, y, ccw_fourth[grav_compat], bladder_converts_by)) {
                        /* if touches the specified element, let it be a clock */
                        store(x, y, O_PRE_CLOCK_1);
                        play_effect_of_element(O_PRE_CLOCK_1, x, y);   /* plays the bladder convert sound */
                    } else {
                        /* is space over the bladder? */
                        if (is_like_space(x, y, opposite[grav_compat])) {
                            if (get(x, y) == O_BLADDER_8) {
                                /* if it is a bladder 8, really move up */
                                move(x, y, opposite[grav_compat], O_BLADDER_1);
                                play_effect_of_element(O_BLADDER, x, y);
                            } else
                                /* if smaller delay, just increase delay. */
                                next(x, y);
                        } else
                            /* if not space, is something sloped over the bladder? */
                            if (sloped_for_bladder(x, y, opposite[grav_compat]) && sloped(x, y, opposite[grav_compat], opposite[grav_compat])) {
                                if (sloped(x, y, opposite[grav_compat], ccw_fourth[opposite[grav_compat]])
                                        && is_like_space(x, y, ccw_fourth[opposite[grav_compat]])
                                        && is_like_space(x, y, ccw_eighth[opposite[grav_compat]])) {
                                    /* rolling up, to left */
                                    if (get(x, y) == O_BLADDER_8) {
                                        /* if it is a bladder 8, really roll */
                                        move(x, y, ccw_fourth[opposite[grav_compat]], O_BLADDER_8);
                                        play_effect_of_element(O_BLADDER, x, y);
                                    } else
                                        /* if smaller delay, just increase delay. */
                                        next(x, y);
                                } else if (sloped(x, y, opposite[grav_compat], cw_fourth[opposite[grav_compat]])
                                           && is_like_space(x, y, cw_fourth[opposite[grav_compat]])
                                           && is_like_space(x, y, cw_eighth[opposite[grav_compat]])) {
                                    /* rolling up, to left */
                                    if (get(x, y) == O_BLADDER_8) {
                                        /* if it is a bladder 8, really roll */
                                        move(x, y, cw_fourth[opposite[grav_compat]], O_BLADDER_8);
                                        play_effect_of_element(O_BLADDER, x, y);
                                    } else
                                        /* if smaller delay, just increase delay. */
                                        next(x, y);
                                }
                            }
                        /* no space, no sloped thing over it - store bladder 1 and that is for now. */
                            else
                                store(x, y, O_BLADDER_1);
                    }
                    break;

                case O_GHOST:
                    if (blows_up_flies(x, y, MV_DOWN) || blows_up_flies(x, y, MV_UP)
                            || blows_up_flies(x, y, MV_LEFT) || blows_up_flies(x, y, MV_RIGHT))
                        explode(x, y);
                    else {
                        int i;

                        /* the ghost is given four possibilities to move. */
                        for (i = 0; i < 4; i++) {
                            static GdDirectionEnum dirs[] = {MV_UP, MV_DOWN, MV_LEFT, MV_RIGHT};
                            GdDirectionEnum random_dir;

                            random_dir = dirs[random.rand_int_range(0, G_N_ELEMENTS(dirs))];
                            if (is_like_space(x, y, random_dir)) {
                                move(x, y, random_dir, O_GHOST);
                                break;  /* ghost did move -> exit loop */
                            }
                        }
                    }
                    break;


                    /*
                     *  A C T I V E    E L E M E N T S
                     */

                case O_AMOEBA:
                    if (hatched && amoeba_state == GD_AM_AWAKE)
                        play_effect_of_element(O_AMOEBA, x, y);
                    /* emulating BD1 amoeba+magic wall bug */
                    if (convert_amoeba_this_frame && amoeba_found_enclosed) {
                        store(x, y, amoeba_enclosed_effect);
                        break;
                    }
                    amoeba_count++;
                    switch (amoeba_state) {
                        case GD_AM_TOO_BIG:
                            store(x, y, amoeba_too_big_effect);
                            break;
                        case GD_AM_ENCLOSED:
                            store(x, y, amoeba_enclosed_effect);
                            break;
                        case GD_AM_SLEEPING:
                        case GD_AM_AWAKE:
                            /* if no amoeba found during THIS SCAN yet, which was able to grow, check this one. */
                            if (amoeba_found_enclosed)
                                /* if still found enclosed, check all four directions, if this one is able to grow. */
                                if (amoeba_eats(x, y, MV_UP) || amoeba_eats(x, y, MV_DOWN)
                                        || amoeba_eats(x, y, MV_LEFT) || amoeba_eats(x, y, MV_RIGHT)) {
                                    amoeba_found_enclosed = false;  /* not enclosed. this is a local (per scan) flag! */
                                    amoeba_state = GD_AM_AWAKE;
                                }

                            /* if alive, check in which dir to grow (or not) */
                            if (amoeba_state == GD_AM_AWAKE) {
                                if (random.rand_int_range(0, 1000000) < amoeba_growth_prob) {
                                    switch (random.rand_int_range(0, 4)) {  /* decided to grow, choose a random direction. */
                                        case 0: /* let this be up. numbers indifferent. */
                                            if (amoeba_eats(x, y, MV_UP))
                                                store(x, y, MV_UP, O_AMOEBA);
                                            break;
                                        case 1: /* down */
                                            if (amoeba_eats(x, y, MV_DOWN))
                                                store(x, y, MV_DOWN, O_AMOEBA);
                                            break;
                                        case 2: /* left */
                                            if (amoeba_eats(x, y, MV_LEFT))
                                                store(x, y, MV_LEFT, O_AMOEBA);
                                            break;
                                        case 3: /* right */
                                            if (amoeba_eats(x, y, MV_RIGHT))
                                                store(x, y, MV_RIGHT, O_AMOEBA);
                                            break;
                                    }
                                }
                            }
                            break;
                    }
                    break;

                case O_AMOEBA_2:
                    if (hatched && amoeba_2_state == GD_AM_AWAKE)
                        play_effect_of_element(O_AMOEBA, x, y);
                    amoeba_2_count++;
                    /* check if it is touching an amoeba, and explosion is enabled */
                    if (amoeba_2_explodes_by_amoeba
                            && (is_like_element(x, y, MV_DOWN, O_AMOEBA) || is_like_element(x, y, MV_UP, O_AMOEBA)
                                || is_like_element(x, y, MV_LEFT, O_AMOEBA) || is_like_element(x, y, MV_RIGHT, O_AMOEBA)))
                        explode(x, y);
                    else
                        switch (amoeba_2_state) {
                            case GD_AM_TOO_BIG:
                                store(x, y, amoeba_2_too_big_effect);
                                break;
                            case GD_AM_ENCLOSED:
                                store(x, y, amoeba_2_enclosed_effect);
                                break;
                            case GD_AM_SLEEPING:
                            case GD_AM_AWAKE:
                                /* if no amoeba found during THIS SCAN yet, which was able to grow, check this one. */
                                if (amoeba_2_found_enclosed)
                                    if (amoeba_eats(x, y, MV_UP) || amoeba_eats(x, y, MV_DOWN)
                                            || amoeba_eats(x, y, MV_LEFT) || amoeba_eats(x, y, MV_RIGHT)) {
                                        amoeba_2_found_enclosed = false; /* not enclosed. this is a local (per scan) flag! */
                                        amoeba_2_state = GD_AM_AWAKE;
                                    }

                                if (amoeba_2_state == GD_AM_AWAKE)  /* if it is alive, decide if it attempts to grow */
                                    if (random.rand_int_range(0, 1000000) < amoeba_2_growth_prob) {
                                        switch (random.rand_int_range(0, 4)) {  /* decided to grow, choose a random direction. */
                                            case 0: /* let this be up. numbers indifferent. */
                                                if (amoeba_eats(x, y, MV_UP))
                                                    store(x, y, MV_UP, O_AMOEBA_2);
                                                break;
                                            case 1: /* down */
                                                if (amoeba_eats(x, y, MV_DOWN))
                                                    store(x, y, MV_DOWN, O_AMOEBA_2);
                                                break;
                                            case 2: /* left */
                                                if (amoeba_eats(x, y, MV_LEFT))
                                                    store(x, y, MV_LEFT, O_AMOEBA_2);
                                                break;
                                            case 3: /* right */
                                                if (amoeba_eats(x, y, MV_RIGHT))
                                                    store(x, y, MV_RIGHT, O_AMOEBA_2);
                                                break;
                                        }
                                    }
                                break;
                        }
                    break;

                case O_ACID:
                    /* choose randomly, if it spreads */
                    if (random.rand_int_range(0, 1000000) <= acid_spread_ratio) {
                        /* the current one explodes */
                        store(x, y, acid_turns_to);
                        /* and if neighbours are eaten, put acid there. */
                        if (is_like_element(x, y, MV_UP, acid_eats_this)) {
                            store(x, y, MV_UP, O_ACID);
                            play_effect_of_element(O_ACID, x, y);
                        }
                        if (is_like_element(x, y, MV_DOWN, acid_eats_this)) {
                            store(x, y, MV_DOWN, O_ACID);
                            play_effect_of_element(O_ACID, x, y);
                        }
                        if (is_like_element(x, y, MV_LEFT, acid_eats_this)) {
                            store(x, y, MV_LEFT, O_ACID);
                            play_effect_of_element(O_ACID, x, y);
                        }
                        if (is_like_element(x, y, MV_RIGHT, acid_eats_this)) {
                            store(x, y, MV_RIGHT, O_ACID);
                            play_effect_of_element(O_ACID, x, y);
                        }
                    }
                    break;

                case O_WATER:
                    if (!water_does_not_flow_down && is_like_space(x, y, MV_DOWN))   /* emulating the odd behaviour in crdr */
                        store(x, y, MV_DOWN, O_WATER_1);
                    if (is_like_space(x, y, MV_UP))
                        store(x, y, MV_UP, O_WATER_1);
                    if (is_like_space(x, y, MV_LEFT))
                        store(x, y, MV_LEFT, O_WATER_1);
                    if (is_like_space(x, y, MV_RIGHT))
                        store(x, y, MV_RIGHT, O_WATER_1);
                    break;

                case O_WATER_16:
                    store(x, y, O_WATER);
                    break;

                case O_H_EXPANDING_WALL:
                case O_V_EXPANDING_WALL:
                case O_H_EXPANDING_STEEL_WALL:
                case O_V_EXPANDING_STEEL_WALL:
                    /* checks first if direction is changed. */
                    if (((get(x, y) == O_H_EXPANDING_WALL || get(x, y) == O_H_EXPANDING_STEEL_WALL) && !expanding_wall_changed)
                            || ((get(x, y) == O_V_EXPANDING_WALL || get(x, y) == O_V_EXPANDING_STEEL_WALL) && expanding_wall_changed)) {
                        if (is_like_space(x, y, MV_LEFT)) {
                            store(x, y, MV_LEFT, get(x, y));
                            play_effect_of_element(get(x, y), x, y, MV_LEFT);
                        } else if (is_like_space(x, y, MV_RIGHT)) {
                            store(x, y, MV_RIGHT, get(x, y));
                            play_effect_of_element(get(x, y), x, y, MV_RIGHT);
                        }
                    } else {
                        if (is_like_space(x, y, MV_UP)) {
                            store(x, y, MV_UP, get(x, y));
                            play_effect_of_element(get(x, y), x, y, MV_UP);
                        } else if (is_like_space(x, y, MV_DOWN)) {
                            store(x, y, MV_DOWN, get(x, y));
                            play_effect_of_element(get(x, y), x, y, MV_DOWN);
                        }
                    }
                    break;

                case O_EXPANDING_WALL:
                case O_EXPANDING_STEEL_WALL:
                    /* the wall which grows in all four directions. */
                    if (is_like_space(x, y, MV_LEFT)) {
                        store(x, y, MV_LEFT, get(x, y));
                        play_effect_of_element(get(x, y), x, y, MV_LEFT);
                    }
                    if (is_like_space(x, y, MV_RIGHT)) {
                        store(x, y, MV_RIGHT, get(x, y));
                        play_effect_of_element(get(x, y), x, y, MV_RIGHT);
                    }
                    if (is_like_space(x, y, MV_UP)) {
                        store(x, y, MV_UP, get(x, y));
                        play_effect_of_element(get(x, y), x, y, MV_UP);
                    }
                    if (is_like_space(x, y, MV_DOWN)) {
                        store(x, y, MV_DOWN, get(x, y));
                        play_effect_of_element(get(x, y), x, y, MV_DOWN);
                    }
                    break;

                case O_SLIME:
                    /*
                     * unpredictable: g_rand_int
                     * predictable: c64 predictable random generator.
                     *    for predictable, a random number is generated, whether or not it is even possible that the stone
                     *    will be able to pass.
                     */
                    if (slime_predictable ? ((c64_rand.random()&slime_permeability_c64) == 0) : random.rand_int_range(0, 1000000) < slime_permeability) {
                        GdDirectionEnum grav = gravity;
                        GdDirectionEnum oppos = opposite[gravity];

                        /* space under the slime? elements may pass from top to bottom then. */
                        if (is_like_space(x, y, grav)) {
                            if (get(x, y, oppos) == slime_eats_1) {
                                store(x, y, grav, slime_converts_1);    /* output a falling xy under */
                                store(x, y, oppos, O_SPACE);
                                play_effect_of_element(O_SLIME, x, y);
                            } else if (get(x, y, oppos) == slime_eats_2) {
                                store(x, y, grav, slime_converts_2);
                                store(x, y, oppos, O_SPACE);
                                play_effect_of_element(O_SLIME, x, y);
                            } else if (get(x, y, oppos) == slime_eats_3) {
                                store(x, y, grav, slime_converts_3);
                                store(x, y, oppos, O_SPACE);
                                play_effect_of_element(O_SLIME, x, y);
                            } else if (get(x, y, oppos) == O_WAITING_STONE) { /* waiting stones pass without awakening */
                                store(x, y, grav, O_WAITING_STONE);
                                store(x, y, oppos, O_SPACE);
                                play_effect_of_element(O_SLIME, x, y);
                            } else if (get(x, y, oppos) == O_CHASING_STONE) { /* chasing stones pass */
                                store(x, y, grav, O_CHASING_STONE);
                                store(x, y, oppos, O_SPACE);
                                play_effect_of_element(O_SLIME, x, y);
                            }
                        } else
                            /* or space over the slime? elements may pass from bottom to up then. */
                            if (is_like_space(x, y, oppos)) {
                                if (get(x, y, grav) == O_BLADDER) {                 /* bladders move UP the slime */
                                    store(x, y, grav, O_SPACE);
                                    store(x, y, oppos, O_BLADDER_1);
                                    play_effect_of_element(O_SLIME, x, y);
                                } else if (get(x, y, grav) == O_FLYING_STONE) {
                                    store(x, y, grav, O_SPACE);
                                    store(x, y, oppos, O_FLYING_STONE_F);
                                    play_effect_of_element(O_SLIME, x, y);
                                } else if (get(x, y, grav) == O_FLYING_DIAMOND) {
                                    store(x, y, grav, O_SPACE);
                                    store(x, y, oppos, O_FLYING_DIAMOND_F);
                                    play_effect_of_element(O_SLIME, x, y);
                                }
                            }
                    }
                    break;

                case O_FALLING_WALL:
                    if (is_like_space(x, y, grav_compat)) {
                        /* try falling if space under. */
                        int yy;
                        for (yy = y + 1; yy < y + h; yy++)
                            /* yy<y+h is to check everything OVER the wall - since caves wrap around !! */
                            if (!is_like_space(x, yy))
                                /* stop cycle when other than space */
                                break;
                        /* if scanning stopped by a player... start falling! */
                        if (get(x, yy) == O_PLAYER || get(x, yy) == O_PLAYER_GLUED || get(x, yy) == O_PLAYER_BOMB) {
                            move(x, y, grav_compat, O_FALLING_WALL_F);
                            /* no sound when the falling wall starts falling! */
                        }
                    }
                    break;

                case O_FALLING_WALL_F:
                    if (is_player(x, y, grav_compat)) {
                        /* if player under, it explodes - the falling wall, not the player! */
                        explode(x, y);
                    } else if (is_like_space(x, y, grav_compat)) {
                        /* continue falling */
                        move(x, y, grav_compat, O_FALLING_WALL_F);
                    } else {
                        /* stop falling */
                        play_effect_of_element(O_FALLING_WALL_F, x, y);
                        store(x, y, O_FALLING_WALL);
                    }
                    break;

                    /*
                     * C O N V E Y O R    B E L T S
                     */
                case O_CONVEYOR_RIGHT:
                case O_CONVEYOR_LEFT:
                    /* only works if gravity is up or down!!! */
                    /* first, check for gravity and running belts. */
                    if (!gravity_disabled && conveyor_belts_active) {
                        GdDirection const *dir;
                        bool left;

                        /* decide direction */
                        left = get(x, y) != O_CONVEYOR_RIGHT;
                        if (conveyor_belts_direction_changed)
                            left = !left;
                        dir = left ? ccw_eighth : cw_eighth;

                        /* CHECK IF IT CONVEYS THE ELEMENT ABOVE IT */
                        /* if gravity is normal, and the conveyor belt has something ABOVE which can be moved
                            OR
                           the gravity is up, so anything that should float now goes DOWN and touches the conveyor */
                        if ((gravity == MV_DOWN && moved_by_conveyor_top(x, y, MV_UP))
                                || (gravity == MV_UP && moved_by_conveyor_bottom(x, y, MV_UP))) {
                            if (is_like_space(x, y, dir[MV_UP])) {
                                store(x, y, dir[MV_UP], get(x, y, MV_UP));  /* move */
                                store(x, y, MV_UP, O_SPACE);    /* and place a space. */
                            }
                        }
                        /* CHECK IF IT CONVEYS THE ELEMENT BELOW IT */
                        if ((gravity == MV_UP && moved_by_conveyor_top(x, y, MV_DOWN))
                                || (gravity == MV_DOWN && moved_by_conveyor_bottom(x, y, MV_DOWN))) {
                            if (is_like_space(x, y, dir[MV_DOWN])) {
                                store(x, y, dir[MV_DOWN], get(x, y, MV_DOWN));  /* move */
                                store(x, y, MV_DOWN, O_SPACE);  /* and clear. */
                            }
                        }
                    }
                    break;

                case O_ROCKET_1:
                    if (is_like_space(x, y, MV_RIGHT)) {
                        move(x, y, MV_RIGHT, O_ROCKET_1);
                        add_particle_set(x, y, O_ROCKET_1);
                    } else
                        explode(x, y);
                    break;
                case O_ROCKET_2:
                    if (is_like_space(x, y, MV_UP)) {
                        move(x, y, MV_UP, O_ROCKET_2);
                        add_particle_set(x, y, O_ROCKET_2);
                    } else
                        explode(x, y);
                    break;
                case O_ROCKET_3:
                    if (is_like_space(x, y, MV_LEFT)) {
                        move(x, y, MV_LEFT, O_ROCKET_3);
                        add_particle_set(x, y, O_ROCKET_3);
                    } else
                        explode(x, y);
                    break;
                case O_ROCKET_4:
                    if (is_like_space(x, y, MV_DOWN)) {
                        move(x, y, MV_DOWN, O_ROCKET_4);
                        add_particle_set(x, y, O_ROCKET_4);
                    } else
                        explode(x, y);
                    break;

                    /*
                     * S I M P L E   C H A N G I N G;   E X P L O S I O N S
                     */
                case O_EXPLODE_3:
                    store(x, y, explosion_3_effect);
                    break;
                case O_EXPLODE_5:
                    store(x, y, explosion_effect);
                    break;
                case O_NUT_CRACK_4:
                    store(x, y, O_DIAMOND);
                    break;
                case O_PRE_DIA_5:
                    store(x, y, diamond_birth_effect);
                    break;
                case O_PRE_STONE_4:
                    store(x, y, O_STONE);
                    break;

                case O_NITRO_EXPL_4:
                    store(x, y, nitro_explosion_effect);
                    break;
                case O_BOMB_EXPL_4:
                    store(x, y, bomb_explosion_effect);
                    break;
                case O_AMOEBA_2_EXPL_4:
                    store(x, y, amoeba_2_explosion_effect);
                    break;

                case O_GHOST_EXPL_4: {
                    static GdElementEnum ghost_explode[] = {
                        O_SPACE, O_SPACE, O_DIRT, O_DIRT, O_CLOCK, O_CLOCK, O_PRE_OUTBOX,
                        O_BOMB, O_BOMB, O_PLAYER, O_GHOST, O_BLADDER, O_DIAMOND, O_SWEET,
                        O_WAITING_STONE, O_BITER_1
                    };

                    store(x, y, ghost_explode[random.rand_int_range(0, G_N_ELEMENTS(ghost_explode))]);
                }
                break;
                case O_PRE_STEEL_4:
                    store(x, y, O_STEEL);
                    break;
                case O_PRE_CLOCK_4:
                    store(x, y, O_CLOCK);
                    break;
                case O_BOMB_TICK_7:
                    explode(x, y);
                    break;

                case O_TRAPPED_DIAMOND:
                    if (diamond_key_collected)
                        store(x, y, O_DIAMOND);
                    break;

                case O_PRE_OUTBOX:
                    if (gate_open) /* if no more diamonds needed */
                        store(x, y, O_OUTBOX);  /* open outbox */
                    break;
                case O_PRE_INVIS_OUTBOX:
                    if (gate_open)  /* if no more diamonds needed */
                        store(x, y, O_INVIS_OUTBOX);    /* open outbox. invisible one :P */
                    break;
                case O_INBOX:
                    player_seen_ago = 0;
                    if (hatched && !inbox_toggle)   /* if it is time of birth */
                        store(x, y, O_PRE_PL_1);
                    inbox_toggle = !inbox_toggle;
                    break;
                case O_PRE_PL_1:
                    player_seen_ago = 0;
                    store(x, y, O_PRE_PL_2);
                    break;
                case O_PRE_PL_2:
                    player_seen_ago = 0;
                    store(x, y, O_PRE_PL_3);
                    break;
                case O_PRE_PL_3:
                    player_seen_ago = 0;
                    store(x, y, O_PLAYER);
                    break;

                case O_PRE_DIA_1:
                case O_PRE_DIA_2:
                case O_PRE_DIA_3:
                case O_PRE_DIA_4:
                case O_PRE_STONE_1:
                case O_PRE_STONE_2:
                case O_PRE_STONE_3:
                case O_BOMB_TICK_1:
                case O_BOMB_TICK_2:
                case O_BOMB_TICK_3:
                case O_BOMB_TICK_4:
                case O_BOMB_TICK_5:
                case O_BOMB_TICK_6:
                case O_PRE_STEEL_1:
                case O_PRE_STEEL_2:
                case O_PRE_STEEL_3:
                case O_BOMB_EXPL_1:
                case O_BOMB_EXPL_2:
                case O_BOMB_EXPL_3:
                case O_NUT_CRACK_1:
                case O_NUT_CRACK_2:
                case O_NUT_CRACK_3:
                case O_GHOST_EXPL_1:
                case O_GHOST_EXPL_2:
                case O_GHOST_EXPL_3:
                case O_EXPLODE_1:
                case O_EXPLODE_2:
                    /* explode 3 is 'effected' */
                case O_EXPLODE_4:
                case O_PRE_CLOCK_1:
                case O_PRE_CLOCK_2:
                case O_PRE_CLOCK_3:
                case O_NITRO_EXPL_1:
                case O_NITRO_EXPL_2:
                case O_NITRO_EXPL_3:
                case O_AMOEBA_2_EXPL_1:
                case O_AMOEBA_2_EXPL_2:
                case O_AMOEBA_2_EXPL_3:
                    /* simply the next identifier */
                    next(x, y);
                    break;
                case O_WATER_1:
                case O_WATER_2:
                case O_WATER_3:
                case O_WATER_4:
                case O_WATER_5:
                case O_WATER_6:
                case O_WATER_7:
                case O_WATER_8:
                case O_WATER_9:
                case O_WATER_10:
                case O_WATER_11:
                case O_WATER_12:
                case O_WATER_13:
                case O_WATER_14:
                case O_WATER_15:
                    sound_play(GD_S_WATER, x, y);
                    /* simply advance the cell the next identifier */
                    next(x, y);
                    break;

                case O_BLADDER_SPENDER:
                    if (is_like_space(x, y, opposite[grav_compat])) {
                        store(x, y, opposite[grav_compat], O_BLADDER);
                        store(x, y, O_PRE_STEEL_1);
                        play_effect_of_element(O_BLADDER_SPENDER, x, y);
                    }
                    break;

                case O_MAGIC_WALL:
                    /* the magic wall effects are handled by the elements which fall
                     * onto the wall. here only the sound is handled. */
                    if (magic_wall_state == GD_MW_ACTIVE) {
                        play_effect_of_element(O_MAGIC_WALL, x, y);
                    }
                    break;

                case O_LAVA:
                    /* lava is handled by the store() routine. here only the visual effect is added. */
                    add_particle_set(x, y, O_LAVA);
                    break;

                default:
                    /* other inanimate elements that do nothing */
                    break;
            }

            /* after processing, check the current coordinate, if it became scanned. */
            /* the scanned bit can be cleared, as it will not be processed again. */
            /* and, it must be cleared, as it should not be scanned; for example, */
            /* if it is, a replicator will not replicate it! */
            unscan(x, y);
        }

    /* POSTPROCESSING */

    /* forget "scanned" flags for objects. */
    /* also, check for time penalties. */
    /* these is something like an effect table, but we do not really use one. */
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            unscan(x, y);
            if (get(x, y) == O_TIME_PENALTY) {
                store(x, y, O_GRAVESTONE);
                time_decrement_sec += time_penalty; /* there is time penalty for destroying the voodoo */
            }
        }

    /* another scan-like routine: */
    /* short explosions (for example, in bd1) started with explode_2. */
    /* internally we use explode_1; and change it to explode_2 if needed. */
    if (short_explosions)
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                if (is_first_stage_of_explosion(x, y)) {
                    next(x, y); /* select next frame of explosion */
                    unscan(x, y); /* forget scanned flag immediately */
                }

    /* this loop finds the coordinates of the player. needed for scrolling and chasing stone.*/
    /* but we only do this, if a living player was found. otherwise "stay" at current coordinates. */
    if (player_state == GD_PL_LIVING) {
        if (active_is_first_found) {
            /* to be 1stb compatible, we do everything backwards. */
            for (int y = h - 1; y >= 0; y--)
                for (int x = w - 1; x >= 0; x--)
                    if (is_player(x, y)) {
                        /* here we remember the coordinates. */
                        player_x = x;
                        player_y = y;
                    }
        } else {
            /* as in the original: look for the last one */
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                    if (is_player(x, y)) {
                        /* here we remember the coordinates. */
                        player_x = x;
                        player_y = y;
                    }
        }
    }
    /* record coordinates of player for chasing stone */
    for (unsigned i = 0; i < PlayerMemSize - 1; i++) {
        player_x_mem[i] = player_x_mem[i + 1];
        player_y_mem[i] = player_y_mem[i + 1];
    }
    player_x_mem[PlayerMemSize - 1] = player_x;
    player_y_mem[PlayerMemSize - 1] = player_y;

    /* SCHEDULING */
    /* updates based on the calculated explosions and per element ckdelays. */
    update_scheduling();

    /* CAVE VARIABLES */

    /* PLAYER */
    if ((player_state == GD_PL_LIVING && player_seen_ago > 15) || kill_player) /* check if player is alive. */
        player_state = GD_PL_DIED;
    if (voodoo_touched) /* check if any voodoo exploded, and kill players the next scan if that happended. */
        kill_player = true;

    /* AMOEBA */
    if (amoeba_state == GD_AM_AWAKE) {
        /* check flags after evaluating. */
        if (amoeba_count >= amoeba_max_count)
            amoeba_state = GD_AM_TOO_BIG;
        if (amoeba_found_enclosed)
            amoeba_state = GD_AM_ENCLOSED;
    }
    /* amoeba can also be turned into diamond by magic wall */
    if (magic_wall_stops_amoeba && magic_wall_state == GD_MW_ACTIVE)
        amoeba_state = GD_AM_ENCLOSED;
    /* AMOEBA 2 */
    if (amoeba_2_state == GD_AM_AWAKE) {
        /* check flags after evaluating. */
        if (amoeba_2_count >= amoeba_2_max_count)
            amoeba_2_state = GD_AM_TOO_BIG;
        if (amoeba_2_found_enclosed)
            amoeba_2_state = GD_AM_ENCLOSED;
    }
    /* amoeba 2 can also be turned into diamond by magic wall */
    if (magic_wall_stops_amoeba && magic_wall_state == GD_MW_ACTIVE)
        amoeba_2_state = GD_AM_ENCLOSED;


    /* now check times. --------------------------- */
    /* decrement time if a voodoo was killed. */
    time -= time_decrement_sec * timing_factor;
    if (time < 0)
        time = 0;

    /* only decrement time when player is already born. */
    if (hatched) {
        int secondsbefore = time / timing_factor;
        time -= speed;
        if (time <= 0)
            time = 0;
        int secondsafter = time / timing_factor;
        if (secondsbefore != secondsafter)
            set_seconds_sound();
        
        /* for statistics */
        time_elapsed += speed;
    }
    /* a gravity switch was activated; seconds counting down */
    if (gravity_will_change > 0) {
        gravity_will_change -= speed;
        if (gravity_will_change < 0)
            gravity_will_change = 0;

        if (gravity_will_change == 0) {
            gravity = gravity_next_direction;
            if (gravity_change_sound)
                sound_play(GD_S_GRAVITY_CHANGE, player_x, player_y);    /* takes precedence over amoeba and magic wall sound */
        }
    }

    /* creatures direction automatically change */
    if (creatures_direction_will_change > 0) {
        creatures_direction_will_change -= speed;
        if (creatures_direction_will_change < 0)
            creatures_direction_will_change = 0;

        if (creatures_direction_will_change == 0) {
            if (creature_direction_auto_change_sound)
                sound_play(GD_S_SWITCH_CREATURES, player_x, player_y);
            creatures_backwards = !creatures_backwards;
            creatures_direction_will_change = creatures_direction_auto_change_time * timing_factor;
        }
    }

    /* magic wall; if active&wait or not wait for hatching */
    if (magic_wall_state == GD_MW_ACTIVE && (hatched || !magic_timer_wait_for_hatching)
            && !(magic_wall_time == 0 && magic_timer_zero_is_infinite)) {
        magic_wall_time -= speed;
        if (magic_wall_time < 0)
            magic_wall_time = 0;
        if (magic_wall_time == 0)
            magic_wall_state = GD_MW_EXPIRED;
    }
    /* we may wait for hatching, when starting amoeba */
    if (amoeba_timer_started_immediately || (amoeba_state == GD_AM_AWAKE && (hatched || !amoeba_timer_wait_for_hatching))) {
        amoeba_time -= speed;
        if (amoeba_time < 0)
            amoeba_time = 0;
        if (amoeba_time == 0)
            amoeba_growth_prob = amoeba_fast_growth_prob;
    }
    /* we may wait for hatching, when starting amoeba */
    if (amoeba_timer_started_immediately || (amoeba_2_state == GD_AM_AWAKE && (hatched || !amoeba_timer_wait_for_hatching))) {
        amoeba_2_time -= speed;
        if (amoeba_2_time < 0)
            amoeba_2_time = 0;
        if (amoeba_2_time == 0)
            amoeba_2_growth_prob = amoeba_2_fast_growth_prob;
    }

    /* check for player hatching. */
    start_signal = false;
    /* if not the c64 scheduling, but the correct frametime is used, hatching delay should always be decremented. */
    /* otherwise, the if (millisecs...) condition below will set this. */
    if (scheduling == GD_SCHEDULING_MILLISECONDS) {     /* NON-C64 scheduling */
        if (hatching_delay_frame > 0) {
            --hatching_delay_frame; /* for milliseconds-based, non-c64 schedulings, hatching delay means frames. */
            if (hatching_delay_frame == 0)
                start_signal = true;
        }
    } else {                            /* C64 scheduling */
        if (hatching_delay_time > 0) {
            hatching_delay_time -= speed; /* for c64 schedulings, hatching delay means milliseconds. */
            if (hatching_delay_time <= 0) {
                hatching_delay_time = 0;
                start_signal = true;
            }
        }
    }

    /* if decremented hatching, and it became zero: */
    if (start_signal) {     /* THIS IS THE CAVE START SIGNAL */
        hatched = true; /* record that now the cave is in its normal state */

        count_diamonds();   /* if diamonds needed is below zero, we count the available diamonds now. */

        /* setup direction auto change */
        if (creatures_direction_auto_change_time) {
            creatures_direction_will_change = creatures_direction_auto_change_time * timing_factor;

            if (creatures_direction_auto_change_on_start)
                creatures_backwards = !creatures_backwards;
        }

        if (player_state == GD_PL_NOT_YET)
            player_state = GD_PL_LIVING;

        sound_play(GD_S_CRACK, player_x, player_y);
    }

    /* for biters */
    if (biters_wait_frame == 0)
        biters_wait_frame = biter_delay_frame;
    else
        --biters_wait_frame;
    /* replicators delay */
    if (replicators_wait_frame == 0)
        replicators_wait_frame = replicator_delay_frame;
    else
        --replicators_wait_frame;


    /* LAST THOUGTS */

    /* check if cave failed by timeout */
    if (player_state != GD_PL_TIMEOUT && time == 0) {
        player_state = GD_PL_TIMEOUT;
        clear_sounds();
        sound_play(GD_S_TIMEOUT, player_x, player_y);
    }

    /* set these for drawing. */
    last_direction = player_move;
    /* here we remember last movements for animation. this is needed here, as animation
       is in sync with the game, not the keyboard directly. (for example, after exiting
       the cave, the player was "running" in the original, till bonus points were counted
       for remaining time and so on. */
    if (player_move == MV_LEFT || player_move == MV_UP_LEFT || player_move == MV_DOWN_LEFT)
        last_horizontal_direction = MV_LEFT;
    if (player_move == MV_RIGHT || player_move == MV_UP_RIGHT || player_move == MV_DOWN_RIGHT)
        last_horizontal_direction = MV_RIGHT;

    // return direction of movement of player, which might be changed if no diagonal movements.
    return player_move;
}
