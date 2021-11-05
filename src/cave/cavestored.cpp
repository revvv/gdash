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

#include <glib/gi18n.h>

#include "cave/cavestored.hpp"
#include "cave/helper/caverandom.hpp"



/* entries. */
/* type given for each element;
 * GD_TYPE_ELEMENT represents a combo box of gdash objects.
 * GD_TAB&LABEL represents a notebook tab or a label.
 * others are self-explanatory. */
PropertyDescription const CaveStored::descriptor[] = {
    /* default data */
    {"", GD_TAB, 0, N_("Cave data")},
    {"Name", GD_TYPE_STRING, 0, N_("Name"), GetterBase::create_new(&CaveStored::name), N_("Name of cave")},
    {"Description", GD_TYPE_STRING, 0, N_("Description"), GetterBase::create_new(&CaveStored::description), N_("Some words about the cave")},
    {"Author", GD_TYPE_STRING, 0, N_("Author"), GetterBase::create_new(&CaveStored::author), N_("Name of author")},
    {"Date", GD_TYPE_STRING, 0, N_("Date"), GetterBase::create_new(&CaveStored::date), N_("Date of creation")},
    {"WWW", GD_TYPE_STRING, 0, N_("WWW"), GetterBase::create_new(&CaveStored::www), N_("Web page or e-mail address")},
    {"Difficulty", GD_TYPE_STRING, 0, N_("Difficulty"), GetterBase::create_new(&CaveStored::difficulty), N_("Difficulty (informative)")},

    {"Selectable", GD_TYPE_BOOLEAN, 0, N_("Selectable as start"), GetterBase::create_new(&CaveStored::selectable), N_("This sets whether the game can be started at this cave.")},
    {"IntermissionProperties.instantlife", GD_TYPE_BOOLEAN, 0, N_("   Instant life"), GetterBase::create_new(&CaveStored::intermission_instantlife), N_("If true, an extra life is given to the player, when the intermission cave is reached.")},
    {"Intermission", GD_TYPE_BOOLEAN, GD_ALWAYS_SAVE, N_("Intermission"), GetterBase::create_new(&CaveStored::intermission), N_("Intermission caves are usually small and fast caves, which are not required to be solved. The player will not lose a life if he is not successful. The game always proceeds to the next cave.")},
    {"IntermissionProperties.rewardlife", GD_TYPE_BOOLEAN, 0, N_("   Reward life"), GetterBase::create_new(&CaveStored::intermission_rewardlife), N_("If true, an extra life is given to the player, when the intermission cave is successfully finished.")},
    {"Size", GD_TYPE_INT, GD_ALWAYS_SAVE, N_("Width"), GetterBase::create_new(&CaveStored::w), N_("Width of cave. The standard size for a cave is 40x22, and 20x12 for an intermission."), 12, 128},
    {"Size", GD_TYPE_INT, GD_ALWAYS_SAVE, N_("Height"), GetterBase::create_new(&CaveStored::h), N_("Height of cave. The standard size for a cave is 40x22, and 20x12 for an intermission."), 12, 128},
    {"Size", GD_TYPE_INT, GD_ALWAYS_SAVE | GD_DONT_SHOW_IN_EDITOR, N_("Visible, left"), GetterBase::create_new(&CaveStored::x1), N_("Visible parts of the cave, upper left and lower right corner."), 0, 127},
    {"Size", GD_TYPE_INT, GD_ALWAYS_SAVE | GD_DONT_SHOW_IN_EDITOR, N_("Visible, upper"), GetterBase::create_new(&CaveStored::y1), N_("Visible parts of the cave, upper left and lower right corner."), 0, 127},
    {"Size", GD_TYPE_INT, GD_ALWAYS_SAVE | GD_DONT_SHOW_IN_EDITOR, N_("Visible, right"), GetterBase::create_new(&CaveStored::x2), N_("Visible parts of the cave, upper left and lower right corner."), 0, 127},
    {"Size", GD_TYPE_INT, GD_ALWAYS_SAVE | GD_DONT_SHOW_IN_EDITOR, N_("Visible, lower"), GetterBase::create_new(&CaveStored::y2), N_("Visible parts of the cave, upper left and lower right corner."), 0, 127},

    {"Charset", GD_TYPE_STRING, 0, N_("Character set"), GetterBase::create_new(&CaveStored::charset), N_("Theme used for displaying the game. Informative, not used by GDash.")},
    {"Fontset", GD_TYPE_STRING, 0, N_("Font set"), GetterBase::create_new(&CaveStored::fontset), N_("Font used during the game. Informative, not used by GDash.")},

    /* story - a tab on its own */
    {"", GD_TAB, 0, N_("Story")},
    {"Story", GD_TYPE_LONGSTRING, 0, 0, GetterBase::create_new(&CaveStored::story), N_("Story for the cave. It will be shown when the cave is played.")},

    /* remark - also a tab on its own */
    {"", GD_TAB, 0, N_("Remark")},
    {"Remark", GD_TYPE_LONGSTRING, 0, 0, GetterBase::create_new(&CaveStored::remark), N_("Remark (informative). Can contain supplementary information about the design of the cave. It is not shown during the game, only when the user requests the cave info dialog, so can also contain spoilers and hints.")},

    // do not show them in editor, but save them in bdcff. they have a separate dialog box
    {"Colors", GD_TYPE_COLOR, GD_ALWAYS_SAVE | GD_DONT_SHOW_IN_EDITOR, 0, GetterBase::create_new(&CaveStored::colorb), 0},
    {"Colors", GD_TYPE_COLOR, GD_ALWAYS_SAVE | GD_DONT_SHOW_IN_EDITOR, 0, GetterBase::create_new(&CaveStored::color0), 0},
    {"Colors", GD_TYPE_COLOR, GD_ALWAYS_SAVE | GD_DONT_SHOW_IN_EDITOR, 0, GetterBase::create_new(&CaveStored::color1), 0},
    {"Colors", GD_TYPE_COLOR, GD_ALWAYS_SAVE | GD_DONT_SHOW_IN_EDITOR, 0, GetterBase::create_new(&CaveStored::color2), 0},
    {"Colors", GD_TYPE_COLOR, GD_ALWAYS_SAVE | GD_DONT_SHOW_IN_EDITOR, 0, GetterBase::create_new(&CaveStored::color3), 0},
    {"Colors", GD_TYPE_COLOR, GD_ALWAYS_SAVE | GD_DONT_SHOW_IN_EDITOR, 0, GetterBase::create_new(&CaveStored::color4), 0},
    {"Colors", GD_TYPE_COLOR, GD_ALWAYS_SAVE | GD_DONT_SHOW_IN_EDITOR, 0, GetterBase::create_new(&CaveStored::color5), 0},

    /* difficulty */
    {"", GD_TAB, 0, N_("Difficulty")},
    {"", GD_LABEL, GD_SHOW_LEVEL_LABEL, N_("Difficulty")},
    {"DiamondsRequired", GD_TYPE_INT_LEVELS, GD_ALWAYS_SAVE, N_("Diamonds needed"), GetterBase::create_new(&CaveStored::level_diamonds), N_("Here zero means automatically count diamonds before level start. If negative, the value is subtracted from that. This is useful for totally random caves."), -100, 999},
    {"", GD_LABEL, 0, N_("Time")},
    {"CaveTime", GD_TYPE_INT_LEVELS, GD_ALWAYS_SAVE, N_("Time (s)"), GetterBase::create_new(&CaveStored::level_time), N_("Time available to solve cave, in seconds."), 1, 999},
    {"CaveMaxTime", GD_TYPE_INT, 0, N_("Maximum time (s)"), GetterBase::create_new(&CaveStored::max_time), N_("If you reach this time by collecting too many clocks, the timer will overflow."), 60, 999},
    {"CaveScheduling", GD_TYPE_SCHEDULING, GD_ALWAYS_SAVE, N_("Scheduling type"), GetterBase::create_new(&CaveStored::scheduling), N_("This flag sets whether the game uses an emulation of the original timing (c64-style), or a more modern milliseconds-based timing. The original game used a delay (empty loop) based timing of caves; this is selected by setting this to BD1, BD2, Construction Kit or Crazy Dream 7. This is a compatibility setting only; milliseconds-based timing is recommended for every new cave.")},
    {"PALTiming", GD_TYPE_BOOLEAN, 0, N_("PAL timing"), GetterBase::create_new(&CaveStored::pal_timing), N_("On the PAL version of the C64 computer, the timer was actually slower than normal seconds. This flag is used to compensate for this. If enabled, one game second will last 1.2 real seconds. Most original games were authored for the PAL version. This is a compatibility setting for imported caves; it is not recommended to enable it for newly authored ones.")},
    {"FrameTime", GD_TYPE_INT_LEVELS, GD_ALWAYS_SAVE, N_("   Speed (ms)"), GetterBase::create_new(&CaveStored::level_speed), N_("Number of milliseconds between game frames. Used when milliseconds-based timing is active, i.e. C64 scheduling is off."), 50, 500},
    {"HatchingDelay", GD_TYPE_INT_LEVELS, 0, N_("   Hatching delay (frames)"), GetterBase::create_new(&CaveStored::level_hatching_delay_frame), N_("This value sets how much the cave will move until the player enters the cave, and is expressed in frames. This is used for the milliseconds-based scheduling."), 1, 40},
    {"CaveDelay", GD_TYPE_INT_LEVELS, GD_ALWAYS_SAVE, N_("   Delay (C64-style)"), GetterBase::create_new(&CaveStored::level_ckdelay), N_("The length of the delay loop between game frames. Used when milliseconds-based timing is inactive, i.e. some kind of C64 or Atari scheduling is selected."), 0, 32},
    {"HatchingTime", GD_TYPE_INT_LEVELS, 0, N_("   Hatching time (seconds)"), GetterBase::create_new(&CaveStored::level_hatching_delay_time), N_("This value sets how much the cave will move until the player enters the cave. This is used for the C64-like schedulings."), 1, 40},
    {"", GD_LABEL, GD_SHOW_LEVEL_LABEL, N_("Scoring")},
    {"TimeValue", GD_TYPE_INT_LEVELS, 0, N_("Score for time"), GetterBase::create_new(&CaveStored::level_timevalue), N_("Points for each second remaining, when the player exits the level."), 0, 50},
    {"DiamondValue", GD_TYPE_INT, GD_ALWAYS_SAVE, N_("Score for diamonds"), GetterBase::create_new(&CaveStored::diamond_value), N_("Number of points per diamond collected, before opening the exit."), 0, 100},
    {"DiamondValue", GD_TYPE_INT, GD_ALWAYS_SAVE, N_("Score for extra diamonds"), GetterBase::create_new(&CaveStored::extra_diamond_value), N_("Number of points per diamond collected, after opening the exit."), 0, 100},

    // cave initial fill - do not show them in editor (they are shown in a separate dialog box), but save them in bdcff
    /* initial fill */
    {"RandSeed", GD_TYPE_INT_LEVELS, GD_DONT_SHOW_IN_EDITOR, NULL /* random seed value */, GetterBase::create_new(&CaveStored::level_rand), NULL, -1, 255},
    {"InitialBorder", GD_TYPE_ELEMENT, GD_DONT_SHOW_IN_EDITOR, NULL /* Initial border */, GetterBase::create_new(&CaveStored::initial_border), NULL},
    {"InitialFill", GD_TYPE_ELEMENT, GD_DONT_SHOW_IN_EDITOR, NULL /* Initial fill */, GetterBase::create_new(&CaveStored::initial_fill), NULL},
    /* RandomFill */
    {"RandomFill", GD_TYPE_ELEMENT, GD_DONT_SHOW_IN_EDITOR, NULL, GetterBase::create_new(&CaveStored::random_fill_1), NULL},
    {"RandomFill", GD_TYPE_INT, GD_DONT_SHOW_IN_EDITOR, NULL, GetterBase::create_new(&CaveStored::random_fill_probability_1), NULL, 0, 255},
    {"RandomFill", GD_TYPE_ELEMENT, GD_DONT_SHOW_IN_EDITOR, NULL, GetterBase::create_new(&CaveStored::random_fill_2), NULL},
    {"RandomFill", GD_TYPE_INT, GD_DONT_SHOW_IN_EDITOR, NULL, GetterBase::create_new(&CaveStored::random_fill_probability_2), NULL, 0, 255},
    {"RandomFill", GD_TYPE_ELEMENT, GD_DONT_SHOW_IN_EDITOR, NULL, GetterBase::create_new(&CaveStored::random_fill_3), NULL},
    {"RandomFill", GD_TYPE_INT, GD_DONT_SHOW_IN_EDITOR, NULL, GetterBase::create_new(&CaveStored::random_fill_probability_3), NULL, 0, 255},
    {"RandomFill", GD_TYPE_ELEMENT, GD_DONT_SHOW_IN_EDITOR, NULL, GetterBase::create_new(&CaveStored::random_fill_4), NULL},
    {"RandomFill", GD_TYPE_INT, GD_DONT_SHOW_IN_EDITOR, NULL, GetterBase::create_new(&CaveStored::random_fill_probability_4), NULL, 0, 255},

    /* PLAYER */
    {"", GD_TAB, 0, N_("Player")},
    /* player */
    {"", GD_LABEL, 0, N_("Player movements")},
    {"DiagonalMovement", GD_TYPE_BOOLEAN, 0, N_("Diagonal movements"), GetterBase::create_new(&CaveStored::diagonal_movements), N_("Controls if the player can move diagonally.")},
    {"ActiveGuyIsFirst", GD_TYPE_BOOLEAN, 0, N_("Uppermost player active"), GetterBase::create_new(&CaveStored::active_is_first_found), N_("In 1stB, cave is scrolled to the uppermost and leftmost player found, whereas in the original game to the last one. Chasing stones also follow the active player.")},
    {"SnapEffect", GD_TYPE_ELEMENT, 0, N_("Snap element"), GetterBase::create_new(&CaveStored::snap_element), N_("Snapping (pressing fire while moving) usually creates space, but it can create any other element.")},
    {"PushingBoulderProb", GD_TYPE_PROBABILITY, 0, N_("Probability of pushing (%)"), GetterBase::create_new(&CaveStored::pushing_stone_prob), N_("Chance of player managing to push a stone, every game cycle he tries. This is the normal probability.")},
    {"", GD_LABEL, 0, N_("Sweet")},
    {"PushingBoulderProb", GD_TYPE_PROBABILITY, 0, N_("Probability of pushing (%)"), GetterBase::create_new(&CaveStored::pushing_stone_prob_sweet), N_("Chance of player managing to push a stone, every game cycle he tries. This is used after eating sweet.")},
    {"PushingMegaStonesAfterSweet", GD_TYPE_BOOLEAN, 0, N_("Mega stones pushable"), GetterBase::create_new(&CaveStored::mega_stones_pushable_with_sweet), N_("If it is true, mega stones can be pushed after eating sweet.")},
    /* rocket launcher */
    {"", GD_LABEL, 0, N_("Rocket launcher")},
    {"RocketLauncher.infinite", GD_TYPE_BOOLEAN, 0, N_("Infinite rockets"), GetterBase::create_new(&CaveStored::infinite_rockets), N_("If it is true, the player is able to launch an infinite number of rockets. Otherwise every rocket launcher contains only a single rocket.")},

    /* pneumatic hammer */
    {"", GD_LABEL, 0, N_("Pneumatic hammer")},
    {"PneumaticHammer.frames", GD_TYPE_INT, 0, N_("Time for hammer (frames)"), GetterBase::create_new(&CaveStored::pneumatic_hammer_frame), N_("This is the number of game frames, a pneumatic hammer is required to break a wall."), 1, 100},
    {"PneumaticHammer.wallsreappear", GD_TYPE_BOOLEAN, 0, N_("Hammered walls reappear"), GetterBase::create_new(&CaveStored::hammered_walls_reappear), N_("If this is set to true, walls broken with a pneumatic hammer will reappear later.")},
    {"PneumaticHammer.wallsreappearframes", GD_TYPE_INT, 0, N_("   Timer for reappear (frames)"), GetterBase::create_new(&CaveStored::hammered_wall_reappear_frame), N_("This sets the number of game frames, after hammered walls reappear, when the above setting is true."), 1, 200},
    /* clock */
    {"", GD_LABEL, GD_SHOW_LEVEL_LABEL, N_("Clock")},
    {"BonusTime", GD_TYPE_INT_LEVELS, 0, N_("Time bonus (s)"), GetterBase::create_new(&CaveStored::level_bonus_time), N_("Bonus time when a clock is collected."), -100, 100},
    /* voodoo */
    {"", GD_LABEL, 0, N_("Voodoo Doll")},
    {"DummyProperties.diamondcollector", GD_TYPE_BOOLEAN, 0, N_("Can collect diamonds"), GetterBase::create_new(&CaveStored::voodoo_collects_diamonds), N_("Controls if a voodoo doll can collect diamonds for the player.")},
    {"DummyProperties.penalty", GD_TYPE_BOOLEAN, 0, N_("Dies if hit by a stone"), GetterBase::create_new(&CaveStored::voodoo_dies_by_stone), N_("Controls if the voodoo doll dies if it is hit by a stone. Then the player gets a time penalty, and it is turned to a gravestone surrounded by steel wall.")},
    {"DummyProperties.destructable", GD_TYPE_BOOLEAN, 0, N_("Disappear in explosion"), GetterBase::create_new(&CaveStored::voodoo_disappear_in_explosion), N_("Controls if the voodoo can be destroyed by an explosion nearby. If not, it is converted to a gravestone, and you get a time penalty. If yes, the voodoo simply disappears.")},
    {"DummyProperties.alwayskillsplayer", GD_TYPE_BOOLEAN, 0, N_("Any way hurt, player explodes"), GetterBase::create_new(&CaveStored::voodoo_any_hurt_kills_player), N_("If this setting is enabled, the player will explode if the voodoo is hurt in any possible way, i.e. touched by a firefly, hit by a stone or an explosion.")},
    {"PenaltyTime", GD_TYPE_INT_LEVELS, 0, N_("Time penalty (s)"), GetterBase::create_new(&CaveStored::level_penalty_time), N_("Penalty time when the voodoo is destroyed by a stone."), 0, 100},

    /* AMOEBA */
    {"", GD_TAB, 0, N_("Amoeba")},
    {"", GD_LABEL, 0, N_("Timing")},
    {"AmoebaProperties.immediately", GD_TYPE_BOOLEAN, 0, N_("Timer started immediately"), GetterBase::create_new(&CaveStored::amoeba_timer_started_immediately), N_("If this flag is enabled, the amoeba slow growth timer will start at the beginning of the cave, regardless of the amoeba being let free or not. This can make a big difference when playing the cave!")},
    {"AmoebaProperties.waitforhatching", GD_TYPE_BOOLEAN, 0, N_("Timer waits for hatching"), GetterBase::create_new(&CaveStored::amoeba_timer_wait_for_hatching), N_("This determines if the amoeba timer starts before the player appearing. Amoeba can always be activated before that; but if this is set to true, the timer will not start. This setting is for compatiblity for some old imported caves. As the player is usually born within a few seconds, changing this setting makes not much difference. It is not advised to change it, set the slow growth time to fit your needs instead.")},
    /* amoeba */
    {"", GD_LABEL, GD_SHOW_LEVEL_LABEL, N_("Amoeba")},
    {"AmoebaThreshold", GD_TYPE_INT_LEVELS, GD_BDCFF_RATIO_TO_CAVE_SIZE, N_("Threshold (cells)"), GetterBase::create_new(&CaveStored::level_amoeba_threshold), N_("If the amoeba grows more than this fraction of the cave, it is considered too big and it converts to the element specified below."), 0, 16383},
    {"AmoebaTime", GD_TYPE_INT_LEVELS, 0, N_("Slow growth time (s)"), GetterBase::create_new(&CaveStored::level_amoeba_time), N_("After this time, amoeba will grow very quickly."), 0, 999},
    {"AmoebaGrowthProb", GD_TYPE_PROBABILITY, 0, N_("Growth ratio, slow (%)"), GetterBase::create_new(&CaveStored::amoeba_growth_prob), N_("This sets the speed at which a slow amoeba grows.")},
    {"AmoebaGrowthProb", GD_TYPE_PROBABILITY, 0, N_("Growth ratio, fast (%)"), GetterBase::create_new(&CaveStored::amoeba_fast_growth_prob), N_("This sets the speed at which a fast amoeba grows.")},
    {"AMOEBABOULDEReffect", GD_TYPE_EFFECT, 0, N_("If too big, converts to"), GetterBase::create_new(&CaveStored::amoeba_too_big_effect), N_("Controls which element an overgrown amoeba converts to.")},
    {"AMOEBADIAMONDeffect", GD_TYPE_EFFECT, 0, N_("If enclosed, converts to"), GetterBase::create_new(&CaveStored::amoeba_enclosed_effect), N_("Controls which element an enclosed amoeba converts to.")},
    {"", GD_LABEL, GD_SHOW_LEVEL_LABEL, N_("Amoeba 2")},
    {"Amoeba2Threshold", GD_TYPE_INT_LEVELS, GD_BDCFF_RATIO_TO_CAVE_SIZE, N_("Threshold (cells)"), GetterBase::create_new(&CaveStored::level_amoeba_2_threshold), N_("If the amoeba grows more than this fraction of the cave, it is considered too big and it converts to the element specified below."), 0, 16383},
    {"Amoeba2Time", GD_TYPE_INT_LEVELS, 0, N_("Slow growth time (s)"), GetterBase::create_new(&CaveStored::level_amoeba_2_time), N_("After this time, amoeba will grow very quickly."), 0, 999},
    {"Amoeba2GrowthProb", GD_TYPE_PROBABILITY, 0, N_("Growth ratio, slow (%)"), GetterBase::create_new(&CaveStored::amoeba_2_growth_prob), N_("This sets the speed at which a slow amoeba grows.")},
    {"Amoeba2GrowthProb", GD_TYPE_PROBABILITY, 0, N_("Growth ratio, fast (%)"), GetterBase::create_new(&CaveStored::amoeba_2_fast_growth_prob), N_("This sets the speed at which a fast amoeba grows.")},
    {"Amoeba2Properties.explode", GD_TYPE_BOOLEAN, 0, N_("Explodes by amoeba"), GetterBase::create_new(&CaveStored::amoeba_2_explodes_by_amoeba), N_("If this setting is enabled, an amoeba 2 will explode if it is touched by a normal amoeba.")},
    {"AMOEBA2EXPLOSIONeffect", GD_TYPE_EFFECT, 0, N_("   Explosion ends in"), GetterBase::create_new(&CaveStored::amoeba_2_explosion_effect), N_("An amoeba 2 explodes to this element, when touched by the original amoeba.")},
    {"AMOEBA2BOULDEReffect", GD_TYPE_EFFECT, 0, N_("If too big, converts to"), GetterBase::create_new(&CaveStored::amoeba_2_too_big_effect), N_("Controls which element an overgrown amoeba converts to.")},
    {"AMOEBA2DIAMONDeffect", GD_TYPE_EFFECT, 0, N_("If enclosed, converts to"), GetterBase::create_new(&CaveStored::amoeba_2_enclosed_effect), N_("Controls which element an enclosed amoeba converts to.")},
    {"AMOEBA2LOOKSLIKEeffect", GD_TYPE_EFFECT, 0, N_("Looks like"), GetterBase::create_new(&CaveStored::amoeba_2_looks_like), N_("Amoeba 2 can look like any other element. Hint: it can also look like a normal amoeba. Or it can look like slime, and then you have two different colored amoebas!")},

    /* magic wall */
    {"", GD_TAB, 0, N_("Magic Wall")},
    {"", GD_LABEL, GD_SHOW_LEVEL_LABEL, N_("Timing")},
    {"MagicWallTime", GD_TYPE_INT_LEVELS, 0, N_("Milling time (s)"), GetterBase::create_new(&CaveStored::level_magic_wall_time), N_("Magic wall will stop after this time, and it cannot be activated again."), 0, 999},
    {"MagicWallProperties.waitforhatching", GD_TYPE_BOOLEAN, 0, N_("Timer waits for hatching"), GetterBase::create_new(&CaveStored::magic_timer_wait_for_hatching), N_("This determines if the magic wall timer starts before the player appearing. Magic can always be activated before that; but if this is set to true, the timer will not start.")},
    {"", GD_LABEL, 0, N_("Conversions")},
    {"MagicWallProperties", GD_TYPE_ELEMENT, 0, N_("Diamond to"), GetterBase::create_new(&CaveStored::magic_diamond_to), N_("As a special effect, magic walls can convert diamonds to any other element.")},
    {"MagicWallProperties", GD_TYPE_ELEMENT, 0, N_("Stone to"), GetterBase::create_new(&CaveStored::magic_stone_to), N_("As a special effect, magic walls can convert stones to any other element.")},
    {"MagicWallProperties.megastoneto", GD_TYPE_ELEMENT, 0, N_("Mega stone to"), GetterBase::create_new(&CaveStored::magic_mega_stone_to), N_("If a mega stone falls into the magic wall, it will drop this element.")},
    {"MagicWallProperties.nitropackto", GD_TYPE_ELEMENT, 0, N_("Nitro pack to"), GetterBase::create_new(&CaveStored::magic_nitro_pack_to), N_("If a nitro pack falls into the magic wall, it will be turned to this element.")},
    {"MagicWallProperties.nutto", GD_TYPE_ELEMENT, 0, N_("Nut to"), GetterBase::create_new(&CaveStored::magic_nut_to), N_("As a special effect, magic walls can convert nuts to any other element.")},
    {"MagicWallProperties.flyingstoneto", GD_TYPE_ELEMENT, 0, N_("Flying stone to"), GetterBase::create_new(&CaveStored::magic_flying_stone_to), N_("If a flying stone climbs up into the magic wall, it will be turned to this element. Remember that flying stones enter the magic wall from its bottom, not from the top!")},
    {"MagicWallProperties.flyingdiamondto", GD_TYPE_ELEMENT, 0, N_("Flying diamonds to"), GetterBase::create_new(&CaveStored::magic_flying_diamond_to), N_("If a flying diamond enters the magic wall, it will be turned to this element. Remember that flying diamonds enter the magic wall from its bottom, not from the top!")},
    {"", GD_LABEL, 0, N_("With amoeba")},
    {"MagicWallProperties.convertamoeba", GD_TYPE_BOOLEAN, 0, N_("Stops amoeba"), GetterBase::create_new(&CaveStored::magic_wall_stops_amoeba), N_("When the magic wall is activated, it can convert amoeba into diamonds.")},
    {"MagicWallProperties.breakscan", GD_TYPE_BOOLEAN, 0, N_("BD1 amoeba bug"), GetterBase::create_new(&CaveStored::magic_wall_breakscan), N_("This setting emulates the BD1 bug, where a stone or a diamond falling into a magic wall sometimes caused the active amoeba to convert into a diamond. The rule is: if all amoeba cells above or left to the point where the stone or the diamond falls into the magic wall are enclosed, the amoeba is converted. The timing implications of the bug are not emulated.")},

    /* slime */
    {"", GD_TAB, 0, N_("Slime")},
    {"", GD_LABEL, GD_SHOW_LEVEL_LABEL, N_("Permeability")},
    {"", GD_TYPE_BOOLEAN, GD_DONT_SAVE, N_("Predictable"), GetterBase::create_new(&CaveStored::slime_predictable), N_("Controls if the predictable random generator is used for slime. It is required for compatibility with some older caves.")},
    /* permeabilities are "always" saved; and according to the predictability, one of them is removed "by hand" before saving the file. */
    {"SlimePermeability", GD_TYPE_PROBABILITY_LEVELS, GD_ALWAYS_SAVE, N_("Permeability (unpredictable, %)"), GetterBase::create_new(&CaveStored::level_slime_permeability), N_("This controls the rate at which elements go through the slime. Higher values represent higher probability of passing. This one is for unpredictable slime.")},
    {"SlimePermeabilityC64", GD_TYPE_INT_LEVELS, GD_ALWAYS_SAVE, N_("Permeability (predictable, bits)"), GetterBase::create_new(&CaveStored::level_slime_permeability_c64), N_("This controls the rate at which elements go through the slime. This one is for predictable slime, and the value is used for a bitwise AND function. The values used by the C64 engines are 0, 128, 192, 224, 240, 248, 252, 254 and 255."), 0, 255},
    {"SlimePredictableC64.seed", GD_TYPE_INT_LEVELS, 0, N_("Random seed (predictable)"), GetterBase::create_new(&CaveStored::level_slime_seed_c64), N_("The random number seed for predictable slime. Use -1 to leave on its default. Not recommended to change. Does not affect unpredictable slime."), -1, 65535},
    {"", GD_LABEL, 0, N_("Passing elements")},
    {"SlimeProperties", GD_TYPE_ELEMENT, 0, N_("Eats this..."), GetterBase::create_new(&CaveStored::slime_eats_1), N_("Slime can let other elements than stone and diamond go through. It always lets a waiting or a chasing stone pass, though. Also, flying diamonds and stones, as well as bladders are always passed.")},
    {"SlimeProperties", GD_TYPE_ELEMENT, 0, N_("  ... and converts to"), GetterBase::create_new(&CaveStored::slime_converts_1), N_("Slime can let other elements than stone and diamond go through. It always lets a waiting or a chasing stone pass, though. Also, flying diamonds and stones, as well as bladders are always passed.")},
    {"SlimeProperties", GD_TYPE_ELEMENT, 0, N_("Eats this..."), GetterBase::create_new(&CaveStored::slime_eats_2), N_("Slime can let other elements than stone and diamond go through. It always lets a waiting or a chasing stone pass, though. Also, flying diamonds and stones, as well as bladders are always passed.")},
    {"SlimeProperties", GD_TYPE_ELEMENT, 0, N_("  ... and converts to"), GetterBase::create_new(&CaveStored::slime_converts_2), N_("Slime can let other elements than stone and diamond go through. It always lets a waiting or a chasing stone pass, though. Also, flying diamonds and stones, as well as bladders are always passed.")},
    {"SlimeProperties", GD_TYPE_ELEMENT, 0, N_("Eats this..."), GetterBase::create_new(&CaveStored::slime_eats_3), N_("Slime can let other elements than stone and diamond go through. It always lets a waiting or a chasing stone pass, though. Also, flying diamonds and stones, as well as bladders are always passed.")},
    {"SlimeProperties", GD_TYPE_ELEMENT, 0, N_("  ... and converts to"), GetterBase::create_new(&CaveStored::slime_converts_3), N_("Slime can let other elements than stone and diamond go through. It always lets a waiting or a chasing stone pass, though. Also, flying diamonds and stones, as well as bladders are always passed.")},

    /* ACTIVE 2 */
    {"", GD_TAB, 0, N_("Other elements")},
    /* acid */
    {"", GD_LABEL, 0, N_("Acid")},
    {"AcidProperties", GD_TYPE_ELEMENT, 0, N_("Eats this element"), GetterBase::create_new(&CaveStored::acid_eats_this), N_("The element which acid eats. If it cannot find any, it simply disappears.")},
    {"AcidProperties", GD_TYPE_PROBABILITY, 0, N_("Spread ratio (%)"), GetterBase::create_new(&CaveStored::acid_spread_ratio), N_("The probability at which an acid will explode and eat neighbouring elements.")},
    {"ACIDEffect", GD_TYPE_EFFECT, 0, N_("Leaves this behind"), GetterBase::create_new(&CaveStored::acid_turns_to), N_("If acid converts to an explosion puff on spreading or any other element.")},
    /* biter */
    {"", GD_LABEL, 0, N_("Biter")},
    {"BiterProperties", GD_TYPE_INT, 0, N_("Delay (frame)"), GetterBase::create_new(&CaveStored::biter_delay_frame), N_("Number of frames biters wait between movements."), 0, 3},
    {"BiterProperties", GD_TYPE_ELEMENT, 0, N_("Eats this"), GetterBase::create_new(&CaveStored::biter_eat), N_("Biters eat this element. They always eat dirt.")},
    /* bladder */
    {"", GD_LABEL, 0, N_("Bladder")},
    {"BladderProperties", GD_TYPE_ELEMENT, 0, N_("Converts to clock by touching"), GetterBase::create_new(&CaveStored::bladder_converts_by), NULL},
    /* expanding wall */
    {"", GD_LABEL, 0, N_("Expanding wall")},
    {"ExpandingWallDirection.changed", GD_TYPE_BOOLEAN, 0, N_("Direction changed"), GetterBase::create_new(&CaveStored::expanding_wall_changed), N_("If this option is enabled, the direction of growing for the horizontal and vertical expanding wall is switched. As you can use both horizontal and vertical expanding walls in a cave, it is not recommended to change this setting, as it might be confusing. You should rather select the type with the correct direction from the element box when drawing the cave.")},
    /* replicator */
    {"", GD_LABEL, 0, N_("Replicator")},
    {"ReplicatorActive", GD_TYPE_BOOLEAN, 0, N_("Active at start"), GetterBase::create_new(&CaveStored::replicators_active), N_("Whether the replicators are turned on or off at the cave start.")},
    {"ReplicatorDelayFrame", GD_TYPE_INT, 0, N_("Delay (frame)"), GetterBase::create_new(&CaveStored::replicator_delay_frame), N_("Number of frames to wait between replicating elements."), 0, 100},
    /* conveyor belt */
    {"", GD_LABEL, 0, N_("Conveyor belt")},
    {"ConveyorBeltActive", GD_TYPE_BOOLEAN, 0, N_("Active at start"), GetterBase::create_new(&CaveStored::conveyor_belts_active), N_("Whether the conveyor belts are moving when the cave starts.")},
    {"ConveyorBeltDirection.changed", GD_TYPE_BOOLEAN, 0, N_("Direction changed"), GetterBase::create_new(&CaveStored::conveyor_belts_direction_changed), N_("If the conveyor belts' movement is changed, i.e. they are running in the opposite direction. As you can freely use left and right going versions of the conveyor belt in a cave, it is not recommended to change this setting, rather you should select the correct one from the element box when drawing.")},
    /* water */
    {"", GD_LABEL, 0, N_("Water")},
    {"WaterProperties.doesnotflowdown", GD_TYPE_BOOLEAN, 0, N_("Does not flow downwards"), GetterBase::create_new(&CaveStored::water_does_not_flow_down), N_("In CrDr, the water element had the odd property that it did not flow downwards, only in other directions. This flag emulates this behaviour.")},
    /* nut */
    {"", GD_LABEL, 0, N_("Nut")},
    {"Nut.whencrushed", GD_TYPE_ELEMENT, 0, N_("Turns to when crushed"), GetterBase::create_new(&CaveStored::nut_turns_to_when_crushed), N_("Normally, a nut contains a diamond. If you crush it with a stone, the diamond will appear after the usual nut explosion sequence. This setting can be used to change the element the nut contains.")},

    /* EFFECTS 1 */
    {"", GD_TAB, 0, N_("Effects")},
    /* cave effects */
    {"", GD_LABEL, 0, N_("Stone and diamond effects")},
    {"BOULDERfallingeffect", GD_TYPE_EFFECT, 0, N_("Falling stones convert to"), GetterBase::create_new(&CaveStored::stone_falling_effect), N_("When a stone begins falling, it converts to this element.")},
    {"BOULDERbouncingeffect", GD_TYPE_EFFECT, 0, N_("Bouncing stones convert to"), GetterBase::create_new(&CaveStored::stone_bouncing_effect), N_("When a stone stops falling and rolling, it converts to this element.")},
    {"DIAMONDfallingeffect", GD_TYPE_EFFECT, 0, N_("Falling diamonds convert to"), GetterBase::create_new(&CaveStored::diamond_falling_effect), N_("When a diamond begins falling, it converts to this element.")},
    {"DIAMONDbouncingeffect", GD_TYPE_EFFECT, 0, N_("Bouncing diamonds convert to"), GetterBase::create_new(&CaveStored::diamond_bouncing_effect), N_("When a diamond stops falling and rolling, it converts to this element.")},

    {"", GD_LABEL, 0, N_("Creature explosion effects")},
    {"FireflyExplodeTo", GD_TYPE_ELEMENT, 0, N_("Fireflies explode to"), GetterBase::create_new(&CaveStored::firefly_explode_to), N_("When a firefly explodes, it will create this element. Change this setting wisely. The firefly is a traditional element which is expected to explode to empty space.")},
    {"AltFireflyExplodeTo", GD_TYPE_ELEMENT, 0, N_("Alt. fireflies explode to"), GetterBase::create_new(&CaveStored::alt_firefly_explode_to), N_("When an alternative firefly explodes, it will create this element. Use this setting wisely. Do not create a firefly which explodes to stones, for example: use the stonefly instead.")},
    {"ButterflyExplodeTo", GD_TYPE_ELEMENT, 0, N_("Butterflies explode to"), GetterBase::create_new(&CaveStored::butterfly_explode_to), N_("When a butterfly explodes, it will create this element. Use this setting wisely. Butterflies should explode to diamonds. If you need a creature which explodes to space, use the firefly instead.")},
    {"AltButterflyExplodeTo", GD_TYPE_ELEMENT, 0, N_("Alt. butterflies explode to"), GetterBase::create_new(&CaveStored::alt_butterfly_explode_to), N_("When an alternative butterfly explodes, it will create this element. Use this setting wisely.")},
    {"StoneflyExplodeTo", GD_TYPE_ELEMENT, 0, N_("Stoneflies explode to"), GetterBase::create_new(&CaveStored::stonefly_explode_to), N_("When a stonefly explodes, it will create this element.")},
    {"DragonflyExplodeTo", GD_TYPE_ELEMENT, 0, N_("Dragonflies explode to"), GetterBase::create_new(&CaveStored::dragonfly_explode_to), N_("When a dragonfly explodes, it will create this element.")},

    {"", GD_LABEL, 0, N_("Explosion effects")},
    {"EXPLOSIONEffect", GD_TYPE_EFFECT, 0, N_("Explosions end in"), GetterBase::create_new(&CaveStored::explosion_effect), N_("This element appears in places where an explosion finishes.")},
    {"DIAMONDBIRTHEffect", GD_TYPE_EFFECT, 0, N_("Diamond births end in"), GetterBase::create_new(&CaveStored::diamond_birth_effect), N_("When a diamond birth animation reaches its end, it will leave this element there. This can be used to change the element butterflies explode to.")},
    {"BOMBEXPLOSIONeffect", GD_TYPE_EFFECT, 0, N_("Bombs explosions end in"), GetterBase::create_new(&CaveStored::bomb_explosion_effect), N_("Use this setting to select the element the exploding bomb creates.")},
    {"NITROEXPLOSIONeffect", GD_TYPE_EFFECT, 0, N_("Nitro explosions end in"), GetterBase::create_new(&CaveStored::nitro_explosion_effect), N_("The nitro explosions can create some element other than space.")},

    /* EFFECTS 2 */
    {"", GD_TAB, 0, N_("More effects")},
    /* visual effects */
    {"", GD_LABEL, 0, N_("Visual effects")},
    {"EXPANDINGWALLLOOKSLIKEeffect", GD_TYPE_EFFECT, 0, N_("Expanding wall looks like"), GetterBase::create_new(&CaveStored::expanding_wall_looks_like), N_("This is a compatibility setting for old caves. If you need an expanding wall which looks like steel, you should rather choose the expanding steel wall from the element box.")},
    {"DIRTLOOKSLIKEeffect", GD_TYPE_EFFECT, 0, N_("Dirt looks like"), GetterBase::create_new(&CaveStored::dirt_looks_like), N_("Compatibility setting. Use it wisely! Anything other than Dirt 2 (which can be used to emulate the Dirt Mod) is not recommended.")},

    /* creature effects */
    {"", GD_LABEL, 0, N_("Creature movement")},
    {"EnemyDirectionProperties.startbackwards", GD_TYPE_BOOLEAN, 0, N_("Start backwards"), GetterBase::create_new(&CaveStored::creatures_backwards), N_("Whether the direction creatures travel will already be switched at the cave start.")},
    {"EnemyDirectionProperties.time", GD_TYPE_INT, 0, N_("Automatically turn (s)"), GetterBase::create_new(&CaveStored::creatures_direction_auto_change_time), N_("If this is greater than zero, creatures will automatically change direction in every x seconds."), 0, 999},
    {"EnemyDirectionProperties.changeathatching", GD_TYPE_BOOLEAN, 0, N_("Auto turn on hatching"), GetterBase::create_new(&CaveStored::creatures_direction_auto_change_on_start), N_("If this is set to true, creatures also turn at the start signal. If false, the first change in direction occurs only later.")},
    /* gravity */
    {"", GD_LABEL, 0, N_("Gravity change")},
    {"Gravitation", GD_TYPE_DIRECTION, 0, N_("Direction"), GetterBase::create_new(&CaveStored::gravity), N_("The direction where stones and diamonds fall.")},
    {"GravitationSwitchActive", GD_TYPE_BOOLEAN, 0, N_("Switch active at start"), GetterBase::create_new(&CaveStored::gravity_switch_active), N_("If set to true, the gravity switch will be already activated, when the cave is started, as if a pot has already been collected.")},
    {"SkeletonsForPot", GD_TYPE_INT, 0, N_("Skeletons needed for pot"), GetterBase::create_new(&CaveStored::skeletons_needed_for_pot), N_("The number of skeletons to be collected to be able to use a pot."), 0, 50},
    {"GravitationChangeDelay", GD_TYPE_INT, 0, N_("Gravity switch delay"), GetterBase::create_new(&CaveStored::gravity_change_time), N_("The gravity changes after a while using the gravity switch. This option sets the number of seconds to wait."), 1, 60},

    /* SOUND */
    {"", GD_TAB, 0, N_("Sound")},
    {"", GD_LABEL, 0, N_("Sound for elements")},
    {"Diamond.sound", GD_TYPE_BOOLEAN, 0, N_("Diamond"), GetterBase::create_new(&CaveStored::diamond_sound), N_("If true, falling diamonds will have sound.")},
    {"Stone.sound", GD_TYPE_BOOLEAN, 0, N_("Stone"), GetterBase::create_new(&CaveStored::stone_sound), N_("If true, falling and pushed stones will have sound.")},
    {"Nut.sound", GD_TYPE_BOOLEAN, 0, N_("Nut"), GetterBase::create_new(&CaveStored::nut_sound), N_("If true, falling and cracked nuts have sound.")},
    {"NitroPack.sound", GD_TYPE_BOOLEAN, 0, N_("Nitro pack"), GetterBase::create_new(&CaveStored::nitro_sound), N_("If true, falling and pushed nitro packs will have sound.")},
    {"ExpandingWall.sound", GD_TYPE_BOOLEAN, 0, N_("Expanding wall"), GetterBase::create_new(&CaveStored::expanding_wall_sound), N_("If true, expanding wall will have sound.")},
    {"FallingWall.sound", GD_TYPE_BOOLEAN, 0, N_("Falling wall"), GetterBase::create_new(&CaveStored::falling_wall_sound), N_("If true, falling wall will have sound.")},
    {"AmoebaProperties.sound", GD_TYPE_BOOLEAN, 0, N_("Amoeba"), GetterBase::create_new(&CaveStored::amoeba_sound), N_("Controls if the living amoeba has sound or not.")},
    {"MagicWallProperties.sound", GD_TYPE_BOOLEAN, 0, N_("Magic wall"), GetterBase::create_new(&CaveStored::magic_wall_sound), N_("If true, the activated magic wall will have sound.")},
    {"SlimeProperties.sound", GD_TYPE_BOOLEAN, 0, N_("Slime"), GetterBase::create_new(&CaveStored::slime_sound), N_("If true, the elements passing slime will have sound.")},
    {"LavaProperties.sound", GD_TYPE_BOOLEAN, 0, N_("Lava"), GetterBase::create_new(&CaveStored::lava_sound), N_("If true, the elements sinking in lava will have sound.")},
    {"ReplicatorProperties.sound", GD_TYPE_BOOLEAN, 0, N_("Replicator"), GetterBase::create_new(&CaveStored::replicator_sound), N_("If true, the new element appearing under the replicator will make sound.")},
    {"AcidProperties.sound", GD_TYPE_BOOLEAN, 0, N_("Acid"), GetterBase::create_new(&CaveStored::acid_spread_sound), N_("If true, the acid spreading will have sound.")},
    {"BiterProperties.sound", GD_TYPE_BOOLEAN, 0, N_("Biter"), GetterBase::create_new(&CaveStored::biter_sound), N_("Biters eating something or pushing a stone will have sound.")},
    {"BladderProperties.sound", GD_TYPE_BOOLEAN, 0, N_("Bladder"), GetterBase::create_new(&CaveStored::bladder_sound), N_("Bladders moving and being pushed can have sound.")},
    {"WaterProperties.sound", GD_TYPE_BOOLEAN, 0, N_("Water"), GetterBase::create_new(&CaveStored::water_sound), N_("If true, the cave containing water will have sound.")},
    {"PneumaticHammer.sound", GD_TYPE_BOOLEAN, 0, N_("Pneumatic hammer"), GetterBase::create_new(&CaveStored::pneumatic_hammer_sound), N_("If true, using the pneumatic hammer will have sound.")},
    {"BladderSpender.sound", GD_TYPE_BOOLEAN, 0, N_("Bladder spender"), GetterBase::create_new(&CaveStored::bladder_spender_sound), N_("If true, the bladder spender will make sound, when the bladder appears.")},
    {"BladderConvert.sound", GD_TYPE_BOOLEAN, 0, N_("Bladder convert"), GetterBase::create_new(&CaveStored::bladder_convert_sound), N_("If true, the bladder converting to a clock will make sound.")},
    {"", GD_LABEL, 0, N_("Event sounds")},
    {"GravityChange.sound", GD_TYPE_BOOLEAN, 0, N_("Gravity change"), GetterBase::create_new(&CaveStored::gravity_change_sound), N_("If true, the gravity changing will make sound.")},
    {"EnemyDirectionProperties.sound", GD_TYPE_BOOLEAN, 0, N_("Creature direction change"), GetterBase::create_new(&CaveStored::creature_direction_auto_change_sound), N_("If this is set to true, creatures changing direction will be signaled by a sound.")},

    /* COMPATIBILITY */
    {"", GD_TAB, 0, N_("Compatibility")},
    {"", GD_LABEL, 0, N_("Skeleton")},
    {"SkeletonsWorthDiamonds", GD_TYPE_INT, GD_COMPATIBILITY_SETTING, N_("Skeletons worth diamonds"), GetterBase::create_new(&CaveStored::skeletons_worth_diamonds), N_("The number of diamonds each skeleton is worth. Normally skeletons are used for letting the player use the pot! They are not intended to be used as a second kind of diamond."), 0, 10},
    {"", GD_LABEL, 0, N_("Borders")},
    {"BorderProperties.lineshift", GD_TYPE_BOOLEAN, 0, N_("Line shifting border"), GetterBase::create_new(&CaveStored::lineshift), N_("If this is set to true, the player exiting on either side will appear one row lower or upper on the other side.")},
    {"BorderProperties.objectwraparound", GD_TYPE_BOOLEAN, 0, N_("Objects wrap around"), GetterBase::create_new(&CaveStored::wraparound_objects), N_("If true, objects will wrap around the cave borders as well, i.e. if you drag a line to the left, part of it will appear on the right hand side of the cave. The drawing in this case is also affected by the line shifting border property. If that one is enabled, too, crossing the left hand side or right hand side boundary will decrement or increment the row, and crossing the top or the bottom boundary will have no effect at all.")},
    {"BorderProperties.scan", GD_TYPE_BOOLEAN, 0, N_("Scan first and last row"), GetterBase::create_new(&CaveStored::border_scan_first_and_last), N_("Elements move on first and last row, too. Usually those rows are the border. The games created by the original editor were not allowed to put anything but steel wall there, so it was not apparent that the borders were not processed by the engine. Some old caves need this for compatibility; it is not recommended to change this setting for newly designed caves, though.")},
    {"", GD_LABEL, 0, N_("Other")},
    {"ShortExplosions", GD_TYPE_BOOLEAN, 0, N_("Short explosions"), GetterBase::create_new(&CaveStored::short_explosions), N_("In 1stB and newer engines, explosions were longer, they took five cave frames to complete, as opposed to four frames in the original.")},
    {"GravityAffectsAll", GD_TYPE_BOOLEAN, 0, N_("Gravity change affects everything"), GetterBase::create_new(&CaveStored::gravity_affects_all), N_("If this is enabled, changing the gravity will also affect bladders (moving and pushing), bladder spenders, falling walls and waiting stones. Otherwise, those elements behave as gravity was always pointing downwards. This is a compatibility setting which is not recommended to change. It is intended for imported caves.")},
    {"EXPLOSION3S", GD_TYPE_EFFECT, 0, N_("Explosion stage 3 to"), GetterBase::create_new(&CaveStored::explosion_3_effect), N_("This element appears as the next stage of explosion 3. Not recommended to change. Check explosion effects on the effects page for a better alternative.")},

    /* tags - a tab on its own */
    {"", GD_TAB, 0, N_("Unknown tags")},
    {"", GD_TYPE_LONGSTRING, GD_DONT_SAVE, 0, GetterBase::create_new(&CaveStored::unknown_tags), N_("Tags which were read from the BDCFF, but are not understood by GDash.")},

    {0}  /* end of array */
};

