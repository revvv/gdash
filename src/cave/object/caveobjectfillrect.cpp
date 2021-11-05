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

#include "cave/object/caveobjectfillrect.hpp"
#include "cave/elementproperties.hpp"

#include <glib/gi18n.h>

#include "fileops/bdcffhelper.hpp"
#include "cave/caverendered.hpp"
#include "misc/printf.hpp"

std::string CaveFillRect::get_bdcff() const {
    BdcffFormat f("FillRect");
    f << p1 << p2 << border_element;
    if (border_element != fill_element)
        f << fill_element;

    return f;
}

std::unique_ptr<CaveObject> CaveFillRect::clone_from_bdcff(const std::string &name, std::istream &is) const {
    Coordinate p1, p2;
    GdElementEnum element, element_fill;
    if (!(is >> p1 >> p2 >> element))
        return NULL;
    /* fill element is optional parameter */
    if (!(is >> element_fill))
        element_fill = element;

    return std::make_unique<CaveFillRect>(p1, p2, element, element_fill);
}

std::unique_ptr<CaveObject> CaveFillRect::clone() const {
    return std::make_unique<CaveFillRect>(*this);
}

/// Create filled rectangle cave object.
CaveFillRect::CaveFillRect(Coordinate _p1, Coordinate _p2, GdElementEnum _element, GdElementEnum _fill_element)
    :   CaveRectangular(_p1, _p2),
        border_element(_element),
        fill_element(_fill_element) {
}

void CaveFillRect::draw(CaveRendered &cave, int order_idx) const {
    /* reorder coordinates if not drawing from northwest to southeast */
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    if (y1 > y2)
        std::swap(y1, y2);
    if (x1 > x2)
        std::swap(x1, x2);

    for (int y = y1; y <= y2; y++)
        for (int x = x1; x <= x2; x++)
            cave.store_rc(x, y, (y == y1 || y == y2 || x == x1 || x == x2) ? border_element : fill_element, order_idx);
}

PropertyDescription const CaveFillRect::descriptor[] = {
    {"", GD_TAB, 0, N_("Draw")},
    {"", GD_TYPE_BOOLEAN_LEVELS, 0, N_("Levels"), GetterBase::create_new(&CaveFillRect::seen_on), N_("Levels on which this object is visible.")},
    {"", GD_TYPE_COORDINATE, 0, N_("Start"), GetterBase::create_new(&CaveFillRect::p1), N_("Specifies one of the corners of the object."), 0, 127},
    {"", GD_TYPE_COORDINATE, 0, N_("End"), GetterBase::create_new(&CaveFillRect::p2), N_("Specifies one of the corners of the object."), 0, 127},
    {"", GD_TYPE_ELEMENT, 0, N_("Border element"), GetterBase::create_new(&CaveFillRect::border_element), N_("The outline will be drawn with this element.")},
    {"", GD_TYPE_ELEMENT, 0, N_("Fill element"), GetterBase::create_new(&CaveFillRect::fill_element), N_("The insides of the rectangle will be filled with this element.")},
    {NULL},
};

std::string CaveFillRect::get_description_markup() const {
    return Printf(_("Rectangle from %d,%d to %d,%d of <b>%ms</b>, filled with <b>%ms</b>"), p1.x, p1.y, p2.x, p2.y, visible_name_lowercase(border_element), visible_name_lowercase(fill_element));
}

GdElementEnum CaveFillRect::get_characteristic_element() const {
    return fill_element;
}
