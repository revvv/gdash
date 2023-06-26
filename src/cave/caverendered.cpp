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

#include <cstdlib>

#include "cave/caverendered.hpp"
#include "cave/elementproperties.hpp"
#include "cave/helper/cavereplay.hpp"
#include "cave/cavestored.hpp"
#include "misc/logger.hpp"

/// Add extra ckdelay to cave by checking the existence some animated elements.
/// BD1 and similar engines had animation bits in cave data, to set which elements to animate (firefly, butterfly, amoeba).
/// animating an element also caused some delay each frame; according to my measurements, around 2.6 ms/element.
/// Also calculate the per iteration ckdelay value, as if we were iterating the cave.
/// So when setting up a cave for the first time, update_scheduling() can be called right after calling
/// this function, and it will immediately calculate the correct speed of the cave, even without
/// iterating it.
void CaveRendered::set_ckdelay_extra_for_animation() {
    g_assert(!map.empty());

    bool has_amoeba = false, has_firefly = false, has_butterfly = false;

    ckdelay_current = 0;
    for (int y = 0; y < height(); y++)
        for (int x = 0; x < width(); x++) {
            ckdelay_current += gd_element_properties[map(x, y)].ckdelay;
            switch (map(x, y)) {
                case O_FIREFLY_1:
                case O_FIREFLY_2:
                case O_FIREFLY_3:
                case O_FIREFLY_4:
                    has_firefly = true;
                    break;
                case O_BUTTER_1:
                case O_BUTTER_2:
                case O_BUTTER_3:
                case O_BUTTER_4:
                    has_butterfly = true;
                    break;
                case O_AMOEBA:
                    has_amoeba = true;
                    break;
                default:
                    /* other animated elements are not important,
                     * because they were not present in bd2. */
                    break;
            }
        }
    ckdelay_extra_for_animation = 0;
    if (has_amoeba)
        ckdelay_extra_for_animation += 2600;
    if (has_firefly)
        ckdelay_extra_for_animation += 2600;
    if (has_butterfly)
        ckdelay_extra_for_animation += 2600;
    if (has_amoeba)
        ckdelay_extra_for_animation += 2600;
}

/// Do some init - setup some cave variables before the game.
/// Put in a different function, so things which are not
/// important for the editor are not done when constructing the cave.
void CaveRendered::setup_for_game() {
    /* find the player which will be the one to scroll to at the beginning of the game (before the player's birth) */
    if (active_is_first_found) {
        /* uppermost player is active */
        for (int y = height() - 1; y >= 0; y--)
            for (int x = width() - 1; x >= 0; x--)
                if (map(x, y) == O_INBOX) {
                    player_x = x;
                    player_y = y;
                }
    } else {
        /* lowermost player is active */
        for (int y = 0; y < height(); y++)
            for (int x = 0; x < width(); x++)
                if (map(x, y) == O_INBOX) {
                    player_x = x;
                    player_y = y;
                }
    }
    for (unsigned i = 0; i < PlayerMemSize; ++i) {
        player_x_mem[i] = player_x;
        player_y_mem[i] = player_y;
    }

    /* select number of milliseconds (for pal and ntsc) */
    timing_factor = pal_timing ? 1200 : 1000;
    time *= timing_factor;
    magic_wall_time *= timing_factor;
    amoeba_time *= timing_factor;
    amoeba_2_time *= timing_factor;
    hatching_delay_time *= timing_factor;

    /* setup maps */
    objects_order.remove();  /* only needed by the editor */
    if (hammered_walls_reappear)
        hammered_reappear.set_size(w, h, 0);
    /* set cave get function; to implement perfect or lineshifting borders */
    if (lineshift)
        map.set_wrap_type(CaveMapFuncs::LineShift);
    else
        map.set_wrap_type(CaveMapFuncs::Perfect);

    /* set speed */
    set_ckdelay_extra_for_animation();
    update_scheduling();
}

