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

#ifndef CAVETYPES_HPP_INCLUDED
#define CAVETYPES_HPP_INCLUDED

#include "config.h"

#include <string>


/// This enum lists all types used in GDash cave classes.
/// When adding a new type, check CaveStored::get_has_levels.
enum GdType {
    GD_TYPE_STRING,          ///< A one-line string. A GdString will be used for that.
    GD_TYPE_LONGSTRING,      ///< Multi-line string. Internally also a GdString.
    GD_TYPE_INT,             ///< An integer of type GdInt.
    GD_TYPE_INT_LEVELS,      ///< Array of integers

    GD_TYPE_PROBABILITY,            ///< A probability; internally a GdInt of values 0..1million.
    GD_TYPE_PROBABILITY_LEVELS,     ///< Array of probabilities

    GD_TYPE_BOOLEAN,         ///< A boolean property.
    GD_TYPE_BOOLEAN_LEVELS,     ///< A boolean array for the difficulty levels

    GD_TYPE_COORDINATE,      ///< Coordinate in the cave
    GD_TYPE_ELEMENT,         ///< An element, eg. space, diamond, dirt.
    GD_TYPE_EFFECT,          ///< An effect; internally a GdElement, but stored in the file in a different way.
    GD_TYPE_COLOR,           ///< A color of type GdColor.
    GD_TYPE_DIRECTION,       ///< A direction of movement.
    GD_TYPE_SCHEDULING,      ///< A scheduling type.

    GD_TAB,                  ///< Not a real type, but a notebook tab in the editor.
    GD_LABEL,                ///< Also not a type, but a label in the editor.
};

enum {
    /* these define the number of the cells in the png file */
    NUM_OF_CELLS_X = 8,
    NUM_OF_CELLS_Y = 54,
    /* +80: placeholder for cells which are rendered by the game; for example diamond+arrow = falling diamond */
    NUM_OF_CELLS = NUM_OF_CELLS_X * NUM_OF_CELLS_Y + 80,
};

/**
 * These are the "objects" (cells) in caves.
 *
 * Many of them have a "scanned" pair, which is required by the engine.
 */ 
enum GdElementEnum {
    O_SPACE,
    O_DIRT,
    O_DIRT_SLOPED_UP_RIGHT,
    O_DIRT_SLOPED_UP_LEFT,
    O_DIRT_SLOPED_DOWN_LEFT,
    O_DIRT_SLOPED_DOWN_RIGHT,
    O_DIRT_BALL,
    O_DIRT_BALL_scanned,
    O_DIRT_BALL_F,
    O_DIRT_BALL_F_scanned,
    O_DIRT_LOOSE,
    O_DIRT_LOOSE_scanned,
    O_DIRT_LOOSE_F,
    O_DIRT_LOOSE_F_scanned,
    O_DIRT2,
    O_BRICK,
    O_BRICK_SLOPED_UP_RIGHT,
    O_BRICK_SLOPED_UP_LEFT,
    O_BRICK_SLOPED_DOWN_LEFT,
    O_BRICK_SLOPED_DOWN_RIGHT,
    O_BRICK_NON_SLOPED,
    O_MAGIC_WALL,
    O_PRE_OUTBOX,
    O_OUTBOX,
    O_PRE_INVIS_OUTBOX,
    O_INVIS_OUTBOX,
    O_STEEL,
    O_STEEL_SLOPED_UP_RIGHT,
    O_STEEL_SLOPED_UP_LEFT,
    O_STEEL_SLOPED_DOWN_LEFT,
    O_STEEL_SLOPED_DOWN_RIGHT,
    O_STEEL_EXPLODABLE,
    O_STEEL_EATABLE,
    O_BRICK_EATABLE,

