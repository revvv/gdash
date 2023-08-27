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
#ifndef CAVEBASE_HPP_INCLUDED
#define CAVEBASE_HPP_INCLUDED

#include "config.h"

#include "cave/colors.hpp"
#include "cave/cavetypes.hpp"

/// @defgroup Cave

/// @ingroup Cave
/// @brief An abstract class for the caves.
/// This is the base class for CaveStored (used to store caves on the disk) and CaveRendered (used to play the game).
/// The base class is used to make the CaveRendered(CaveStored&) constructor simple - no need to copy all properties
/// one by one.
class CaveBase {
public:
    int height() const {
        return h;
    }
    int width() const {
        return w;
    }

    GdString name;            ///< Name of cave
    GdString description;     ///< Some words about the cave
    GdString author;            ///< Author
    GdString difficulty;        ///< difficulty of the game, for info purposes
    GdString www;               ///< link to author's webpage
    GdString date;              ///< date of creation
    GdString story;             ///< story for the cave - will be shown when the cave is played.
    GdString remark;            ///< some note

    GdBool intermission;                ///< is this cave an intermission?
    GdBool intermission_instantlife;    ///< one life extra, if the intermission is reached
    GdBool intermission_rewardlife; ///< one life extra, if the intermission is successfully finished

    GdBool diagonal_movements;      ///< are diagonal movements allowed?
    GdElement snap_element;         ///< snapping (press fire+move) usually leaves space behind, but can be other
    GdBool short_explosions;            ///< in >=1stb, diamond and creature explosions were of 5 stages

    GdScheduling scheduling;        ///< scheduling type; see above
    GdBool pal_timing;              ///< use faster seconds

    GdBool active_is_first_found;           ///< active player is the uppermost.
    GdBool lineshift;                       ///< true is line shifting emulation, false is perfect borders emulation
    GdBool border_scan_first_and_last;  ///< if true, scans the first and last line of the border. false for plck
    GdBool wraparound_objects;          ///< if this is true, object drawing (cave rendering) will wraparound as well.
    GdBool border_fix;                  ///< if true, delay wrapped objects . false for no delay

    GdInt max_time;         ///< the maximum time in seconds. if above, it overflows

    GdInt w, h;             ///< Sizes of cave, width and height.
    GdInt x1, y1, x2, y2;   ///< Visible part of the cave
    GdColor colorb;         ///< border color
    GdColor color0, color1, color2, color3, color4, color5; ///< c64-style colors; 4 and 5 are amoeba and slime.

    GdInt diamond_value;            ///< Score for a diamond.
    GdInt extra_diamond_value;  ///< Score for a diamond, when gate is open.

    GdBool stone_sound;
    GdBool diamond_sound;
    GdBool nitro_sound;
    GdBool falling_wall_sound;
    GdBool expanding_wall_sound;
    GdBool bladder_spender_sound;
    GdBool bladder_convert_sound;
    GdBool amoeba_sound;            ///< if the living amoeba has sound.
    GdBool slime_sound;             ///< slime has sound
    GdBool lava_sound;              ///< elements sinking in lava have sound
    GdBool water_sound;             ///< water has sound
    GdBool biter_sound;             ///< biters have sound
    GdBool magic_wall_sound;        ///< magic wall has sound
    GdBool acid_spread_sound;       ///< acid has sound
    GdBool bladder_sound;           ///< bladder moving and pushing has sound
    GdBool replicator_sound;        ///< when replicating an element, play sound or not.
    GdBool creature_direction_auto_change_sound;    ///< automatically changing creature direction may have the sound of the creature dir switch
    GdBool gravity_change_sound;
    GdBool pneumatic_hammer_sound;
    GdBool nut_sound;

    GdBool magic_wall_stops_amoeba; ///< Turning on magic wall changes amoeba to diamonds.
    GdBool magic_wall_breakscan;    ///< Currently this setting enabled will turn the amoeba to an enclosed state. To implement buggy BD1 behaviour.
    GdBool magic_timer_zero_is_infinite;    ///< magic wall timer 0 is interpreted as infinite
    GdBool magic_timer_wait_for_hatching;   ///< magic wall timer does not start before player's birth

