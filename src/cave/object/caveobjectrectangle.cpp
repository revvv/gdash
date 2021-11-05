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

#include "cave/object/caveobjectrectangle.hpp"
#include "cave/elementproperties.hpp"

#include <glib/gi18n.h>

#include "fileops/bdcffhelper.hpp"
#include "cave/caverendered.hpp"
#include "misc/printf.hpp"

std::string CaveRectangle::get_bdcff() const {
    return BdcffFormat("Rectangle") << p1 << p2 << element;
}

std::unique_ptr<CaveObject> CaveRectangle::clone_from_bdcff(const std::string &name, std::istream &is) const {
    Coordinate p1, p2;
    GdElementEnum element;
    if (!(is >> p1 >> p2 >> element))
        return NULL;
    return std::make_unique<CaveRectangle>(p1, p2, element);
}


std::unique_ptr<CaveObject> CaveRectangle::clone() const {
    return std::make_unique<CaveRectangle>(*this);
}


CaveRectangle::CaveRectangle(Coordinate _p1, Coordinate _p2, GdElementEnum _element)
    :   CaveRectangular(_p1, _p2),
        element(_element) {
}

/* rectangle, frame only */
void CaveRectangle::draw(CaveRendered &cave, int order_idx) const {
    /* reorder coordinates if not drawing from northwest to southeast */
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    if (y1 > y2)
        std::swap(y1, y2);
    if (x1 > x2)
        std::swap(x1, x2);

    for (int x = x1; x <= x2; x++) {
        cave.store_rc(x, y1, element, order_idx);
        cave.store_rc(x, y2, element, order_idx);
    }
    for (int y = y1; y <= y2; y++) {
        cave.store_rc(x1, y, element, order_idx);
        cave.store_rc(x2, y, element, order_idx);
    }
}

PropertyDescription const CaveRectangle::descriptor[] = {
    {"", GD_TAB, 0, N_("Rectangle")},
    {"", GD_TYPE_BOOLEAN_LEVELS, 0, N_("Levels"), GetterBase::create_new(&CaveRectangle::seen_on), N_("Levels on which this object is visible.")},
    {"", GD_TYPE_COORDINATE, 0, N_("Start corner"), GetterBase::create_new(&CaveRectangle::p1), N_("Specifies one of the corners of the object."), 0, 127},
    {"", GD_TYPE_COORDINATE, 0, N_("End corner"), GetterBase::create_new(&CaveRectangle::p2), N_("Specifies one of the corners of the object."), 0, 127},
    {"", GD_TYPE_ELEMENT, 0, N_("Element"), GetterBase::create_new(&CaveRectangle::element), N_("The element to draw.")},
    {NULL},
};

std::string CaveRectangle::get_description_markup() const {
    return Printf(_("Rectangle from %d,%d to %d,%d of <b>%ms</b>"), p1.x, p1.y, p2.x, p2.y, visible_name_lowercase(element));
}

GdElementEnum CaveRectangle::get_characteristic_element() const {
    return element;
}