PropertyDescription const CaveStored::color_dialog[] = {
    {"", GD_TAB, 0, N_("Colors")},
    {"", GD_TYPE_COLOR, 0, N_("Border color"), GetterBase::create_new(&CaveStored::colorb), N_("Border color for C64 graphics. Only for compatibility, not used by GDash.")},
    {"", GD_TYPE_COLOR, 0, N_("Background color"), GetterBase::create_new(&CaveStored::color0), N_("Background color for C64 graphics")},
    {"", GD_TYPE_COLOR, 0, N_("Color 1 (dirt)"), GetterBase::create_new(&CaveStored::color1), N_("Foreground color 1 for C64 graphics")},
    {"", GD_TYPE_COLOR, 0, N_("Color 2 (steel wall)"), GetterBase::create_new(&CaveStored::color2), N_("Foreground color 2 for C64 graphics")},
    {"", GD_TYPE_COLOR, 0, N_("Color 3 (brick wall)"), GetterBase::create_new(&CaveStored::color3), N_("Foreground color 3 for C64 graphics")},
    {"", GD_TYPE_COLOR, 0, N_("Amoeba color"), GetterBase::create_new(&CaveStored::color4), N_("Amoeba color for C64 graphics")},
    {"", GD_TYPE_COLOR, 0, N_("Slime color"), GetterBase::create_new(&CaveStored::color5), N_("Slime color for C64 graphics")},
    {0}
};

