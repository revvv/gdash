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

#include "cave/object/caveobjectpoint.hpp"
#include "cave/elementproperties.hpp"

#include <glib/gi18n.h>

#include "fileops/bdcffhelper.hpp"
#include "cave/caverendered.hpp"
#include "misc/printf.hpp"

/* POINT */
CavePoint::CavePoint(Coordinate _p, GdElementEnum _element)
    :   p(_p),
        element(_element) {
}

std::string CavePoint::get_bdcff() const {
    return BdcffFormat("Point") << p << element;
}

std::unique_ptr<CaveObject> CavePoint::clone_from_bdcff(const std::string &name, std::istream &is) const {
    Coordinate p;
    GdElementEnum element;

    if (!(is >> p >> element))
        return NULL;
    return std::make_unique<CavePoint>(p, element);
}

std::unique_ptr<CaveObject> CavePoint::clone() const {
    return std::make_unique<CavePoint>(*this);
};

void CavePoint::draw(CaveRendered &cave, int order_idx) const {
    cave.store_rc(p.x, p.y, element, order_idx);
}

PropertyDescription const CavePoint::descriptor[] = {
    {"", GD_TAB, 0, N_("Point")},
    {"", GD_TYPE_BOOLEAN_LEVELS, 0, N_("Levels"), GetterBase::create_new(&CavePoint::seen_on), N_("Levels on which this object is visible.")},
    {"", GD_TYPE_COORDINATE, 0, N_("Coordinate"), GetterBase::create_new(&CavePoint::p), N_("Specifies the coordinate of the single element to draw."), 0, 127},
    {"", GD_TYPE_ELEMENT, 0, N_("Element"), GetterBase::create_new(&CavePoint::element), N_("The element to draw.")},
    {NULL},
};

std::string CavePoint::get_coordinates_text() const {
    return Printf("%d,%d", p.x, p.y);
}

void CavePoint::create_drag(Coordinate current, Coordinate displacement) {
    p = current;
}

void CavePoint::move(Coordinate current, Coordinate displacement) {
    p += displacement;
}

void CavePoint::move(Coordinate displacement) {
    p += displacement;
}

std::string CavePoint::get_description_markup() const {
    return Printf(_("Point of <b>%ms</b> at %d,%d"), visible_name_lowercase(element), p.x, p.y);
}

GdElementEnum CavePoint::get_characteristic_element() const {
    return element;
}
