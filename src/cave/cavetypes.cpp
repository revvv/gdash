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
#include <sstream>
#include <glib/gi18n.h>
#include <cstdlib>
#include <iomanip>

#include "cave/cavetypes.hpp"
#include "cave/helper/namevaluepair.hpp"
#include "cave/elementproperties.hpp"
#include "misc/logger.hpp"


/* TRANSLATORS: None here means "no direction to move"; when there is no gravity while stirring the pot. */
static const char *direction_name[] = { N_("None"), N_("Up"), N_("Up+right"), N_("Right"), N_("Down+right"), N_("Down"), N_("Down+left"), N_("Left"), N_("Up+left") };
static const char *direction_filename[] = { "none", "up", "upright", "right", "downright", "down", "downleft", "left", "upleft" };
static_assert(MV_MAX == G_N_ELEMENTS(direction_filename));
static_assert(MV_MAX == G_N_ELEMENTS(direction_name));

static const char *scheduling_name[] = { N_("Milliseconds"), "BD1", "BD2", "Construction Kit", "Crazy Dream 7", "Atari BD1", "Atari BD2/Construction Kit" };
static const char *scheduling_filename[] = { "ms", "bd1", "bd2", "plck", "crdr7", "bd1atari", "bd2ckatari" };
static_assert(GD_SCHEDULING_MAX == G_N_ELEMENTS(scheduling_filename));
static_assert(GD_SCHEDULING_MAX == G_N_ELEMENTS(scheduling_name));

/* used for bdcff engine flag. */
static const char *engines_name[] = {"BD1", "BD2", "PLCK", "1stB", "Crazy Dream", "Crazy Light"};
static const char *engines_filename[] = {"BD1", "BD2", "PLCK", "1stB", "CrDr", "CrLi"};
static_assert(GD_ENGINE_MAX == G_N_ELEMENTS(engines_filename));
static_assert(GD_ENGINE_MAX == G_N_ELEMENTS(engines_name));

/// Write a coordinate to an output stream.
/// Delimits the x and y components with space.
std::ostream &operator<<(std::ostream &os, Coordinate const &p) {
    return (os << p.x << ' ' << p.y);
}

/// Read a coordinate from an input stream.
/// Reads x and y coordinates; if both could be read, set p.
std::istream &operator>>(std::istream &is, Coordinate &p) {
    int x, y;
    is >> x >> y;
    /* only modify p if read both parameters correctly */
    if (is) {
        p.x = x;
        p.y = y;
    }
    return is;
}

/// Get on-screen description of a coordinate.
std::string visible_name(Coordinate const &p) {
    std::ostringstream os;
    os << '(' << p.x << ',' << p.y << ')';
    return os.str();
}

/// Drag the corners of the rectangle set by p1 and p2.
/// This is used for many objects in the editor. When clicking and dragging
/// one of the corners of a square, its size can be changed.
/// Whereas by clicking and dragging the edges, the whole square is moved.
/// We can detect clicking on the edges by comparing the coordinate clicked
/// with p1 and p2; all four possibilities have to be taken into account.
/// @param p1 One corner of the rectangle.
/// @param p2 The other corner of the rectangle.
/// @param current The coordinate clicked.
/// @param displacement The movement vector.
void Coordinate::drag_rectangle(Coordinate &p1, Coordinate &p2, Coordinate current, Coordinate displacement) {
    /* dragging objects which are box-shaped */
    if (current.x == p1.x && current.y == p1.y) {       /* try to drag (x1;y1) corner. */
        p1.x += displacement.x;
        p1.y += displacement.y;
    } else if (current.x == p2.x && current.y == p1.y) { /* try to drag (x2;y1) corner. */
        p2.x += displacement.x;
        p1.y += displacement.y;
    } else if (current.x == p1.x && current.y == p2.y) { /* try to drag (x1;y2) corner. */
        p1.x += displacement.x;
        p2.y += displacement.y;
    } else if (current.x == p2.x && current.y == p2.y) { /* try to drag (x2;y2) corner. */
        p2.x += displacement.x;
        p2.y += displacement.y;
    } else {
        /* drag the whole thing */
        p1.x += displacement.x;
        p1.y += displacement.y;
        p2.x += displacement.x;
        p2.y += displacement.y;
    }
}