    O_STONE,
    O_STONE_scanned,
    O_STONE_F,
    O_STONE_F_scanned,
    O_FLYING_STONE,
    O_FLYING_STONE_scanned,
    O_FLYING_STONE_F,
    O_FLYING_STONE_F_scanned,
    O_MEGA_STONE,
    O_MEGA_STONE_scanned,
    O_MEGA_STONE_F,
    O_MEGA_STONE_F_scanned,
    O_DIAMOND,
    O_DIAMOND_scanned,
    O_DIAMOND_F,
    O_DIAMOND_F_scanned,
    O_FLYING_DIAMOND,
    O_FLYING_DIAMOND_scanned,
    O_FLYING_DIAMOND_F,
    O_FLYING_DIAMOND_F_scanned,
    O_NUT,
    O_NUT_scanned,
    O_NUT_F,
    O_NUT_F_scanned,

    O_BLADDER_SPENDER,
    O_INBOX,

    O_H_EXPANDING_WALL,
    O_H_EXPANDING_WALL_scanned,
    O_V_EXPANDING_WALL,
    O_V_EXPANDING_WALL_scanned,
    O_EXPANDING_WALL,
    O_EXPANDING_WALL_scanned,
    O_H_EXPANDING_STEEL_WALL,
    O_H_EXPANDING_STEEL_WALL_scanned,
    O_V_EXPANDING_STEEL_WALL,
    O_V_EXPANDING_STEEL_WALL_scanned,
    O_EXPANDING_STEEL_WALL,
    O_EXPANDING_STEEL_WALL_scanned,

    O_EXPANDING_WALL_SWITCH,
    O_CREATURE_SWITCH,
    O_BITER_SWITCH,
    O_REPLICATOR_SWITCH,
    O_CONVEYOR_SWITCH,
    O_CONVEYOR_DIR_SWITCH,

    O_ACID,
    O_ACID_scanned,
    O_FALLING_WALL,
    O_FALLING_WALL_F,
    O_FALLING_WALL_F_scanned,

    O_BOX,
    O_TIME_PENALTY,
    O_GRAVESTONE,
    O_STONE_GLUED,
    O_DIAMOND_GLUED,
    O_DIAMOND_KEY,
    O_TRAPPED_DIAMOND,
    O_CLOCK,
    O_DIRT_GLUED,
    O_KEY_1,
    O_KEY_2,
    O_KEY_3,
    O_DOOR_1,
    O_DOOR_2,
    O_DOOR_3,

    O_POT,
    O_GRAVITY_SWITCH,
    O_PNEUMATIC_HAMMER,
    O_TELEPORTER,
    O_SKELETON,
    O_WATER,
    O_WATER_1,
    O_WATER_2,
    O_WATER_3,
    O_WATER_4,
    O_WATER_5,
    O_WATER_6,
    O_WATER_7,
    O_WATER_8,
    O_WATER_9,
    O_WATER_10,
    O_WATER_11,
    O_WATER_12,
    O_WATER_13,
    O_WATER_14,
    O_WATER_15,
    O_WATER_16,
    O_COW_1,
    O_COW_2,
    O_COW_3,
    O_COW_4,
    O_COW_1_scanned,
    O_COW_2_scanned,
    O_COW_3_scanned,
    O_COW_4_scanned,
    O_COW_ENCLOSED_1,
    O_COW_ENCLOSED_2,
    O_COW_ENCLOSED_3,
    O_COW_ENCLOSED_4,
    O_COW_ENCLOSED_5,
    O_COW_ENCLOSED_6,
    O_COW_ENCLOSED_7,
    O_WALLED_DIAMOND,
    O_WALLED_KEY_1,
    O_WALLED_KEY_2,
    O_WALLED_KEY_3,

    O_AMOEBA,
    O_AMOEBA_scanned,
    O_AMOEBA_2,
    O_AMOEBA_2_scanned,
    O_REPLICATOR,
    O_CONVEYOR_LEFT,
    O_CONVEYOR_RIGHT,
    O_LAVA,
    O_SWEET,
    O_VOODOO,
    O_SLIME,
    O_BLADDER,
    O_BLADDER_1,
    O_BLADDER_2,
    O_BLADDER_3,
    O_BLADDER_4,
    O_BLADDER_5,
    O_BLADDER_6,
    O_BLADDER_7,
    O_BLADDER_8,

