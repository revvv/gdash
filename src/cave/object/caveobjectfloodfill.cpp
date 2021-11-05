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

#include "cave/object/caveobjectfloodfill.hpp"

#include <glib/gi18n.h>

#include "fileops/bdcffhelper.hpp"
#include "cave/caverendered.hpp"
#include "misc/printf.hpp"
#include "cave/elementproperties.hpp"

/// Create a floodfill object.
/// @param _start Start coordinates of fill.
/// @param _search_element Overwrite this element.
/// @param _fill_element Draw this element.
CaveFloodFill::CaveFloodFill(Coordinate _start, GdElementEnum _fill_element, GdElementEnum _search_element)
    :   CaveFill(_start, _fill_element),
        search_element(_search_element) {
}

std::string CaveFloodFill::get_bdcff() const {
    return BdcffFormat("FloodFill") << start << fill_element << search_element;
}

std::unique_ptr<CaveObject> CaveFloodFill::clone_from_bdcff(const std::string &name, std::istream &is) const {
    Coordinate start;
    GdElementEnum fill, search;
    if (!(is >> start >> fill >> search))
        return NULL;

    return std::make_unique<CaveFloodFill>(start, fill, search);
}

std::unique_ptr<CaveObject> CaveFloodFill::clone() const {
    return std::make_unique<CaveFloodFill>(*this);
};

/// Standard recursive floodfill algorithm.
void CaveFloodFill::draw_proc(CaveRendered &cave, int x, int y, int order_idx) const {
    if (x < 0 || y < 0 || x >= cave.w || y >= cave.h)
        return;
    if (cave.map(x, y) != search_element)
        return;
    cave.store_rc(x, y, fill_element, order_idx);

    draw_proc(cave, x - 1, y, order_idx);
    draw_proc(cave, x, y - 1, order_idx);
    draw_proc(cave, x + 1, y, order_idx);
    draw_proc(cave, x, y + 1, order_idx);
}

void CaveFloodFill::draw(CaveRendered &cave, int order_idx) const {
    if (search_element == fill_element)
        return;
    draw_proc(cave, start.x, start.y, order_idx);
}

PropertyDescription const CaveFloodFill::descriptor[] = {
    {"", GD_TAB, 0, N_("Flood fill")},
    {"", GD_TYPE_BOOLEAN_LEVELS, 0, N_("Levels"), GetterBase::create_new(&CaveFloodFill::seen_on), N_("Levels on which this object is visible.")},
    {"", GD_TYPE_COORDINATE, 0, N_("Start"), GetterBase::create_new(&CaveFloodFill::start), N_("The coordinate to start the filling at."), 0, 127},
    {"", GD_TYPE_ELEMENT, 0, N_("Search element"), GetterBase::create_new(&CaveFloodFill::search_element), N_("The element which specifies the area to be filled, and will be overwritten.")},
    {"", GD_TYPE_ELEMENT, 0, N_("Fill element"), GetterBase::create_new(&CaveFloodFill::fill_element), N_("The element used to fill the area.")},
    {NULL},
};

std::string CaveFloodFill::get_description_markup() const {
    return Printf(_("Flood fill from %d,%d of <b>%ms</b>, replacing <b>%ms</b>"), start.x, start.y, visible_name_lowercase(fill_element), visible_name_lowercase(search_element));
}