/// get on-screen visible "name" of an int
std::string visible_name(GdInt const &i) {
    std::ostringstream os;
    os << i;
    return os.str();
}

/// get on-screen visible "name" of a probability
/// @todo change 1000000.0 to a constant EVERYWHERE in the code
/// @todo check everywhere when reading and writing if a +0.5 is needed, and explain why
std::string visible_name(GdProbability const &p) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(2) << p * 100.0 / 1000000.0 << '%';
    return os.str();
}

/// get on-screen visible "name" of a bool. Already translated to the user's language.
const char *visible_name(GdBool const &b) {
    return b ? _("Yes") : _("No");
}

/// get on-screen visible name of a direction. Already translated to the user's language.
const char *visible_name(GdDirectionEnum dir) {
    g_assert(dir >= 0 && unsigned(dir) < G_N_ELEMENTS(direction_name));
    return _(direction_name[dir]);
}

/// get on-screen visible name of a scheduling. Already translated to the user's language.
const char *visible_name(GdSchedulingEnum sched) {
    g_assert(sched >= 0 && unsigned(sched) < G_N_ELEMENTS(scheduling_name));
    return _(scheduling_name[sched]);
}

/// get on-screen visible name of an engine. Already translated to the user's language.
const char *visible_name(GdEngineEnum eng) {
    g_assert(eng >= 0 && unsigned(eng) < G_N_ELEMENTS(engines_name));
    return engines_name[eng];
}

/// Get on-screen visible name of an element. Already translated to the user's language.
const char *visible_name(GdElementEnum elem) {
    return _(gd_element_properties[elem].visiblename);
}

/// Get on-screen visible name of an element, but made lowercased, so it can be put in a sentence.
const char *visible_name_lowercase(GdElementEnum elem) {
    /* TRANSLATORS: in some languages (for example, German) nouns must be written capitalized.
     * In other languages, nouns can be written uncapitalized.
     * When the name of an element is put in a sentence, this has to be taken into account.
     *
     * For example, the element name in English is "Brick wall",
     * and it is possible to write "Line of brick wall" (uppercase B -> lowercase b in the sentence).
     * In German, the same is "Ziegelmauer", and "Linie aus Ziegelmauer" (Z remains in upper case).
     *
     * If the language you are translating to writes nouns capitalized (for example, German),
     * translate this string to "lowercase-element-names-no".
     * Otherwise translate this to "lowercase-element-values-yes".
     */
    if (gd_str_equal(gettext("lowercase-element-names-yes"), "lowercase-element-names-no")) {
        /* no lowercase names, simply return the original visible name (translated of course) */
        return visible_name(elem);
    }
    /* lowercase names used - create the translated one if needed, and make it lowercase */
    if (gd_element_properties[elem].visiblename && gd_element_properties[elem].lowercase_name.empty()) {
        gd_element_properties[elem].lowercase_name =
            gd_tostring_free(g_utf8_strdown(visible_name(elem), -1));
    }
    return gd_element_properties[elem].lowercase_name.c_str();
}


/// This function returns the translated name of the element, but the
/// attribute that might be present is stripped.
/// For example, if the original name is "Firefly, left", then the
/// function will return "Firefly".
std::string visible_name_no_attribute(GdElementEnum elem) {
    char **words = g_strsplit_set(visible_name(elem), ",(", -1);
    g_strstrip(words[0]);
    std::string ret = words[0];
    g_strfreev(words);
    return ret;
}


