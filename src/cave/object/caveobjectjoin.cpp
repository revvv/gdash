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

#include "cave/object/caveobjectjoin.hpp"

#include <glib/gi18n.h>

#include "fileops/bdcffhelper.hpp"
#include "cave/caverendered.hpp"
#include "misc/printf.hpp"
#include "misc/util.hpp"
#include "cave/elementproperties.hpp"

std::string CaveJoin::get_bdcff() const {
    return BdcffFormat(backwards ? "AddBackward" : "Add") << dist << search_element << put_element;
}

std::unique_ptr<CaveObject> CaveJoin::clone_from_bdcff(const std::string &name, std::istream &is) const {
    Coordinate dist;
    GdElementEnum search, replace;
    if (!(is >> dist >> search >> replace))
        return NULL;

    bool backwards = gd_str_ascii_caseequal(name, "AddBackward");
    return std::make_unique<CaveJoin>(dist, search, replace, backwards);
}

std::unique_ptr<CaveObject> CaveJoin::clone() const {
    return std::make_unique<CaveJoin>(*this);
};

CaveJoin::CaveJoin(Coordinate _dist, GdElementEnum _search_element, GdElementEnum _put_element, bool _backward)
    :   dist(_dist),
        search_element(_search_element),
        put_element(_put_element),
        backwards(_backward) {
}

void CaveJoin::draw(CaveRendered &cave, int order_idx) const {
    /* find every object, and put fill_element next to it. relative coordinates dx,dy */
    if (!backwards) {
        /* from top to bottom */
        for (int y = 0; y < cave.h; y++)
            for (int x = 0; x < cave.w; x++)
                if (cave.map(x, y) == search_element) {
                    /* these new coordinates should wrap around, too. that is needed by profi boulder caves. */
                    /* but they will be wrapped around by store_rc */
                    cave.store_rc(x + dist.x, y + dist.y, put_element, order_idx);
                }
    } else {
        /* from bottom to top */
        for (int y = cave.h - 1; y >= 0; --y)
            for (int x = cave.w - 1; x >= 0; --x)
                if (cave.map(x, y) == search_element) {
                    /* these new coordinates should wrap around, too. that is needed by profi boulder caves. */
                    /* but they will be wrapped around by store_rc */
                    cave.store_rc(x + dist.x, y + dist.y, put_element, order_idx);
                }
    }
}

PropertyDescription const CaveJoin::descriptor[] = {
    {"", GD_TAB, 0, N_("Draw")},
    {"", GD_TYPE_BOOLEAN_LEVELS, 0, N_("Levels"), GetterBase::create_new(&CaveJoin::seen_on), N_("Levels on which this object is visible.")},
    {"", GD_TYPE_ELEMENT, 0, N_("Search element"), GetterBase::create_new(&CaveJoin::search_element), N_("The element to look for. Every element found will generate an add element.")},
    {"", GD_TYPE_ELEMENT, 0, N_("Add element"), GetterBase::create_new(&CaveJoin::put_element), N_("The element to draw.")},
    {"", GD_TYPE_COORDINATE, 0, N_("Distance"), GetterBase::create_new(&CaveJoin::dist), N_("The distance to draw the new element at."), -40, 40},
    {
        "", GD_TYPE_BOOLEAN, 0, N_("Backwards"), GetterBase::create_new(&CaveJoin::backwards), N_("Normally the cave is searched from top to bottom, to find the search element, and draw the add element nearby. If the distance vector of the drawn element is pointing down or right, it might be feasible to do the search from bottom to top, so search elements are not overwritten before finding them, or "
        "the elements may be multiplied, if the search and add elements are the same. "
        "When drawing a join element, GDash automatically selects backward search, if needed - so this setting is provided for compatiblity of old caves only.")
    },
    {NULL},
};

std::string CaveJoin::get_coordinates_text() const {
    return Printf("%+d,%+d", dist.x, dist.y);
}

/// Drag a join object in the editor.
/// Sets displacement and guesses if backwards searching is needed.
/// Here it could be made automatic, as it is more intuitive, and
/// BDCFF can code backward searches as well.
void CaveJoin::create_drag(Coordinate current, Coordinate displacement) {
    dist += displacement;
    /* when just created, guess backwards flag. */
    backwards = dist.y > 0 || (dist.y == 0 && dist.x > 0);
}

void CaveJoin::move(Coordinate current, Coordinate displacement) {
    dist += displacement;
}

void CaveJoin::move(Coordinate displacement) {
    dist += displacement;
}

std::string CaveJoin::get_description_markup() const {
    return Printf(_("Join <b>%ms</b> to every <b>%ms</b>, distance %+d,%+d"), visible_name_lowercase(put_element), visible_name_lowercase(search_element), dist.x, dist.y);
}

GdElementEnum CaveJoin::get_characteristic_element() const {
    return put_element;
}