PropertyDescription const CaveStored::random_dialog[] = {
    {"", GD_TAB, 0, N_("Random fill")},
    /* initial fill */
    {
        "", GD_TYPE_INT_LEVELS, 0, N_("Random seed"), GetterBase::create_new(&CaveStored::level_rand),
        N_("Random seed value controls the predictable random number generator, which fills the cave initially. If set to -1, cave is totally random every time it is played."), -1, GD_CAVE_SEED_MAX
    },
    {"", GD_TYPE_ELEMENT, 0, N_("Initial fill"), GetterBase::create_new(&CaveStored::initial_fill), NULL},
    {"", GD_TYPE_INT, GD_BD_PROBABILITY, N_("Probability 1"), GetterBase::create_new(&CaveStored::random_fill_probability_1), NULL, 0, 255},
    {"", GD_TYPE_ELEMENT, 0, N_("Random fill 1"), GetterBase::create_new(&CaveStored::random_fill_1), NULL},
    {"", GD_TYPE_INT, GD_BD_PROBABILITY, N_("Probability 2"), GetterBase::create_new(&CaveStored::random_fill_probability_2), NULL, 0, 255},
    {"", GD_TYPE_ELEMENT, 0, N_("Random fill 2"), GetterBase::create_new(&CaveStored::random_fill_2), NULL},
    {"", GD_TYPE_INT, GD_BD_PROBABILITY, N_("Probability 3"), GetterBase::create_new(&CaveStored::random_fill_probability_3), NULL, 0, 255},
    {"", GD_TYPE_ELEMENT, 0, N_("Random fill 3"), GetterBase::create_new(&CaveStored::random_fill_3), NULL},
    {"", GD_TYPE_INT, GD_BD_PROBABILITY, N_("Probability 4"), GetterBase::create_new(&CaveStored::random_fill_probability_4), NULL, 0, 255},
    {"", GD_TYPE_ELEMENT, 0, N_("Random fill 4"), GetterBase::create_new(&CaveStored::random_fill_4), NULL},
    {"", GD_TYPE_ELEMENT, 0, N_("Initial border"), GetterBase::create_new(&CaveStored::initial_border), N_("The border which is drawn around the cave, after filling it. You can set it to no element to skip the drawing.")},
    {0}
};


