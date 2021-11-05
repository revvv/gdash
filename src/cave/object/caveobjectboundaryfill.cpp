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

#include "fileops/bdcffhelper.hpp"
#include "cave/caverendered.hpp"
#include "cave/elementproperties.hpp"

#include "cave/object/caveobjectboundaryfill.hpp"


std::string CaveBoundaryFill::get_bdcff() const {
    return BdcffFormat("BoundaryFill") << start << fill_element << border_element;
}

std::unique_ptr<CaveObject> CaveBoundaryFill::clone_from_bdcff(const std::string &name, std::istream &is) const {
    Coordinate start;
    GdElementEnum fill, boundary;
    if (!(is >> start >> fill >> boundary))
        return NULL;

    return std::make_unique<CaveBoundaryFill>(start, fill, boundary);
}

std::unique_ptr<CaveObject> CaveBoundaryFill::clone() const {
    return std::make_unique<CaveBoundaryFill>(*this);
}


/// Create a new boundary fill objects.
/// @param _start The starting coordinates of the fill.
/// @param _border_element This is the element which is drawn on the outline of the area.
/// @param _fill_element The inside of the area will be filled with this element.
CaveBoundaryFill::CaveBoundaryFill(Coordinate _start, GdElementEnum _fill_element, GdElementEnum _border_element)
    :   CaveFill(_start, _fill_element),
        border_element(_border_element) {
}

/// Recursive function to fill an area of the cave, until the boundary is reached.
/// @param cave The cave to do the drawing in.
/// @param x The x coordinate to draw at.
/// @param y The y coordinate to draw at.
void CaveBoundaryFill::draw_proc(CaveRendered &cave, CaveMap<bool> &filled, int x, int y) const {
    if (x < 0 || y < 0 || x >= cave.w || y >= cave.h)
        return;
    if (cave.map(x, y) == border_element || filled(x, y))
        return;
    
    filled(x, y) = true;
    draw_proc(cave, filled, x - 1, y);
    draw_proc(cave, filled, x, y - 1);
    draw_proc(cave, filled, x + 1, y);
    draw_proc(cave, filled, x, y + 1);
}

/// Draw the object.
/// First it fills the insides of the area using draw_proc on a bool map.
/// Then it draws the final fill elements using this map.
/// @param cave The cave to draw in.
/// @param level The level the cave is rendered on.
void CaveBoundaryFill::draw(CaveRendered &cave, int order_idx) const {
    /* draw_proc fills the area 'true'. */
    /* after the fill, we change all filled cells to the fill_element. */
    CaveMap<bool> filled(cave.w, cave.h);
    draw_proc(cave, filled, start.x, start.y);
    for (int y = 0; y < cave.h; y++)
        for (int x = 0; x < cave.w; x++)
            if (filled(x, y))
                cave.store_rc(x, y, fill_element, order_idx);
}

PropertyDescription const CaveBoundaryFill::descriptor[] = {
    {"", GD_TAB, 0, N_("Boundary fill")},
    {"", GD_TYPE_BOOLEAN_LEVELS, 0, N_("Levels"), GetterBase::create_new(&CaveBoundaryFill::seen_on), N_("Levels on which this object is visible.")},
    {"", GD_TYPE_COORDINATE, 0, N_("Start"), GetterBase::create_new(&CaveBoundaryFill::start), N_("The coordinate to start the fill at."), 0, 127},
    {"", GD_TYPE_ELEMENT, 0, N_("Border element"), GetterBase::create_new(&CaveBoundaryFill::border_element), N_("This is the border of the fill. The closed area must be bounded by this element.")},
    {"", GD_TYPE_ELEMENT, 0, N_("Fill element"), GetterBase::create_new(&CaveBoundaryFill::fill_element), N_("This is the element the closed area will be filled with.")},
    {NULL},
};

std::string CaveBoundaryFill::get_description_markup() const {
    return Printf(_("Boundary fill from %d,%d of <b>%ms</b>, border <b>%ms</b>"), start.x, start.y, visible_name_lowercase(fill_element), visible_name_lowercase(border_element));
}