/// Count diamonds in a cave, and set diamonds_needed accordingly.
/// Cave diamonds needed can be set to n<=0. If so, count the diamonds at the time of the hatching, and decrement that value from
/// the number of diamonds found. Of course, this function is to be called from the cave engine, at the exact time of hatching.
void CaveRendered::count_diamonds() {
    /* if automatically counting diamonds. if this was negative,
     * the sum will be this less than the number of all the diamonds in the cave */
    if (diamonds_needed <= 0) {
        for (int y = 0; y < height(); y++)
            for (int x = 0; x < width(); x++)
                switch (map(x, y)) {
                    case O_DIAMOND:
                    case O_DIAMOND_F:
                    case O_FLYING_DIAMOND:
                    case O_FLYING_DIAMOND_F:
                        ++diamonds_needed;
                        break;
                    case O_SKELETON:
                        diamonds_needed += skeletons_worth_diamonds;
                        break;
                    default:
                        break;
                }
        if (diamonds_needed < 0)
            /* if still below zero, let this be 0, so gate will be open immediately */
            diamonds_needed = 0;
    }
}


/// Draw a cave into a gfx buffer (integer map) - set the cave cell index from the png.
/// Takes a cave and a gfx buffer, and fills the buffer with cell indexes.
/// The indexes might change if bonus life flash is active (small lines in "SPACE" cells),
/// for the paused state (which is used in gdash but not in sdash) - yellowish color.
/// Also one can select the animation frame (0..7) to draw the cave on. So the caller manages
/// increasing that.
/// If a cell is changed, it is flagged with GD_REDRAW; the flag can be cleared by the caller.
/// @param gfx_buffer A map, which must be the same size as the map of the cave.
/// @param bonus_life_flash Set to true, if the player got a bonus life. The space element will change accordingly.
/// @param animcycle Animation cycle - an integer between 0 and 7 to select animated frames.
/// @param hate_invisible_outbox Show invisible outboxes as visible (blinking) ones.
void CaveRendered::draw_indexes(CaveMap<int> &gfx_buffer, CaveMap<bool> const &covered, bool bonus_life_flash, int animcycle, bool hate_invisible_outbox) {
    int elemdrawing[O_MAX_INDEX];

    g_assert(!map.empty());
    g_assert(animcycle >= 0);
    g_assert(animcycle <= 7);

    if (last_direction != MV_STILL) {   /* he is moving, so stop blinking and tapping. */
        player_blinking = false;
        player_tapping = false;
    } else {                    /* he is idle, so animations can be done. */
        if (animcycle == 0) { /* blinking and tapping is started at the beginning of animation sequences. */
            player_blinking = g_random_int_range(0, 4) == 0; /* 1/4 chance of blinking, every sequence. */
            if (g_random_int_range(0, 16) == 0) /* 1/16 chance of starting or stopping tapping. */
                player_tapping = !player_tapping;
        }
    }

    for (int x = 0; x < O_MAX_INDEX; x++)
        elemdrawing[x] = gd_element_properties[x].image_game;
    if (bonus_life_flash)
        elemdrawing[O_SPACE] = gd_element_properties[O_FAKE_BONUS].image_game;
    elemdrawing[O_MAGIC_WALL] = gd_element_properties[magic_wall_state == GD_MW_ACTIVE ? O_MAGIC_WALL : O_BRICK].image_game;
    elemdrawing[O_CREATURE_SWITCH] = gd_element_properties[creatures_backwards ? O_CREATURE_SWITCH_ON : O_CREATURE_SWITCH].image_game;
    elemdrawing[O_EXPANDING_WALL_SWITCH] = gd_element_properties[expanding_wall_changed ? O_EXPANDING_WALL_SWITCH_VERT : O_EXPANDING_WALL_SWITCH_HORIZ].image_game;
    elemdrawing[O_GRAVITY_SWITCH] = gd_element_properties[gravity_switch_active ? O_GRAVITY_SWITCH_ACTIVE : O_GRAVITY_SWITCH].image_game;
    elemdrawing[O_REPLICATOR_SWITCH] = gd_element_properties[replicators_active ? O_REPLICATOR_SWITCH_ON : O_REPLICATOR_SWITCH_OFF].image_game;
    if (!replicators_active)
        /* if the replicators are inactive, do not animate them. */
        elemdrawing[O_REPLICATOR] = abs(elemdrawing[O_REPLICATOR]);
    elemdrawing[O_CONVEYOR_SWITCH] = gd_element_properties[conveyor_belts_active ? O_CONVEYOR_SWITCH_ON : O_CONVEYOR_SWITCH_OFF].image_game;
    if (conveyor_belts_direction_changed) {
        /* if direction is changed, animation is changed. */
        int temp = elemdrawing[O_CONVEYOR_LEFT];
        elemdrawing[O_CONVEYOR_LEFT] = elemdrawing[O_CONVEYOR_RIGHT];
        elemdrawing[O_CONVEYOR_RIGHT] = temp;

        elemdrawing[O_CONVEYOR_DIR_SWITCH] = gd_element_properties[O_CONVEYOR_DIR_CHANGED].image_game;
    } else
        elemdrawing[O_CONVEYOR_DIR_SWITCH] = gd_element_properties[O_CONVEYOR_DIR_NORMAL].image_game;
    if (!conveyor_belts_active) {
        /* if they are not running, do not animate them. */
        elemdrawing[O_CONVEYOR_LEFT] = abs(elemdrawing[O_CONVEYOR_LEFT]);
        elemdrawing[O_CONVEYOR_RIGHT] = abs(elemdrawing[O_CONVEYOR_RIGHT]);
    }
    if (animcycle & 2) {
        elemdrawing[O_PNEUMATIC_ACTIVE_LEFT] += 2;  /* also a hack, like biter_switch */
        elemdrawing[O_PNEUMATIC_ACTIVE_RIGHT] += 2;
        elemdrawing[O_PLAYER_PNEUMATIC_LEFT] += 2;
        elemdrawing[O_PLAYER_PNEUMATIC_RIGHT] += 2;
    }
    /* player */
    int draw;
    if (last_direction == MV_STILL) { /* player is idle. */
        if (player_blinking && player_tapping)
            draw = gd_element_properties[O_PLAYER_TAP_BLINK].image_game;
        else if (player_blinking)
            draw = gd_element_properties[O_PLAYER_BLINK].image_game;
        else if (player_tapping)
            draw = gd_element_properties[O_PLAYER_TAP].image_game;
        else
            draw = gd_element_properties[O_PLAYER].image_game;
    } else if (last_direction == MV_UP)
        draw = gd_element_properties[O_PLAYER_UP].image_game;
    else if (last_direction == MV_DOWN)
        draw = gd_element_properties[O_PLAYER_DOWN].image_game;
    else if (last_horizontal_direction == MV_LEFT)
        draw = gd_element_properties[O_PLAYER_LEFT].image_game;
    else /* mv_right */
        draw = gd_element_properties[O_PLAYER_RIGHT].image_game;
    elemdrawing[O_PLAYER] = draw;
    elemdrawing[O_PLAYER_GLUED] = draw;
    /* player with bomb/rocketlauncher does not blink or tap - no graphics drawn for that.
     * running is drawn using w/o bomb/rocketlauncher cells */
    if (last_direction != MV_STILL) {
        elemdrawing[O_PLAYER_BOMB] = draw;
        elemdrawing[O_PLAYER_ROCKET_LAUNCHER] = draw;
    }
    elemdrawing[O_INBOX] = gd_element_properties[inbox_flash_toggle ? O_OUTBOX_OPEN : O_OUTBOX_CLOSED].image_game;
    elemdrawing[O_OUTBOX] = gd_element_properties[inbox_flash_toggle ? O_OUTBOX_OPEN : O_OUTBOX_CLOSED].image_game;
    elemdrawing[O_BITER_SWITCH] = gd_element_properties[O_BITER_SWITCH].image_game + biter_delay_frame; /* hack, cannot do this with gd_element_properties */
    /* visual effects */
    elemdrawing[O_DIRT] = elemdrawing[dirt_looks_like];
    elemdrawing[O_EXPANDING_WALL] = elemdrawing[expanding_wall_looks_like];
    elemdrawing[O_V_EXPANDING_WALL] = elemdrawing[expanding_wall_looks_like];
    elemdrawing[O_H_EXPANDING_WALL] = elemdrawing[expanding_wall_looks_like];
    elemdrawing[O_AMOEBA_2] = elemdrawing[amoeba_2_looks_like];

    /* change only graphically */
    if (hate_invisible_outbox) {
        elemdrawing[O_PRE_INVIS_OUTBOX] = elemdrawing[O_PRE_OUTBOX];
        elemdrawing[O_INVIS_OUTBOX] = elemdrawing[O_OUTBOX];
    }

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            int draw;

            if (covered(x, y))          /* if covered, real element is not important */
                draw = gd_element_properties[O_COVERED].image_game;
            else
                draw = elemdrawing[map(x, y)];
            if ((last_direction == MV_LEFT || last_direction == MV_RIGHT)
                && is_player(x, y) && can_be_pushed(x, y, last_direction)) {
                if (last_direction == MV_LEFT)
                    draw = elemdrawing[O_PLAYER_PUSH_LEFT];
                else
                    draw = elemdrawing[O_PLAYER_PUSH_RIGHT];
            }

            /* if negative, animated. */
            if (draw < 0)
                draw = -draw + animcycle;
            /* flash */
            if (gate_open_flash)
                draw += NUM_OF_CELLS;

            /* set to buffer, with caching */
            if (gfx_buffer(x, y) != draw)
                gfx_buffer(x, y) = draw | GD_REDRAW;
        }
    }
}