PropertyDescription const CaveStored::cave_statistics_data[] = {
    // TRANSLATORS: short for "number of times played". 5 chars max!
    {"StatPlayed", GD_TYPE_INT_LEVELS, 0, NC_("Statistics", "Play"), GetterBase::create_new(&CaveStored::stat_level_played), NULL},
    // TRANSLATORS: short for "number of times played successfully". 5 chars max!
    {"StatPlayedSuccessfully", GD_TYPE_INT_LEVELS, 0, NC_("Statistics", "Succ"), GetterBase::create_new(&CaveStored::stat_level_played_successfully), NULL},
    // TRANSLATORS: short for "best time". 5 chars max!
    {"StatBestTime", GD_TYPE_INT_LEVELS, 0, NC_("Statistics", "Time"), GetterBase::create_new(&CaveStored::stat_level_best_time), NULL},
    // TRANSLATORS: short for "max diamonds collected". 5 chars max!
    {"StatMostDiamonds", GD_TYPE_INT_LEVELS, 0, NC_("Statistics", "Diam"), GetterBase::create_new(&CaveStored::stat_level_most_diamonds), NULL},
    // TRANSLATORS: short for "max score collected". 5 chars max!
    {"StatHighestScore", GD_TYPE_INT_LEVELS, 0, NC_("Statistics", "Score"), GetterBase::create_new(&CaveStored::stat_level_highest_score), NULL},
    {0}
};



