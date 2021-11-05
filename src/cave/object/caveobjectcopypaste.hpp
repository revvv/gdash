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
#ifndef CAVEOBJECTCOPYPASTE_HPP_INCLUDED
#define CAVEOBJECTCOPYPASTE_HPP_INCLUDED

#include "config.h"

#include "cave/object/caveobject.hpp"

/// Copy and paste cave object.
/// Copies the rectangle of p1,p2 to the destination.
/// This is not really a CaveRectangular, so it does not derive from that. Position p1,p2 here are
/// NOT the position of the object, but the source area.
class CaveCopyPaste : public CaveObject {
private:
    Coordinate p1, p2;      ///< Source area of copy. Order of p1 and p2 (lower left, upper right etc) is not important.
    Coordinate dest;        ///< Upper left coordinates of destination.
    GdBool mirror;          ///< Mirror the copy horizontally.
    GdBool flip;            ///< Mirror the copy vertically.

public:
    CaveCopyPaste(Coordinate _p1, Coordinate _p2, Coordinate _dest);
    CaveCopyPaste() = default;
    Type get_type() const { return GD_COPY_PASTE; }
    virtual void draw(CaveRendered &cave, int order_idx) const;
    virtual std::unique_ptr<CaveObject> clone() const;
    void set_mirror_flip(bool _mirror, bool _flip);
    virtual std::string get_bdcff() const;
    virtual std::unique_ptr<CaveObject> clone_from_bdcff(const std::string &name, std::istream &is) const;

private:
    static PropertyDescription const descriptor[];

public:
    virtual PropertyDescription const *get_description_array() const {
        return descriptor;
    }

    virtual void create_drag(Coordinate current, Coordinate displacement);
    virtual void move(Coordinate current, Coordinate displacement);
    virtual void move(Coordinate displacement);
    virtual std::string get_coordinates_text() const;
    virtual GdElementEnum get_characteristic_element() const;
    virtual std::string get_description_markup() const;
};


#endif