/// Convert cave time stored in milliseconds to a visible time in seconds.
/// Internal time may be in real milliseconds or "1200 milliseconds/second"
/// for pal timing. This is taken into account by this function.
/// Cave time returned is rounded _UP_ to seconds. So at the exact moment when it changes from
/// 2sec remaining to 1sec remaining, the player has exactly one second. When it changes
/// to zero, it is the exact moment of timeout.
/// @param internal_time The internal time variable of the cave.
/// @return The time value in seconds, which can be shown to the user.
int CaveRendered::time_visible(int internal_time) const {
    return (internal_time + timing_factor - 1) / timing_factor;
}

/// Calculate adler checksum for a rendered cave; this can be used for more caves.
void gd_cave_adler_checksum_more(const CaveRendered &cave, unsigned &a, unsigned &b) {
    for (int y = 0; y < cave.h; y++) {
        for (int x = 0; x < cave.w; x++) {
            a += gd_element_properties[cave.map(x, y)].character;
            b += a;

            a %= 65521;
            b %= 65521;
        }
    }
}

/// Calculate adler checksum for a single rendered cave
unsigned gd_cave_adler_checksum(const CaveRendered &cave) {
    unsigned a = 1;
    unsigned b = 0;

    gd_cave_adler_checksum_more(cave, a, b);
    return (b << 16) + a;
}