    O_WAITING_STONE,
    O_WAITING_STONE_scanned,
    O_CHASING_STONE,
    O_CHASING_STONE_scanned,
    O_GHOST,
    O_GHOST_scanned,
    O_FIREFLY_1,
    O_FIREFLY_2,
    O_FIREFLY_3,
    O_FIREFLY_4,
    O_FIREFLY_1_scanned,
    O_FIREFLY_2_scanned,
    O_FIREFLY_3_scanned,
    O_FIREFLY_4_scanned,
    O_ALT_FIREFLY_1,
    O_ALT_FIREFLY_2,
    O_ALT_FIREFLY_3,
    O_ALT_FIREFLY_4,
    O_ALT_FIREFLY_1_scanned,
    O_ALT_FIREFLY_2_scanned,
    O_ALT_FIREFLY_3_scanned,
    O_ALT_FIREFLY_4_scanned,
    O_BUTTER_1,
    O_BUTTER_2,
    O_BUTTER_3,
    O_BUTTER_4,
    O_BUTTER_1_scanned,
    O_BUTTER_2_scanned,
    O_BUTTER_3_scanned,
    O_BUTTER_4_scanned,
    O_ALT_BUTTER_1,
    O_ALT_BUTTER_2,
    O_ALT_BUTTER_3,
    O_ALT_BUTTER_4,
    O_ALT_BUTTER_1_scanned,
    O_ALT_BUTTER_2_scanned,
    O_ALT_BUTTER_3_scanned,
    O_ALT_BUTTER_4_scanned,
    O_STONEFLY_1,
    O_STONEFLY_2,
    O_STONEFLY_3,
    O_STONEFLY_4,
    O_STONEFLY_1_scanned,
    O_STONEFLY_2_scanned,
    O_STONEFLY_3_scanned,
    O_STONEFLY_4_scanned,
    O_BITER_1,
    O_BITER_2,
    O_BITER_3,
    O_BITER_4,
    O_BITER_1_scanned,
    O_BITER_2_scanned,
    O_BITER_3_scanned,
    O_BITER_4_scanned,
    O_DRAGONFLY_1,
    O_DRAGONFLY_2,
    O_DRAGONFLY_3,
    O_DRAGONFLY_4,
    O_DRAGONFLY_1_scanned,
    O_DRAGONFLY_2_scanned,
    O_DRAGONFLY_3_scanned,
    O_DRAGONFLY_4_scanned,

    O_PRE_PL_1,
    O_PRE_PL_2,
    O_PRE_PL_3,
    O_PLAYER,
    O_PLAYER_scanned,
    O_PLAYER_BOMB,
    O_PLAYER_BOMB_scanned,
    O_PLAYER_ROCKET_LAUNCHER,
    O_PLAYER_ROCKET_LAUNCHER_scanned,
    O_PLAYER_GLUED,
    O_PLAYER_STIRRING,

    O_ROCKET_LAUNCHER,
    O_ROCKET_1,
    O_ROCKET_1_scanned,
    O_ROCKET_2,
    O_ROCKET_2_scanned,
    O_ROCKET_3,
    O_ROCKET_3_scanned,
    O_ROCKET_4,
    O_ROCKET_4_scanned,

    O_BOMB,
    O_BOMB_TICK_1,
    O_BOMB_TICK_2,
    O_BOMB_TICK_3,
    O_BOMB_TICK_4,
    O_BOMB_TICK_5,
    O_BOMB_TICK_6,
    O_BOMB_TICK_7,

