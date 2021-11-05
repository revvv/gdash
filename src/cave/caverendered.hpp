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

#ifndef CAVERENDERED_HPP_INCLUDED
#define CAVERENDERED_HPP_INCLUDED

#include "config.h"

#include <glib.h>
#include <list>

#include "cave/cavebase.hpp"
#include "cave/helper/caverandom.hpp"
#include "cave/helper/cavesound.hpp"
#include "cave/helper/cavemap.hpp"
#include "cave/particle.hpp"

class CaveStored;
class CaveObject;

/// These are states of the magic wall.
/// @todo ezt nem kéne plainolddatába rakni?
enum MagicWallState {
    GD_MW_DORMANT,                  ///< Starting with this.
    GD_MW_ACTIVE,                   ///< Boulder or diamond dropped into.
    GD_MW_EXPIRED                   ///< Turned off after magic_wall_milling_time.
};

/// These are states of the player.
enum PlayerState {
    GD_PL_NOT_YET,              ///< Not yet living. Beginning of cave time.
    GD_PL_LIVING,               ///< Ok.
    GD_PL_TIMEOUT,              ///< Time is up
    GD_PL_DIED,                 ///< Died.
    GD_PL_EXITED                ///< Exited the cave, proceed to next one
};

/// States of amoeba
enum AmoebaState {
    GD_AM_SLEEPING,     ///< sleeping - not yet let out.
    GD_AM_AWAKE,        ///< living, growing
    GD_AM_TOO_BIG,      ///< grown too big, will convert to stones
    GD_AM_ENCLOSED,     ///< enclosed, will convert to diamonds
};

/// For signalling a cell to be redrawn in the graphics map.
enum { GD_REDRAW = 1 << 10 };


/// @ingroup Cave
class CaveRendered : public CaveBase {
private:
    void add_particle_set(int x, int y, GdElementEnum particletype);
    void play_effect_of_element(GdElementEnum element, int x, int y, GdDirectionEnum dir = MV_STILL, bool particles = true);
    void play_eat_sound_of_element(GdElementEnum element, int x, int y);
    void play_eat_particle_of_element(GdElementEnum element, int x, int y);

    GdElementEnum player_eat_element(GdElementEnum element, int x, int y, GdDirectionEnum dir);

    void cell_explode(int x, int y, GdElementEnum explode_to);
    void creature_explode(int x, int y, GdElementEnum explode_to);
    void nitro_explode(int x, int y);
    void voodoo_explode(int x, int y);
    void cell_explode_skip_voodoo(int x, int y, GdElementEnum expl);
    void ghost_explode(int x, int y);
    void bomb_explode(int x, int y);
    void explode(int x, int y);
    void explode(int x, int y, GdDirectionEnum dir);

    bool explodes_by_hit(int x, int y, GdDirectionEnum dir) const;
    bool non_explodable(int x, int y) const;
    bool amoeba_eats(int x, int y, GdDirectionEnum dir) const;
    bool sloped(int x, int y, GdDirectionEnum dir, GdDirectionEnum slop) const;
    bool sloped_for_bladder(int x, int y, GdDirectionEnum dir) const;
    bool blows_up_flies(int x, int y, GdDirectionEnum dir) const;
    bool rotates_ccw(int x, int y) const;
    bool is_player(int x, int y) const;
    bool is_player(int x, int y, GdDirectionEnum dir) const;
    bool can_be_hammered(int x, int y, GdDirectionEnum dir) const;
    bool can_be_pushed(int x, int y, GdDirectionEnum dir) const;
    bool is_first_stage_of_explosion(int x, int y) const;
    bool moved_by_conveyor_top(int x, int y, GdDirectionEnum dir) const;
    bool moved_by_conveyor_bottom(int x, int y, GdDirectionEnum dir) const;
    bool is_scanned(int x, int y) const;
    bool is_scanned(int x, int y, GdDirectionEnum dir) const;
    bool is_like_element(int x, int y, GdDirectionEnum dir, GdElementEnum e) const;
    bool is_like_space(int x, int y, GdDirectionEnum dir = MV_STILL) const;
    bool is_like_dirt(int x, int y, GdDirectionEnum dir = MV_STILL) const;