/// Creates a CharToElementTable for conversion.
/// Adds all fixed elements, read from the gd_element_properties array.
CharToElementTable::CharToElementTable() {
    for (unsigned i = 0; i < ArraySize; i++)
        table[i] = O_UNKNOWN;

    /* then set fixed characters */
    for (unsigned i = 0; i < O_MAX; i++) {
        int c = gd_element_properties[i].character;

        if (c != 0) {
            /* check if already used for element */
            g_assert(table[c] == O_UNKNOWN);
            table[c] = GdElementEnum(i);
        }
    }
}

/**
 * @brief Return the GdElementEnum assigned to the character.
 *
 * @param i The character.
 * @return The element, or O_UNKNOWN if character is invalid.
 */
GdElementEnum CharToElementTable::get(unsigned i) const {
    if (i >= ArraySize || table[i] == O_UNKNOWN) {
        gd_warning("Invalid character representing element: %c", char(i));
        return O_UNKNOWN;
    }
    return table[i];
}

/**
 * @brief Find an empty character to store the element in a map.
 * If finds a suitable character, also remembers.
 *
 * @param e The element to find place for.
 * @return The (new) character for the element.
 */
unsigned CharToElementTable::find_place_for(GdElementEnum e) {
    const char *not_allowed = "<>&[]/=\\";

    // first check if it is already in the array.
    for (unsigned i = 32; i < ArraySize; ++i)
        if (table[i] == e)
            return i;

    unsigned i;
    for (i = 32; i < ArraySize; ++i)
        // if found a good empty char, break
        if (table[i] == O_UNKNOWN && strchr(not_allowed, i) == NULL)
            break;
    if (i >= ArraySize)
        throw std::runtime_error("no more characters");
    table[i] = e;
    return i;
}

/**
 * @brief Set an element assigned to a character.
 *
 * @param i The character.
 * @param e The element assigned.
 */
void CharToElementTable::set(unsigned i, GdElementEnum e) {
    if (i >= ArraySize) {
        gd_warning("Invalid character representing element: %c", char(i));
        return;
    }

    if (table[i] != O_UNKNOWN)
        gd_warning("Character %c already used by elements %s", char(i), visible_name(table[i]));

    table[i] = e;
}

static NameValuePair<GdElementEnum> name_to_element;