    O_NITRO_PACK,
    O_NITRO_PACK_scanned,
    O_NITRO_PACK_F,
    O_NITRO_PACK_F_scanned,
    O_NITRO_PACK_EXPLODE,
    O_NITRO_PACK_EXPLODE_scanned,

    O_PRE_CLOCK_0,
    O_PRE_CLOCK_1,
    O_PRE_CLOCK_2,
    O_PRE_CLOCK_3,
    O_PRE_CLOCK_4,
    O_PRE_DIA_0,
    O_PRE_DIA_1,
    O_PRE_DIA_2,
    O_PRE_DIA_3,
    O_PRE_DIA_4,
    O_PRE_DIA_5,
    O_EXPLODE_0,
    O_EXPLODE_1,
    O_EXPLODE_2,
    O_EXPLODE_3,
    O_EXPLODE_4,
    O_EXPLODE_5,
    O_PRE_STONE_0,
    O_PRE_STONE_1,
    O_PRE_STONE_2,
    O_PRE_STONE_3,
    O_PRE_STONE_4,
    O_PRE_STEEL_0,
    O_PRE_STEEL_1,
    O_PRE_STEEL_2,
    O_PRE_STEEL_3,
    O_PRE_STEEL_4,
    O_GHOST_EXPL_0,
    O_GHOST_EXPL_1,
    O_GHOST_EXPL_2,
    O_GHOST_EXPL_3,
    O_GHOST_EXPL_4,
    O_BOMB_EXPL_0,
    O_BOMB_EXPL_1,
    O_BOMB_EXPL_2,
    O_BOMB_EXPL_3,
    O_BOMB_EXPL_4,
    O_NITRO_EXPL_0,
    O_NITRO_EXPL_1,
    O_NITRO_EXPL_2,
    O_NITRO_EXPL_3,
    O_NITRO_EXPL_4,
    O_AMOEBA_2_EXPL_0,
    O_AMOEBA_2_EXPL_1,
    O_AMOEBA_2_EXPL_2,
    O_AMOEBA_2_EXPL_3,
    O_AMOEBA_2_EXPL_4,
    O_NUT_CRACK_0,
    O_NUT_CRACK_1,
    O_NUT_CRACK_2,
    O_NUT_CRACK_3,
    O_NUT_CRACK_4,

    // these are used internally for the pneumatic hammer, and should not be used in the editor!
    // (not even as an effect destination or something like that)
    O_PLAYER_PNEUMATIC_LEFT,
    O_PLAYER_PNEUMATIC_RIGHT,
    O_PNEUMATIC_ACTIVE_LEFT,
    O_PNEUMATIC_ACTIVE_RIGHT,

    /** unknown element imported or read from bdcff */
    O_UNKNOWN,
    /** a "do not draw this" element used when creating the cave. can be used, for example, to skip drawing a maze's path */
    O_NONE,

    /** last index of elements which can be in the cave. the elements below only help simplifying some drawing routines. */
    O_MAX,

    // fake elements to help drawing of a cave
    O_FAKE_BONUS,
    O_OUTBOX_CLOSED,
    O_OUTBOX_OPEN,
    O_COVERED,
    O_PLAYER_LEFT,
    O_PLAYER_RIGHT,
    O_PLAYER_UP,
    O_PLAYER_DOWN,
    O_PLAYER_TAP,
    O_PLAYER_BLINK,
    O_PLAYER_TAP_BLINK,
    O_PLAYER_PUSH_LEFT,
    O_PLAYER_PUSH_RIGHT,
    O_CREATURE_SWITCH_ON,
    O_EXPANDING_WALL_SWITCH_HORIZ,
    O_EXPANDING_WALL_SWITCH_VERT,
    O_GRAVITY_SWITCH_ACTIVE,
    O_REPLICATOR_SWITCH_ON,
    O_REPLICATOR_SWITCH_OFF,
    O_CONVEYOR_DIR_NORMAL,
    O_CONVEYOR_DIR_CHANGED,
    O_CONVEYOR_SWITCH_OFF,
    O_CONVEYOR_SWITCH_ON,

