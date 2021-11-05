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

#ifndef ELEMENTPROPERTIES_HPP_INCLUDED
#define ELEMENTPROPERTIES_HPP_INCLUDED

#include "config.h"

#include "cavetypes.hpp"

/// This enum lists some properties of elements, which are used by the engine.
enum ElementPropertyEnum {
    E_P_SCANNED,                ///< is a scanned element

    E_P_SLOPED_LEFT,            ///< stones and diamonds roll to left on this
    E_P_SLOPED_RIGHT,           ///< stones and diamonds roll to right on this
    E_P_SLOPED_UP,              ///< stones and diamonds roll from up on this
    E_P_SLOPED_DOWN,            ///< stones and diamonds roll from down on this

    E_P_BLADDER_SLOPED,         ///< element acts sloped also for the bladder

    E_P_AMOEBA_CONSUMES,        ///< amoeba can eat this
    E_P_DIRT,                   ///< it is dirt, or something similar (dirt2 or sloped dirt)
    E_P_BLOWS_UP_FLIES,         ///< flies blow up, if they touch this
    E_P_EXPLODES_BY_HIT,        ///< explodes if hit by a stone

    E_P_EXPLOSION_FIRST_STAGE,  ///< set for first stage of every explosion. helps slower/faster explosions changing

    E_P_NON_EXPLODABLE,         ///< selfexplaining
    E_P_CCW,                    ///< this creature has a default counterclockwise rotation (for example, o_fire_1)
    E_P_CAN_BE_HAMMERED,        ///< can be broken by pneumatic hammer
    E_P_CAN_BE_PUSHED,          ///< can be pushed by player
    E_P_VISUAL_EFFECT,          ///< if the element can use a visual effect. used to check consistency of the code
    E_P_PLAYER,                 ///< easier to find out if it is a player element
    E_P_MOVED_BY_CONVEYOR_TOP,      ///< can be moved by conveyor belt
    E_P_MOVED_BY_CONVEYOR_BOTTOM,   ///< can be moved UNDER the conveyor belt
};

/// To be able to combine properties, a bitmask is made.
/// Every property from ElementPropertyEnum should be listed here,
/// with 1<<x. Also here it is possible to combine them.
enum ElementPropertyBitMask {
    P_SCANNED = 1 << E_P_SCANNED,
    P_SLOPED_LEFT = 1 << E_P_SLOPED_LEFT,
    P_SLOPED_RIGHT = 1 << E_P_SLOPED_RIGHT,
    P_SLOPED_UP = 1 << E_P_SLOPED_UP,
    P_SLOPED_DOWN = 1 << E_P_SLOPED_DOWN,
    P_SLOPED = P_SLOPED_LEFT | P_SLOPED_RIGHT | P_SLOPED_UP | P_SLOPED_DOWN, ///< To say "any direction"
    P_BLADDER_SLOPED = 1 << E_P_BLADDER_SLOPED,

    P_AMOEBA_CONSUMES = 1 << E_P_AMOEBA_CONSUMES,
    P_DIRT = 1 << E_P_DIRT,
    P_BLOWS_UP_FLIES = 1 << E_P_BLOWS_UP_FLIES,

    P_EXPLODES_BY_HIT = 1 << E_P_EXPLODES_BY_HIT,
    P_EXPLOSION_FIRST_STAGE = 1 << E_P_EXPLOSION_FIRST_STAGE,

    P_NON_EXPLODABLE = 1 << E_P_NON_EXPLODABLE,
    P_CCW = 1 << E_P_CCW,
    P_CAN_BE_HAMMERED = 1 << E_P_CAN_BE_HAMMERED,
    P_CAN_BE_PUSHED = 1 << E_P_CAN_BE_PUSHED,
    P_VISUAL_EFFECT = 1 << E_P_VISUAL_EFFECT,
    P_PLAYER = 1 << E_P_PLAYER,
    P_MOVED_BY_CONVEYOR_TOP = 1 << E_P_MOVED_BY_CONVEYOR_TOP,
    P_MOVED_BY_CONVEYOR_BOTTOM = 1 << E_P_MOVED_BY_CONVEYOR_BOTTOM,
};

/// Description of a single element.
/// An array of this is made to store properties of all elements.
struct GdElementPorperty {
    GdElementEnum element;      ///< element number, for example O_DIRT. In the array, should be equal to the index of the array item.
    GdElementEnum pair;         ///< the scanned/not scanned pair
    const char *visiblename;    ///< name in editor, for example "Dirt". some have different names than their real engine meaning!
    unsigned int flags;         ///< flags for the engine, like P_SLOPED or P_EXPLODES

    const char *filename;       ///< name in bdcff file, like "DIRT"
    char character;             ///< character representation in bdcff file, like '.'

    int image;                  ///< image in editor (index in cells.png)
    int image_simple;           ///< image for simple view  in editor, and for combo box (index in cells.png)
    int image_game;             ///< image for game. negative if animated

    int ckdelay;                ///< ckdelay ratio - how much time required (in average) for a c64 to process this element - in microseconds.

    char character_new;         ///< character given automatically for elements which don't have one defined in original bdcff description
    std::string lowercase_name; ///< lowercase of translated name. for editor; generated inside the game.
};


extern GdElementPorperty gd_element_properties[];


/// returns true, if the given element is scanned
inline bool is_scanned_element(GdElementEnum e) {
    return (gd_element_properties[e].flags & P_SCANNED) != 0;
}


/// This function converts an element to its scanned pair.
inline GdElementEnum scanned_pair(GdElementEnum of_what) {
    if (gd_element_properties[of_what].flags & P_SCANNED) // already scanned?
        return of_what;
    return gd_element_properties[of_what].pair;
}


/// This function converts an element to its scanned pair.
inline GdElementEnum nonscanned_pair(GdElementEnum of_what) {
    if (!(gd_element_properties[of_what].flags & P_SCANNED)) // already nonscanned?
        return of_what;
    return gd_element_properties[of_what].pair;
}


GdElementEnum gd_element_get_hammered(GdElementEnum elem);


#endif