    GdProbability amoeba_growth_prob;       ///< Amoeba slow growth probability
    GdProbability amoeba_fast_growth_prob;  ///< Amoeba fast growth probability
    GdElement amoeba_enclosed_effect;   ///< an enclosed amoeba converts to this element
    GdElement amoeba_too_big_effect;    ///< an amoeba grown too big converts to this element

    GdProbability amoeba_2_growth_prob;     ///< Amoeba slow growth probability
    GdProbability amoeba_2_fast_growth_prob;    ///< Amoeba fast growth probability
    GdElement amoeba_2_enclosed_effect; ///< an enclosed amoeba converts to this element
    GdElement amoeba_2_too_big_effect;  ///< an amoeba grown too big converts to this element
    GdBool amoeba_2_explodes_by_amoeba; ///< amoeba 2 will explode if touched by amoeba1
    GdElement amoeba_2_explosion_effect;    ///< amoeba 2 explosion ends in ...
    GdElement amoeba_2_looks_like;  ///< an amoeba 2 looks like this element

    GdBool amoeba_timer_started_immediately;    ///< FALSE: amoeba will start life at the first possibility of growing.
    GdBool amoeba_timer_wait_for_hatching;  ///< amoeba timer does not start before player's birth

    GdElement acid_eats_this;       ///< acid eats this element
    GdProbability acid_spread_ratio;        ///< Probability of acid blowing up, each frame
    GdElement acid_turns_to;        ///< whether acid converts to explosion on spreading or other
    GdElement nut_turns_to_when_crushed;    ///< when a nut is hit by a stone, it converts to this element

    GdBool slime_predictable;               ///< predictable random start for slime. yes for plck.
    GdElement slime_eats_1, slime_converts_1;   ///< slime eats element x and converts to element x; for example diamond -> falling diamond
    GdElement slime_eats_2, slime_converts_2;
    GdElement slime_eats_3, slime_converts_3;

    GdBool voodoo_collects_diamonds;    ///< Voodoo can collect diamonds
    GdBool voodoo_dies_by_stone;        ///< Voodoo can be killed by a falling stone
    GdBool voodoo_disappear_in_explosion;   ///< Voodoo can be destroyed by and explosion
    GdBool voodoo_any_hurt_kills_player;    ///< Voodoo can be destroyed by and explosion

    GdBool water_does_not_flow_down;    ///< if true, water will not grow downwards, only in other directions.

    GdElement bladder_converts_by;  ///< bladder converts to clock by touching this element

    GdInt biter_delay_frame;        ///< frame count biters do move
    GdElement biter_eat;        ///< biters eat this

    GdBool expanding_wall_changed;  ///< expanding wall direction is changed

    GdInt replicator_delay_frame;       ///< replicator delay in frames (number of frames to wait between creating a new element)
    GdBool replicators_active;      ///< replicators are active.

    GdBool conveyor_belts_active;
    GdBool conveyor_belts_direction_changed;

    GdElement explosion_effect;         ///< explosion finally converts to this element after its last stage.
    GdElement explosion_3_effect;       ///< O_EXPLODE_3 converts to this element. diego effect, for compatibility.
    GdElement diamond_birth_effect;     ///< a diamond birth converts to this element after its last stage. diego effect.
    GdElement bomb_explosion_effect;        ///< bombs explode to this element. diego effect (almost).
    GdElement nitro_explosion_effect;   ///< nitros explode to this

    GdElement firefly_explode_to;       ///< fireflies explode to this when hit by a stone
    GdElement alt_firefly_explode_to;   ///< alternative fireflies explode to this when hit by a stone
    GdElement butterfly_explode_to;     ///< butterflies explode to this when hit by a stone
    GdElement alt_butterfly_explode_to; ///< alternative butterflies explode to this when hit by a stone
    GdElement stonefly_explode_to;      ///< stoneflies explode to this when hit by a stone
    GdElement dragonfly_explode_to;     ///< dragonflies explode to this when hit by a stone