int gd_cave_check_replays(CaveStored &cave, bool report, bool remove, bool repair) {
    int wrong = 0;
    for (std::list<CaveReplay>::iterator it = cave.replays.begin(); it != cave.replays.end(); ++it) {
        CaveReplay &replay = *it;
        GdInt checksum;

        CaveRendered rendered(cave, replay.level - 1, replay.seed);
        checksum = gd_cave_adler_checksum(rendered);

        replay.wrong_checksum = false;
        /* count wrong ones... the checksum might be changed later to "repair" */
        if (replay.checksum != 0 && checksum != replay.checksum)
            wrong++;

        if (replay.checksum == 0 || repair) {
            /* if no checksum found, add one. or if repair requested, overwrite old one. */
            replay.checksum = checksum;
        } else {
            /* if has a checksum, compare with this one. */
            if (replay.checksum != checksum) {
                replay.wrong_checksum = true;

                if (report)
                    gd_warning("%s: replay played by %s at %s is invalid", cave.name, replay.player_name, replay.date);

                if (remove)
                    cave.replays.erase(it);
            }
        }
    }

    return wrong;
}


/// Put an element to the specified position.
/// Performs range checking.
/// If wraparound objects are selected, wraps around x coordinates, with or without lineshift.
/// (The y coordinate is not wrapped, as it did not work like that on the c64.)
/// Order is a pointer to the CaveObject describing this object which sets this element of the map.
/// Thus the editor can identify which cell was created by which object.
/// @param x The x coordinate to draw at.
/// @param y The y coordinate to draw at.
/// @param element The element to draw.
/// @param order_idx Index the object which draws this element, or -1 if none.
void CaveRendered::store_rc(int x, int y, GdElementEnum element, int order_idx) {
    /* if we do not need to draw, exit now */
    if (element == O_NONE)
        return;

    /* if objects wrap around (mainly in imported caves), correct the coordinates */
    if (wraparound_objects) {
        if (lineshift)
            CaveMapFuncs::lineshift_wrap_coords_only_x(w, h, x, y);
        else
            CaveMapFuncs::perfect_wrap_coords(w, h, x, y);
    }

    /* if the above wraparound code fixed the coordinates, this will always be true. */
    /* if lineshift drawing is enabled, y might be negative or overflow. */
    if (x >= 0 && x < w && y >= 0 && y < h) {
        map(x, y) = element;
        objects_order(x, y) = order_idx;
    }
}