void gd_cave_types_init() {
    /* put names to a hash table */
    /* this is a helper for file read operations */

    for (int i = 0; i < O_MAX; i++) {
        const char *key = gd_element_properties[i].filename;

        g_assert(key != NULL && !gd_str_equal(key, ""));
        /* check if every name is used once */
        g_assert(!name_to_element.has_name(key));
        name_to_element.add(key, GdElementEnum(i));
    }
    /* for compatibility with tim stridmann's memorydump->bdcff converter... .... ... */
    name_to_element.add("HEXPANDING_WALL", O_H_EXPANDING_WALL);
    name_to_element.add("FALLING_DIAMOND", O_DIAMOND_F);
    name_to_element.add("FALLING_BOULDER", O_STONE_F);
    name_to_element.add("EXPLOSION1S", O_EXPLODE_1);
    name_to_element.add("EXPLOSION2S", O_EXPLODE_2);
    name_to_element.add("EXPLOSION3S", O_EXPLODE_3);
    name_to_element.add("EXPLOSION4S", O_EXPLODE_4);
    name_to_element.add("EXPLOSION5S", O_EXPLODE_5);
    name_to_element.add("EXPLOSION1D", O_PRE_DIA_1);
    name_to_element.add("EXPLOSION2D", O_PRE_DIA_2);
    name_to_element.add("EXPLOSION3D", O_PRE_DIA_3);
    name_to_element.add("EXPLOSION4D", O_PRE_DIA_4);
    name_to_element.add("EXPLOSION5D", O_PRE_DIA_5);
    name_to_element.add("WALL2", O_STEEL_EXPLODABLE);
    /* compatibility with old bd-faq (pre disassembly of bladder) */
    name_to_element.add("BLADDERd9", O_BLADDER_8);

    /* create table to show errors at the start of the application */
    CharToElementTable _ctet;

    /* check element database for faults. */
    for (int i = 0; gd_element_properties[i].element != O_MAX_INDEX; i++) {
        g_assert(gd_element_properties[i].element == i);
        /* game pixbuf should not use (generated) editor pixbuf */
        g_assert(abs(gd_element_properties[i].image_game) < NUM_OF_CELLS_X * NUM_OF_CELLS_Y);
        /* editor pixbuf should not be animated */
        g_assert(gd_element_properties[i].image >= 0);
        if (gd_element_properties[i].flags & P_CAN_BE_HAMMERED)
            g_assert(gd_element_get_hammered(GdElementEnum(i)) != O_NONE);

        /* if its pair is not the same as itself, it is a scanned pair. */
        if (gd_element_properties[i].pair != i) {
            /* check if it has correct scanned pair, a->b, b->a */
            g_assert(gd_element_properties[gd_element_properties[i].pair].pair == i);
            if (gd_element_properties[i].flags & P_SCANNED) {
                /* if this one is the scanned */
                /* check if non-scanned pair is not tagged as scanned */
                g_assert((gd_element_properties[gd_element_properties[i].pair].flags & P_SCANNED) == 0);
                /* check if no ckdelay */
                g_assert(gd_element_properties[i].ckdelay == 0);
            } else if (gd_element_properties[gd_element_properties[i].pair].flags & P_SCANNED) {
                /* if this one is the non-scanned */
                g_assert((gd_element_properties[gd_element_properties[i].pair].flags & P_SCANNED) != 0);
            } else {
                /* scan pair - one of them should be scanned */
                g_assert_not_reached();
            }
        }
    }
}

/// Load an element from a stream, where it is stored in its name.
/// If loading fails, the stream is set to an error state.
/// If the element name is not found, an error state is also set.
/// @param is The istream to load from.
/// @param e The element to store to.
std::istream &operator>>(std::istream &is, GdElementEnum &e) {
    std::string s;
    if (is >> s) {
        if (!name_to_element.has_name(s))
            is.setstate(std::ios::failbit);
        else
            e = name_to_element.lookup_name(s);
    }
    return is;
}

/// Save a GdBool to a stream, by writing either "false" or "true".
std::ostream &operator<<(std::ostream &os, GdBool const &b) {
    os << (b ? "true" : "false");
    return os;
}

/// Convert a string to a GdBool.
/// If conversion succeeds, sets b; otherwise b is left untouched.
/// @param s The string to convert. Can contain 0, 1, true, false, on, off, yes, no.
/// @param b The GdBool to write to.
/// @return true, if the conversion succeeded.
bool read_from_string(const std::string &s, GdBool &b) {
    if (s == "1"
            || gd_str_ascii_caseequal(s, "true")
            || gd_str_ascii_caseequal(s, "on")
            || gd_str_ascii_caseequal(s, "yes")) {
        b = true;
        return true;
    } else if (s == "0"
               || gd_str_ascii_caseequal(s, "false")
               || gd_str_ascii_caseequal(s, "off")
               || gd_str_ascii_caseequal(s, "no")) {
        b = false;
        return true;
    }
    return false;
}

/// Save a GdInt to an ostream.
std::ostream &operator<<(std::ostream &os, GdInt const &i) {
    /* have to convert, or else it would be infinite recursion? */
    int conv = i;
    os << conv;
    return os;
}

/// Load a GdInt from a string.
/// If conversion succeeds, sets i; otherwise it is left untouched.
/// @param s The string to convert.
/// @param i The GdInt to write to.
/// @return true, if the conversion succeeded.
bool read_from_string(const std::string &s, GdInt &i) {
    std::istringstream is(s);
    /* was saved as a normal int */
    int read;
    bool success = (bool)(is >> read);
    if (success)
        i = read;
    return success;
}