void CaveStored::set_gdash_defaults() {
    /* default data */
    selectable = true;
    intermission = false;
    intermission_instantlife = false;
    intermission_rewardlife = true;
    w = 40;
    h = 22;
    x1 = 0;
    y1 = 0;
    x2 = 39;
    y2 = 21;
    colorb = GdColor::from_c64(0);
    color0 = GdColor::from_c64(0);
    color1 = GdColor::from_c64(8);
    color2 = GdColor::from_c64(11);
    color3 = GdColor::from_c64(1);
    color4 = GdColor::from_c64(5);
    color5 = GdColor::from_c64(6);

    /* difficulty */
    for (unsigned i = 0; i < 5; i++) {
        level_diamonds[i] = 10;
        level_time[i] = 999;
        level_timevalue[i] = i + 1;
        level_ckdelay[i] = 0;
        level_hatching_delay_time[i] = 2;
        level_speed[i] = 200;
        level_hatching_delay_frame[i] = 21;
        level_rand[i] = i;
    }
    diamond_value = 0;
    extra_diamond_value = 0;
    max_time = 999;
    pal_timing = false;
    scheduling = GD_SCHEDULING_MILLISECONDS;

    /* initial fill */
    initial_border = O_STEEL;
    initial_fill = O_DIRT;
    random_fill_1 = O_DIRT;
    random_fill_probability_1 = 0;
    random_fill_2 = O_DIRT;
    random_fill_probability_2 = 0;
    random_fill_3 = O_DIRT;
    random_fill_probability_3 = 0;
    random_fill_4 = O_DIRT;
    random_fill_probability_4 = 0;

    /* PLAYER */
    diagonal_movements = false;
    active_is_first_found = true;
    snap_element = O_SPACE;
    pushing_stone_prob = 250000;
    pushing_stone_prob_sweet = 1000000;
    mega_stones_pushable_with_sweet = false;
    pneumatic_hammer_frame = 5;
    hammered_walls_reappear = false;
    hammered_wall_reappear_frame = 100;
    voodoo_collects_diamonds = false;
    voodoo_disappear_in_explosion = true;
    voodoo_dies_by_stone = false;
    voodoo_any_hurt_kills_player = false;
    for (unsigned i = 0; i < 5; i++) {
        level_bonus_time[i] = 30;
        level_penalty_time[i] = 30;
    }

    /* magic wall */
    for (unsigned i = 0; i < 5; i++)
        level_magic_wall_time[i] = 999;
    magic_diamond_to = O_STONE_F;
    magic_stone_to = O_DIAMOND_F;
    magic_mega_stone_to = O_NITRO_PACK_F;
    magic_nitro_pack_to = O_MEGA_STONE_F;
    magic_nut_to = O_NUT_F;
    magic_flying_stone_to = O_FLYING_DIAMOND_F;
    magic_flying_diamond_to = O_FLYING_STONE_F;
    magic_wall_stops_amoeba = true;
    magic_timer_wait_for_hatching = false;

    /* amoeba */
    amoeba_timer_started_immediately = true;
    amoeba_timer_wait_for_hatching = false;
    for (unsigned i = 0; i < 5; i++) {
        level_amoeba_threshold[i] = 200;
        level_amoeba_time[i] = 999;
    }
    amoeba_growth_prob = 31250;
    amoeba_fast_growth_prob = 250000;
    amoeba_timer_started_immediately = true;
    amoeba_timer_wait_for_hatching = false;
    amoeba_too_big_effect = O_STONE;
    amoeba_enclosed_effect = O_DIAMOND;

    /* amoeba 2 */
    for (unsigned i = 0; i < 5; i++) {
        level_amoeba_2_threshold[i] = 200;
        level_amoeba_2_time[i] = 999;
    }
    amoeba_2_growth_prob = 31250;
    amoeba_2_fast_growth_prob = 250000;
    amoeba_2_too_big_effect = O_STONE;
    amoeba_2_enclosed_effect = O_DIAMOND;
    amoeba_2_explodes_by_amoeba = true;
    amoeba_2_looks_like = O_AMOEBA_2;
    amoeba_2_explosion_effect = O_SPACE;

    /* water */
    water_does_not_flow_down = false;

    /* nut */
    nut_turns_to_when_crushed = O_NUT_CRACK_1;

    /* expanding */
    expanding_wall_changed = false;

    /* replicator */
    replicator_delay_frame = 4;
    replicators_active = true;

    /* conveyor belt */
    conveyor_belts_active = true;
    conveyor_belts_direction_changed = false;

    /* slime */
    slime_predictable = true;
    for (unsigned i = 0; i < 5; i++) {
        level_slime_seed_c64[i] = -1;
        level_slime_permeability_c64[i] = 0;
        level_slime_permeability[i] = 1000000;
    }
    slime_eats_1 = O_DIAMOND;
    slime_converts_1 = O_DIAMOND_F;
    slime_eats_2 = O_STONE;
    slime_converts_2 = O_STONE_F;
    slime_eats_3 = O_NUT;
    slime_converts_3 = O_NUT_F;

    /* acid */
    acid_eats_this = O_DIRT;
    acid_spread_ratio = 31250;
    acid_turns_to = O_EXPLODE_3;

    /* biter */
    biter_delay_frame = 0;
    biter_eat = O_DIAMOND;

    /* bladder */
    bladder_converts_by = O_VOODOO;

    /* SOUND */
    amoeba_sound = true;
    magic_wall_sound = true;
    slime_sound = true;
    lava_sound = true;
    replicator_sound = true;
    acid_spread_sound = true;
    biter_sound = true;
    bladder_sound = true;
    water_sound = true;
    stone_sound = true;
    nut_sound = true;
    diamond_sound = true;
    falling_wall_sound = true;
    expanding_wall_sound = true;
    nitro_sound = true;
    pneumatic_hammer_sound = true;
    bladder_spender_sound = true;
    bladder_convert_sound = true;
    gravity_change_sound = true;
    creature_direction_auto_change_sound = true;

    /* creature effects */
    creatures_backwards = false;
    creatures_direction_auto_change_time = 0;
    creatures_direction_auto_change_on_start = false;
    /* cave effects */
    explosion_effect = O_SPACE;
    explosion_3_effect = O_EXPLODE_4;
    diamond_birth_effect = O_DIAMOND;
    bomb_explosion_effect = O_BRICK;
    nitro_explosion_effect = O_SPACE;
    firefly_explode_to = O_EXPLODE_1;
    alt_firefly_explode_to = O_EXPLODE_1;
    butterfly_explode_to = O_PRE_DIA_1;
    alt_butterfly_explode_to = O_PRE_DIA_1;
    stonefly_explode_to = O_PRE_STONE_1;
    dragonfly_explode_to = O_EXPLODE_1;

    stone_falling_effect = O_STONE_F;
    stone_bouncing_effect = O_STONE;
    diamond_falling_effect = O_DIAMOND_F;
    diamond_bouncing_effect = O_DIAMOND;
    /* visual effects */
    expanding_wall_looks_like = O_BRICK;
    dirt_looks_like = O_DIRT;
    /* gravity */
    gravity = MV_DOWN;
    gravity_switch_active = false;
    skeletons_needed_for_pot = 5;
    gravity_change_time = 10;

    /* COMPATIBILITY */
    border_scan_first_and_last = true;
    lineshift = false;
    wraparound_objects = false;
    short_explosions = true;
    skeletons_worth_diamonds = 0;
    gravity_affects_all = true;

    level_speed[0] = 180;
    level_speed[1] = 160;
    level_speed[2] = 140;
    level_speed[3] = 120;
    level_speed[4] = 120;
};

