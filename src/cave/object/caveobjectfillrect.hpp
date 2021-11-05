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
#ifndef CAVEOBJECTFILLRECT_HPP_INCLUDED
#define CAVEOBJECTFILLRECT_HPP_INCLUDED

#include "config.h"

#include "cave/object/caveobject.hpp"
#include "cave/object/caveobjectrectangular.hpp"

/// Filled rectangle cave object.
/// The rectangle with a border of 1 cells in width is filled with another element.
class CaveFillRect : public CaveRectangular {
    GdElement border_element;   ///< Border of the rectangle is this element.
    GdElement fill_element;     ///< Insides of the rectangle is this element.

public:
    CaveFillRect(Coordinate _p1, Coordinate _p2, GdElementEnum _element, GdElementEnum _fill_element);
    CaveFillRect() = default;
    Type get_type() const { return GD_FILLED_RECTANGLE; }
    virtual std::unique_ptr<CaveObject> clone() const;
    virtual void draw(CaveRendered &cave, int order_idx) const;
    virtual std::string get_bdcff() const;
    virtual std::unique_ptr<CaveObject> clone_from_bdcff(const std::string &name, std::istream &is) const;

private:
    static PropertyDescription const descriptor[];

public:
    virtual PropertyDescription const *get_description_array() const {
        return descriptor;
    }
    
    virtual GdElementEnum get_characteristic_element() const;
    virtual std::string get_description_markup() const;
};


#endif

