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

#include "cave/object/caveobjectfill.hpp"

#include "misc/printf.hpp"


/// Create a new fill object.
/// @param _start The starting coordinates of the fill.
/// @param _fill_element The inside of the area will be filled with this element.
CaveFill::CaveFill(Coordinate _start, GdElementEnum _fill_element)
    :   start(_start),
        fill_element(_fill_element) {
}

std::string CaveFill::get_coordinates_text() const {
    return Printf("%d,%d", start.x, start.y);
}

GdElementEnum CaveFill::get_characteristic_element() const {
    return fill_element;
}

void CaveFill::create_drag(Coordinate current, Coordinate displacement) {
    start = current;
}

void CaveFill::move(Coordinate current, Coordinate displacement) {
    if (start == current)   /* can only drag by the starting point */
        start += displacement;
}

void CaveFill::move(Coordinate displacement) {
    start += displacement;
}