    GdElementEnum get(int x, int y) const;
    GdElementEnum get(int x, int y, GdDirectionEnum dir) const;
    void store(int x, int y, GdElementEnum element, bool disable_particle = false);
    void store(int x, int y, GdDirectionEnum dir, GdElementEnum element);
    void move(int x, int y, GdDirectionEnum dir, GdElementEnum element);
    void next(int x, int y);
    void unscan(int x, int y);
    
    void update_scheduling();

public:
    CaveRendered(CaveStored const &cave, int level, int seed);
    void create_map(CaveStored const &data, int level);

    void setup_for_game();
    void count_diamonds();
    void set_ckdelay_extra_for_animation();

    /* game playing helpers */
    void draw_indexes(CaveMap<int> &gfx_buffer, CaveMap<bool> const &covered, bool bonus_life_flash, int animcycle, bool hate_invisible_outbox);
    int time_visible(int internal_time) const;
    void set_seconds_sound();
    void sound_play(GdSound sound, int x, int y);
    void clear_sounds();
    GdDirectionEnum iterate(GdDirectionEnum player_move, bool player_fire, bool suicide);
    void store_rc(int x, int y, GdElementEnum element, int order_idx);

    bool do_teleporter(int px, int py, GdDirectionEnum player_move);
    bool do_push(int x, int y, GdDirectionEnum player_move, bool player_fire);

    void do_start_fall(int x, int y, GdDirectionEnum falling_direction, GdElementEnum falling_element);
    bool do_fall_try_crush_voodoo(int x, int y, GdDirectionEnum fall_dir);
    bool do_fall_try_eat_voodoo(int x, int y, GdDirectionEnum fall_dir);
    bool do_fall_try_crack_nut(int x, int y, GdDirectionEnum fall_dir, GdElementEnum bouncing);
    bool do_fall_try_magic(int x, int y, GdDirectionEnum fall_dir, GdElementEnum magic);
    bool do_fall_try_crush(int x, int y, GdDirectionEnum fall_dir);
    void do_fall_roll_or_stop(int x, int y, GdDirectionEnum fall_dir, GdElementEnum bouncing);

    // Cave maps
    CaveMap<int> objects_order;         ///< two-dimensional map of cave; each cell is an index to the drawing object, which created this element. -1 if map or random
    CaveMap<int> hammered_reappear;     ///< integer map of cave; if non-zero, a brick wall will appear there
    CaveMap<GdElementEnum> map;         ///< cave map

    // Variables for random number generation
    GdInt render_seed;                  ///< the seed value, which was used to render the cave, is saved here. will be used by record&playback
    RandomGenerator random;             ///< random number generator of rendered cave
    C64RandomGenerator c64_rand;        ///< used for predictable random generator during the game.

    GdBool hatched;                     ///< hatching has happened. (timers may run, ...)
    GdBool gate_open;                   ///< self-explaining
    GdInt rendered_on;                  ///< rendered at level x (0 is level1, 4 is level5)
    GdInt timing_factor;                ///< number of "milliseconds" in each second :) 1000 for ntsc, 1200 for pal.

    GdInt speed;                        ///< Time between game cycles in ms
    GdInt ckdelay;                      ///< a ckdelay value for the level this cave is rendered for