/// Creates a map for a playable cave.
/// It is in a separate function, so the editor can call it - and there
/// is no need to always recreate the full CaveRendered object.
/// Also changes the random seed values!
/// Must write this in a way so it can be called many times for a single CaveRendered object!
/// @param data The stored cave to read the map, objects and random values from
void CaveRendered::create_map(CaveStored const &data, int level) {
    rendered_on = level;
    if (data.map.empty()) {
        /* if we have no map, fill with predictable random generator. */
        map.set_size(w, h);

        /* IF CAVE HAS NO MAP, USE THE RANDOM NUMBER GENERATOR */
        /* init c64 randomgenerator */
        if (data.level_rand[level] < 0)
            c64_rand.set_seed(random.rand_int_range(0, 256), random.rand_int_range(0, 256));
        else
            c64_rand.set_seed(data.level_rand[level]);

        /* generate random fill
         * start from row 1 (0 skipped), and fill also the borders on left and right hand side,
         * as c64 did. this way works the original random generator the right way.
         * also, do not fill last row, that is needed for the random seeds to be correct
         * after filling! predictable slime will use it. */
        for (int y = 1; y < h - 1; y++) {
            for (int x = 0; x < w; x++) {
                int randm;

                if (data.level_rand[level] < 0)
                    randm = random.rand_int_range(0, 256);  /* use the much better glib random generator */
                else
                    randm = c64_rand.random();  /* use c64 */

                /* select the element to draw the way it was done on c64 */
                GdElement element = data.initial_fill;
                if (randm < data.random_fill_probability_1)
                    element = data.random_fill_1;
                if (randm < data.random_fill_probability_2)
                    element = data.random_fill_2;
                if (randm < data.random_fill_probability_3)
                    element = data.random_fill_3;
                if (randm < data.random_fill_probability_4)
                    element = data.random_fill_4;

                map(x, y) = element;
            }
        }

        /* draw initial border */
        for (int y = 0; y < h; y++) {
            map(0, y) = data.initial_border;
            map(w - 1, y) = data.initial_border;
        }
        for (int x = 0; x < w; x++) {
            map(x, 0) = data.initial_border;
            map(x, h - 1) = data.initial_border;
        }
    } else {
        /* if the cave has a map, simply use it, no need to fill with random elements */
        map = data.map;
        /* initialize c64 predictable random for slime. the values were taken from afl bd, see docs/internals.txt */
        c64_rand.set_seed(0, 0x1e);
    }

    /* render cave objects above random data or map */
    /* first, set map wraparound type - this is for the get's to work correctly */
    if (lineshift)
        map.set_wrap_type(CaveMapFuncs::LineShift);
    else
        map.set_wrap_type(CaveMapFuncs::Perfect);
    /* then draw objects */
    objects_order.fill(-1);
    for (int order_idx = 0; order_idx < (int)data.objects.size(); ++order_idx) {
        CaveObject const & object = data.objects[order_idx];
        if (object.seen_on[rendered_on])
            object.draw(*this, order_idx);
    }
}