/// Creates a new CaveStored.
/// Sets GDash defaults for all cave properties. GDash defaults are the same as BDCFF defaults.
CaveStored::CaveStored() {
    set_gdash_defaults();
}


bool CaveStored::has_levels() const {
    PropertyDescription const *prop_desc = get_description_array();
    
    /* if we find any cave variable which has levels AND one of the levels
     * is set to a different value, return "true" as yes we have levels */
    for (unsigned i = 0; prop_desc[i].identifier != NULL; ++i) {
        PropertyDescription const &prop = prop_desc[i];
        switch (prop.type)  {
            case GD_TYPE_BOOLEAN_LEVELS:
                {
                    GdBool const *arr = this->get<GdBoolLevels>(prop.prop);
                    for (unsigned j = 1; j < 5; ++j)
                        if (arr[0] != arr[j])
                            return true;
                }
                break;
            case GD_TYPE_INT_LEVELS:
                {
                    GdInt const *arr = this->get<GdIntLevels>(prop.prop);
                    for (unsigned j = 1; j < 5; ++j)
                        if (arr[0] != arr[j])
                            return true;
                }
                break;
            case GD_TYPE_PROBABILITY_LEVELS:
                {
                    GdProbability const *arr = this->get<GdProbabilityLevels>(prop.prop);
                    for (unsigned j = 1; j < 5; ++j)
                        if (arr[0] != arr[j])
                            return true;
                }
                break;
            default:
                break;
        }
    }
    
    /* now we check the objects. if any of them is neither seenonall nor invisible, it
     * is visible on a specific level only, so return true */
    for (auto it = objects.cbegin();  it != objects.cend(); ++it) {
        CaveObject const & obj = *it;
        if (!obj.is_invisible() && !obj.is_seen_on_all())
            return true;
    }
    
    /* no difference - return false, we have no levels */
    return false;
}