/// Save a GdProbability to an ostream.
std::ostream &operator<<(std::ostream &os, GdProbability const &i) {
    double conv = i / 1000000.0;
    os << conv;
    return os;
}

/// Load a GdProbability (stored as a floating point number) from a string.
/// If conversion succeeds, sets i; otherwise it is left untouched.
/// @param s The string to convert.
/// @param p The GdProbability to write to.
/// @return true, if the conversion succeeded.
bool read_from_string(const std::string &s, GdProbability &p) {
    std::istringstream is(s);
    double read;
    bool success = (is >> read) && (read >= 0 && read <= 1);
    if (success)
        p = read * 1000000 + 0.5;
    return success;
}

/// Load a GdInt (stored as a floating point number) from a string.
/// If conversion succeeds, sets i; otherwise it is left untouched.
/// @param s The string to convert.
/// @param i The GdInt to write to.
/// @param conversion_ratio The number to multiply the converted value with. (1million for probabilities, cave width*height for ratios)
/// @return true, if the conversion succeeded.
bool read_from_string(const std::string &s, GdInt &i, double conversion_ratio) {
    std::istringstream is(s);
    double read;
    bool success = (is >> read) && (read >= 0 && read <= 1);
    if (success)
        i = read * conversion_ratio + 0.5;
    return success;
}

/// Save a GdScheduling to an ostream with its name.
std::ostream &operator<<(std::ostream &os, GdScheduling const &s) {
    os << scheduling_filename[s];
    return os;
}

/// Read a GdScheduling from a string.
/// If conversion succeeds, sets sch; otherwise it is left untouched.
/// @param s The string to convert.
/// @param sch The GdScheduling to write to.
/// @return true, if the conversion succeeded.
bool read_from_string(const std::string &s, GdScheduling &sch) {
    for (unsigned i = 0; i < G_N_ELEMENTS(scheduling_filename); ++i)
        if (gd_str_ascii_caseequal(s, scheduling_filename[i])) {
            sch = GdSchedulingEnum(i);
            return true;
        }
    return false;
}

/// Save a GdDirection to an ostream with its name.
std::ostream &operator<<(std::ostream &os, GdDirection const &d) {
    os << direction_name[d];
    return os;
}

/// Read a GdDirection from a string.
/// If conversion succeeds, sets d; otherwise it is left untouched.
/// @param s The string to convert.
/// @param d The GdDirection to write to.
/// @return true, if the conversion succeeded.
bool read_from_string(const std::string &s, GdDirection &d) {
    for (unsigned i = 0; i < G_N_ELEMENTS(direction_filename); ++i)
        if (gd_str_ascii_caseequal(s, direction_filename[i])) {
            d = GdDirectionEnum(i);
            return true;
        }
    return false;
}

/// Save a GdElement to an ostream with its name.
std::ostream &operator<<(std::ostream &os, GdElement const &e) {
    os << gd_element_properties[e].filename;
    return os;
}

/// Read a GdElement from a string.
/// If conversion succeeds, sets e; otherwise it is left untouched.
/// @param s The string to convert.
/// @param e The GdElement to write to.
/// @return true, if the conversion succeeded.
bool read_from_string(const std::string &s, GdElement &e) {
    if (!name_to_element.has_name(s))
        return false;
    e = name_to_element.lookup_name(s);
    return true;
}

/// Read a GdEngine from a string.
/// If conversion succeeds, sets e; otherwise it is left untouched.
/// @param s The string to convert.
/// @param e The GdEngine to write to.
/// @return true, if the conversion succeeded.
bool read_from_string(const std::string &s, GdEngine &e) {
    for (unsigned i = 0; i < G_N_ELEMENTS(engines_filename); ++i)
        if (gd_str_ascii_caseequal(s, engines_filename[i])) {
            e = GdEngineEnum(i);
            return true;
        }
    return false;
}