    // fake element for the help window, to show the player without an exclamation mark
    O_PLAYER_HELP,

    // fake elements - arrows, quiestion mark & the like, which are drawn by the editor
    O_QUESTION_MARK,
    O_EATABLE,
    O_DOWN_ARROW,
    O_LEFTRIGHT_ARROW,
    O_EVERYDIR_ARROW,
    O_GLUED,
    O_OUT,
    O_EXCLAMATION_MARK,
    
    O_MAX_INDEX,
};


/// Enumerates directions of movement.
enum GdDirectionEnum {
    MV_STILL,               ///< No movement.

    /* directions */
    MV_UP,
    MV_UP_RIGHT,
    MV_RIGHT,
    MV_DOWN_RIGHT,
    MV_DOWN,
    MV_DOWN_LEFT,
    MV_LEFT,
    MV_UP_LEFT,

    MV_MAX,                 ///< To have an index of normal movements

    MV_UP_2 = MV_MAX,       ///< Move up, two cells.
    MV_UP_RIGHT_2,          ///< Move up&right, two cells.
    MV_RIGHT_2,
    MV_DOWN_RIGHT_2,
    MV_DOWN_2,
    MV_DOWN_LEFT_2,
    MV_LEFT_2,
    MV_UP_LEFT_2,
};

/// Enumerates known schedulings.
enum GdSchedulingEnum {
    GD_SCHEDULING_MILLISECONDS,         ///< Perfect scheduling, milliseconds-based.
    GD_SCHEDULING_BD1,                  ///< C64 BD1
    GD_SCHEDULING_BD2,                  ///< C64 BD2
    GD_SCHEDULING_PLCK,                 ///< C64 construction kit
    GD_SCHEDULING_CRDR,                 ///< C64 crazy dream
    GD_SCHEDULING_BD1_ATARI,            ///< Atari BD1
    GD_SCHEDULING_BD2_PLCK_ATARI,       ///< Atari BD2 and construction kit

    GD_SCHEDULING_MAX                   ///< Number of possible movements
};

/// Engine types; used in BDCFF Engine= line to set some defaults.
enum GdEngineEnum {
    GD_ENGINE_BD1,
    GD_ENGINE_BD2,
    GD_ENGINE_PLCK,
    GD_ENGINE_1STB,
    GD_ENGINE_CRDR7,
    GD_ENGINE_CRLI,

    GD_ENGINE_MAX,
};

/// A template of a wrapper class around plain old data (int, bool, enum) variables.
/// Creates a constructor which initializes them to 0. All functions are inlined.
template <typename T>
class PlainOldData {
private:
    T value;
public:
    PlainOldData(const T &value = T()): value(value) {}
    operator T &() {
        return value;
    }
    operator const T &() const {
        return value;
    }
};

/// A boolean stored in a cave.
typedef PlainOldData<bool> GdBool;
/// An array of bools for cave difficulty levels.
typedef GdBool GdBoolLevels[5];
/// An integer stored in a cave.
typedef PlainOldData<int> GdInt;
/// An array of integers for the different cave levels.
typedef GdInt GdIntLevels[5];
/// A cave element stored in a cave.
typedef PlainOldData<GdElementEnum> GdElement;
/// A scheduling type stored in a cave.
typedef PlainOldData<GdSchedulingEnum> GdScheduling;
/// A direction stored in a cave.
typedef PlainOldData<GdDirectionEnum> GdDirection;
/// A string stored in a cave. Strong alias of std::string.
class GdString: public std::string {
public:
    GdString &operator=(const char *x) {
        std::string::operator=(x);
        return *this;
    }
    GdString &operator=(const std::string &x) {
        std::string::operator=(x);
        return *this;
    }
};
/// A probability between 0.000 and 1.000.
/// It is stored as an integer between 0 and 1000000; the in-file representation is a double.
/// A different class (not simply a typedef to the plainolddata<int>), so it can be
/// overloaded for operator<<.
class GdProbability: private PlainOldData<int> {
public:
    GdProbability() = default;
    GdProbability(const int &value): PlainOldData<int>(value) {}
    using PlainOldData<int>::operator int &;
    using PlainOldData<int>::operator const int &;
};
/// An array of probabilities for cave difficulty levels.
typedef GdProbability GdProbabilityLevels[5];
/// Used only for BDCFF reading.
typedef PlainOldData<GdEngineEnum> GdEngine;