/// Create a new CaveRendered, which is a cave used for game.
/// @param data The original CaveStored with objects and maybe no map.
/// @param level The level to draw at, 0 is level1, 4 is level5.
/// @param seed Random seed.
CaveRendered::CaveRendered(CaveStored const &data, int level, int seed)
    :
    CaveBase(data),
    amoeba_state(GD_AM_SLEEPING),
    amoeba_2_state(GD_AM_SLEEPING),
    magic_wall_state(GD_MW_DORMANT),
    player_state(GD_PL_NOT_YET) {
    rendered_on = level;

    render_seed = seed;

    time = data.level_time[level];
    timevalue = data.level_timevalue[level];
    diamonds_needed = data.level_diamonds[level];
    magic_wall_time = data.level_magic_wall_time[level];
    slime_permeability = data.level_slime_permeability[level];
    slime_permeability_c64 = data.level_slime_permeability_c64[level];
    time_bonus = data.level_bonus_time[level];
    time_penalty = data.level_penalty_time[level];
    amoeba_time = data.level_amoeba_time[level];
    amoeba_max_count = data.level_amoeba_threshold[level];
    amoeba_2_time = data.level_amoeba_2_time[level];
    amoeba_2_max_count = data.level_amoeba_2_threshold[level];
    hatching_delay_time = data.level_hatching_delay_time[level];
    hatching_delay_frame = data.level_hatching_delay_frame[level];
    speed = data.level_speed[level];
    ckdelay = data.level_ckdelay[level];

    random.set_seed(render_seed);
    objects_order.resize(w, h);
    create_map(data, level);

    /* if a specific slime seed is requested, change it now, after creating map data */
    /* if there is -1 in the c64 random seed, it means "leave the values those left here by the cave setup routine" */
    if (data.level_slime_seed_c64[level] != -1)
        c64_rand.set_seed(data.level_slime_seed_c64[level] / 256, data.level_slime_seed_c64[level] % 256);

    /* check if we use c64 ckdelay or milliseconds for timing */
    /* if so set something for first iteration, then later it will be calculated */
    if (scheduling != GD_SCHEDULING_MILLISECONDS)
        speed = 120;

    gd_cave_correct_visible_size(*this);

    last_direction = MV_STILL;
    last_horizontal_direction = MV_STILL;
}


void CaveRendered::update_scheduling() {
    /* SCHEDULING */

    /* update timing calculated by iterating and counting elements which
     * were slow to process on c64.
     * some caves were delay loop based.
     * some had proper timing routine - but if the cave was too complex,
     * it ran slower than the time requested.
     * this is were std::max is used, to select the slower. */
    switch (GdSchedulingEnum(scheduling)) {
        case GD_SCHEDULING_MILLISECONDS:
            /* speed already contains the milliseconds value, do not touch it */
            break;

        case GD_SCHEDULING_BD1:
            if (!intermission)
                /* non-intermissions */
                speed = (88 + 3.66 * ckdelay + (ckdelay_current + ckdelay_extra_for_animation) / 1000);
            else
                /* intermissions were quicker, as only lines 1-12 were processed by the engine. */
                speed = (60 + 3.66 * ckdelay + (ckdelay_current + ckdelay_extra_for_animation) / 1000);
            break;

        case GD_SCHEDULING_BD1_ATARI:
            /* about 20ms/frame faster than c64 version */
            if (!intermission)
                speed = (74 + 3.2 * ckdelay + ckdelay_current / 1000); /* non-intermissions */
            else
                speed = (65 + 2.88 * ckdelay + ckdelay_current / 1000); /* for intermissions */
            break;

        case GD_SCHEDULING_BD2:
            /* 60 is a guess. */
            speed = std::max(60 + (ckdelay_current + ckdelay_extra_for_animation) / 1000, ckdelay * 20);
            break;

        case GD_SCHEDULING_PLCK:
            /* 65 is totally empty cave in construction kit, with delay=0) */
            speed = std::max(65 + ckdelay_current / 1000, ckdelay * 20);
            break;

        case GD_SCHEDULING_BD2_PLCK_ATARI:
            /* a really fast engine; timing works like c64 plck. */
            /* 40 ms was measured in the construction kit, with delay=0 */
            speed = std::max(40 + ckdelay_current / 1000, ckdelay * 20);
            break;

        case GD_SCHEDULING_CRDR:
            if (hammered_walls_reappear)        /* this made the engine very slow. */
                ckdelay_current += 60000;
            speed = std::max(130 + ckdelay_current / 1000, ckdelay * 20);
            break;

        case GD_SCHEDULING_MAX:
            /* to avoid compiler warning */
            g_assert_not_reached();
            break;
    }
}