    GdElement stone_falling_effect;     ///< a falling stone converts to this element. diego effect.
    GdElement diamond_falling_effect;   ///< a falling diamond converts to this element. diego effect.
    GdElement stone_bouncing_effect;    ///< a bouncing stone converts to this element. diego effect.
    GdElement diamond_bouncing_effect;  ///< a bouncing diamond converts to this element. diego effect.

    GdElement expanding_wall_looks_like;    ///< an expanding wall looks like this element. diego effect.
    GdElement dirt_looks_like;          ///< dirt looks like this element. diego effect.

    GdElement magic_stone_to;       ///< magic wall converts falling stone to
    GdElement magic_diamond_to;     ///< magic wall converts falling diamond to
    GdElement magic_mega_stone_to;  ///< magic wall converts a falling mega stone to
    GdElement magic_nitro_pack_to;  ///< magic wall converts a falling nitro pack to
    GdElement magic_nut_to;             ///< magic wall converts a falling nut to
    GdElement magic_flying_stone_to;    ///< flying stones are converted to
    GdElement magic_flying_diamond_to;  ///< flying diamonds are converted to

    GdProbability pushing_stone_prob;       ///< probability of pushing stone
    GdProbability pushing_stone_prob_sweet; ///< probability of pushing, after eating sweet
    GdBool mega_stones_pushable_with_sweet; ///< mega stones may be pushed with sweet

    GdBool creatures_backwards; ///< creatures changed direction
    GdBool creatures_direction_auto_change_on_start;    ///< the change occurs also at the start signal
    GdInt creatures_direction_auto_change_time; ///< creatures automatically change direction every x seconds

    GdInt skeletons_needed_for_pot; ///< how many skeletons to be collected, to use a pot
    GdInt skeletons_worth_diamonds; ///< for crazy dream 7 compatibility: collecting skeletons might open the cave door.

    GdDirection gravity;            ///< direction of gravity in cave, usually MV_DOWN
    GdInt gravity_change_time;      ///< number of seconds, after which gravity is changed
    GdBool gravity_affects_all; ///< if true, gravity also affects falling wall, bladder and waiting stones
    GdBool gravity_switch_active;   ///< true if gravity switch is activated, and can be used.

    GdBool hammered_walls_reappear; ///< if true, hammered walls will reappear some time after hammering
    GdInt pneumatic_hammer_frame;   ///< number of frames it takes to hammer a wall
    GdInt hammered_wall_reappear_frame; ///< number of frames, after which a hammered wall will reappear

    GdBool infinite_rockets;        ///< If true, the player which got a rocket launcher will be able to launch an infinite number of rockets
};

/* cave manipulation */
void gd_cave_correct_visible_size(CaveBase &cave);

void gd_cave_set_random_c64_colors(CaveBase &cave);
void gd_cave_set_random_c64dtv_colors(CaveBase &cave);
void gd_cave_set_random_atari_colors(CaveBase &cave);
void gd_cave_set_random_colors(CaveBase &cave, GdColor::Type type);


/* arrays for movements */
/* also no1 and bd2 cave data import helpers; line direction coordinates */
/* arrays for movements. also no1 and bd2 cave data import helpers; line direction coordinates */
/* static arrays, because they are in the header. this way they can be "inlined" */
static const int gd_dx[] = { 0, 0, 1, 1, 1, 0, -1, -1, -1, 0, 2, 2, 2, 0, -2, -2, -2 };
static const int gd_dy[] = { 0, -1, -1, 0, 1, 1, 1, 0, -1, -2, -2, 0, 2, 2, 2, 0, -2 };

GdDirectionEnum gd_direction_from_keypress(bool up, bool down, bool left, bool right);



#endif