/// A primary building block for objects in a cave: this class represents a coordinate.
class Coordinate {
public:
    GdInt x;    ///< Horizontal (x) coordinate
    GdInt y;    ///< Vertical (y) coordinate

    /// Default constructor: create a coordinate of (0,0).
    Coordinate() : x(0), y(0) {}

    /// Create a coordinate of (x,y).
    Coordinate(int x, int y) : x(x), y(y) {}

    /// Add a vector to a coordinate.
    /// @param p The vector to add.
    Coordinate &operator+=(Coordinate rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    /// Add two coordinates (vectors).
    Coordinate operator+(Coordinate rhs) const {
        return Coordinate(x + rhs.x, y + rhs.y);
    }

    /// Compare two coordinates for equality.
    /// @return True, if they are the same.
    bool operator==(Coordinate rhs) const {
        return x == rhs.x && y == rhs.y;
    }
    static void drag_rectangle(Coordinate &p1, Coordinate &p2, Coordinate current, Coordinate displacement);
};


/// Class which helps cave map to BDCFF conversions.
class CharToElementTable {
    // table to hold char->element; 0..127 ascii values
    enum { ArraySize = 128 };
    GdElementEnum table[ArraySize];
public:
    CharToElementTable();
    GdElementEnum get(unsigned i) const;
    unsigned find_place_for(GdElementEnum e);
    void set(unsigned i, GdElementEnum e);
};

/// Initialize the GDash caves type system. Must be called at program start.
void gd_cave_types_init();

// These istream operators are needed by the bdcff reader to read some object types.
std::istream &operator>>(std::istream &is, GdElementEnum &e);
std::istream &operator>>(std::istream &is, Coordinate &p);

std::ostream &operator<<(std::ostream &os, GdBool const &b);
std::ostream &operator<<(std::ostream &os, GdInt const &i);
std::ostream &operator<<(std::ostream &os, GdProbability const &p);
std::ostream &operator<<(std::ostream &os, GdScheduling const &s);
std::ostream &operator<<(std::ostream &os, GdDirection const &d);
std::ostream &operator<<(std::ostream &os, GdElement const &s);
std::ostream &operator<<(std::ostream &os, GdEngine const &s);
std::ostream &operator<<(std::ostream &os, Coordinate const &p);

bool read_from_string(const std::string &s, GdBool &b);
bool read_from_string(const std::string &s, GdInt &i);
bool read_from_string(const std::string &s, GdInt &i, double conversion_ratio);
bool read_from_string(const std::string &s, GdProbability &i);
bool read_from_string(const std::string &s, GdDirection &d);
bool read_from_string(const std::string &s, GdScheduling &sch);
bool read_from_string(const std::string &s, GdElement &e);
bool read_from_string(const std::string &s, GdEngine &e);

const char *visible_name(GdBool const &b);
std::string visible_name(GdInt const &i);
std::string visible_name(GdProbability const &p);
const char *visible_name(GdSchedulingEnum sched);
const char *visible_name(GdDirectionEnum dir);
const char *visible_name(GdElementEnum elem);
const char *visible_name_lowercase(GdElementEnum elem);
std::string visible_name_no_attribute(GdElementEnum elem);
const char *visible_name(GdEngineEnum eng);
std::string visible_name(Coordinate const &p);

#endif