    GdInt hatching_delay_frame;         ///< frames remaining till hatching.
    GdInt hatching_delay_time;          ///< time remaining (milliseconds) till hatching
    GdInt time_bonus;                   ///< bonus time for clock collected.
    GdInt time_penalty;                 ///< Time penalty when voodoo destroyed.
    GdInt time;                         ///< milliseconds remaining to finish cave
    GdInt time_elapsed;                 ///< Time elapsed (in milliseconds) since the start of the cave
    GdInt timevalue;                    ///< points for remaining seconds - for current level
    GdInt diamonds_needed;              ///< diamonds needed to open outbox
    GdInt diamonds_collected;           ///< diamonds collected
    GdInt skeletons_collected;          ///< number of skeletons collected
    GdInt gate_open_flash;              ///< flashing of screen when gate opens, it lasts two iterations
    GdInt amoeba_time;                  ///< Amoeba growing slow (low probability, default 3%) for milliseconds. After that, fast growth default (25%)
    GdInt amoeba_2_time;                ///< Amoeba growing slow (low probability, default 3%) for milliseconds. After that, fast growth default (25%)
    GdInt amoeba_max_count;             ///< selected amoeba 1 threshold for this level
    GdInt amoeba_2_max_count;           ///< selected amoeba 2 threshold for this level
    AmoebaState amoeba_state;           ///< state of amoeba 1
    GdBool convert_amoeba_this_frame;   ///< To implement BD1 buggy amoeba+magic wall behaviour.
    AmoebaState amoeba_2_state;         ///< state of amoeba 2
    GdInt magic_wall_time;              ///< magic wall 'on' state for seconds
    GdProbability slime_permeability;           ///< true random slime
    GdInt slime_permeability_c64;       ///< Appearing in bd 2
    MagicWallState magic_wall_state;    ///< State of magic wall
    PlayerState player_state;           ///< Player state. not yet living, living, exited...
    GdInt player_seen_ago;              ///< player was seen this number of scans ago
    GdBool kill_player;                 ///< Voodoo died, or used pressed escape to restart level.
    GdBool sweet_eaten;                 ///< player ate sweet, he's strong. prob_sweet applies, and also able to push chasing stones
    GdInt player_x, player_y;           ///< Coordinates of player (for scrolling)
    enum { PlayerMemSize = 16 };
    GdInt player_x_mem[PlayerMemSize], player_y_mem[PlayerMemSize];             ///< coordinates of player, for chasing stone
    GdInt key1, key2, key3;             ///< The player is holding this number of keys of each color
    GdBool diamond_key_collected;       ///< Key collected, so trapped diamonds convert to diamonds
    GdBool inbox_flash_toggle;          ///< negated every scan. helps drawing inboxes, and making players be born at different times.
    GdInt biters_wait_frame;                ///< number of frames to wait until biters will move again
    GdInt replicators_wait_frame;           ///< number of frames to wait until replicators are activated again
    GdInt creatures_direction_will_change;  ///< creatures automatically change direction every x seconds

    GdInt gravity_will_change;              ///< gravity will change in this number of milliseconds
    GdBool gravity_disabled;                ///< when the player is stirring the pot, there is no gravity.
    GdDirectionEnum gravity_next_direction; ///< next direction when the gravity changes. will be set by the player "getting" a gravity switch
    GdBool got_pneumatic_hammer;            ///< true if the player has a pneumatic hammer
    GdInt pneumatic_hammer_active_delay;    ///< number of frames to wait, till pneumatic hammer will destroy the wall

    GdDirectionEnum last_direction;             ///< last direction player moved. used by draw routines
    GdDirectionEnum last_horizontal_direction;  ///< last horizontal direction, the player has moved. used by drawing routines
    GdBool player_blinking;                     ///< Player is blinking at the moment. Used by drawing routines
    GdBool player_tapping;                      ///< Player is tapping with his legs. Used by drawing routines

    GdBool voodoo_touched;

    SoundWithPos sound1, sound2, sound3;        ///< sound set for 3 channels after each iteration
    std::list<ParticleSet> particles;
    GdColor dirt_particle_color, dirt_2_particle_color, diamond_particle_color,
            stone_particle_color, mega_stone_particle_color,
            explosion_particle_color, magic_wall_particle_color, expanding_wall_particle_color,
            expanding_steel_wall_particle_color, lava_particle_color;

    /* these should be inside some context variable */
    GdInt score;                        ///< Score got this frame.
    GdInt ckdelay_current;              ///< ckdelay value for the current iteration
    GdInt ckdelay_extra_for_animation;  ///< bd1 and similar engines had animation bits in cave data, to set which elements to animate (firefly, butterfly, amoeba). animating an element also caused some delay each frame; according to my measurements, around 2.6 ms/element.
};

unsigned gd_cave_adler_checksum(const CaveRendered &cave);
void gd_cave_adler_checksum_more(const CaveRendered &cave, unsigned &a, unsigned &b);
int gd_cave_check_replays(CaveStored &cave, bool report, bool remove, bool repair);

#endif

