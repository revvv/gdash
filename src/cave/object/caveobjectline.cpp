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

#include "cave/object/caveobjectline.hpp"
#include "cave/elementproperties.hpp"

#include <glib/gi18n.h>
#include <cstdlib>

#include "fileops/bdcffhelper.hpp"
#include "cave/caverendered.hpp"
#include "misc/printf.hpp"

/// Create a line cave object.
/// @param _p1 Starting point of the line.
/// @param _p2 Ending point of the line.
/// @param _element Element to draw.
CaveLine::CaveLine(Coordinate _p1, Coordinate _p2, GdElementEnum _element)
    :   p1(_p1),
        p2(_p2),
        element(_element) {
}

std::string CaveLine::get_bdcff() const {
    return BdcffFormat("Line") << p1 << p2 << element;
}

std::unique_ptr<CaveObject> CaveLine::clone_from_bdcff(const std::string &name, std::istream &is) const {
    Coordinate p1, p2;
    GdElementEnum element;
    if (!(is >> p1 >> p2 >> element))
        return NULL;
    return std::make_unique<CaveLine>(p1, p2, element);
}

std::unique_ptr<CaveObject> CaveLine::clone() const {
    return std::make_unique<CaveLine>(*this);
}

/// Draws the line.
/// Uses Bresenham's algorithm, as descriped in Wikipedia.
void CaveLine::draw(CaveRendered &cave, int order_idx) const {
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    bool steep = abs(y2 - y1) > abs(x2 - x1);
    if (steep) {
        std::swap(x1, y1);  /* yes, change x with y */
        std::swap(x2, y2);
    }
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    int dx = x2 - x1;
    int dy = abs(y2 - y1);
    int error = 0;
    int ystep = (y1 < y2) ? 1 : -1;

    int y = y1;
    for (int x = x1; x <= x2; x++) {
        if (steep)
            cave.store_rc(y, x, element, order_idx); /* and here we change them back, if needed */
        else
            cave.store_rc(x, y, element, order_idx);
        error += dy;
        if (error * 2 >= dx) {
            y += ystep;
            error -= dx;
        }
    }
}

PropertyDescription const CaveLine::descriptor[] = {
    {"", GD_TAB, 0, N_("Line")},
    {"", GD_TYPE_BOOLEAN_LEVELS, 0, N_("Levels"), GetterBase::create_new(&CaveLine::seen_on), N_("Levels on which this object is visible.")},
    {"", GD_TYPE_COORDINATE, 0, N_("Start"), GetterBase::create_new(&CaveLine::p1), N_("This is the start point of the line."), 0, 127},
    {"", GD_TYPE_COORDINATE, 0, N_("End"), GetterBase::create_new(&CaveLine::p2), N_("This is the end point of the line."), 0, 127},
    {"", GD_TYPE_ELEMENT, 0, N_("Element"), GetterBase::create_new(&CaveLine::element), N_("The element to draw.")},
    {NULL},
};

std::string CaveLine::get_coordinates_text() const {
    return Printf("%d,%d-%d,%d", p1.x, p1.y, p2.x, p2.y);
}

void CaveLine::create_drag(Coordinate current, Coordinate displacement) {
    p2 = current;
}

void CaveLine::move(Coordinate current, Coordinate displacement) {
    if (current == p1)
        p1 += displacement; /* move endpoint 1 */
    else if (current == p2)
        p2 += displacement; /* or move endpoint 2 */
    else {
        p1 += displacement; /* or move the whole thing */
        p2 += displacement;
    }
}

void CaveLine::move(Coordinate displacement) {
    p1 += displacement;
    p2 += displacement;
}

std::string CaveLine::get_description_markup() const {
    return Printf(_("Line from %d,%d to %d,%d of <b>%ms</b>"), p1.x, p1.y, p2.x, p2.y, visible_name_lowercase(element));
}

GdElementEnum CaveLine::get_characteristic_element() const {
    return element;
}
