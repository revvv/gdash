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
#ifndef CAVEOBJECTRECTANGLE_HPP_INCLUDED
#define CAVEOBJECTRECTANGLE_HPP_INCLUDED

#include "config.h"

#include "cave/object/caveobject.hpp"
#include "cave/object/caveobjectrectangular.hpp"

/* RECTANGLE */
class CaveRectangle : public CaveRectangular {
    GdElement element;

public:
    CaveRectangle(Coordinate _p1, Coordinate _p2, GdElementEnum _element);
    CaveRectangle() = default;
    Type get_type() const { return GD_RECTANGLE; }
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
    virtual std::string get_description_markup() const;
    virtual GdElementEnum get_characteristic_element() const;
};


#endif

