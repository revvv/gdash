/*
 * Copyright (c) 2007-2013, Czirkos Zoltan http://code.google.com/p/gdash/
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef _GD_CAVEOBJECTBOUNDARYFILL
#define _GD_CAVEOBJECTBOUNDARYFILL

#include "config.h"

#include "cave/object/caveobject.hpp"
#include "cave/object/caveobjectfill.hpp"

/// A cave objects which fills the inside of an area set by a border.
class CaveBoundaryFill : public CaveFill {
    GdElement border_element;       ///< The border of the area is this element.
    void draw_proc(CaveRendered &cave, int x, int y) const;

public:
    CaveBoundaryFill(Coordinate _start, GdElementEnum _border_element, GdElementEnum _fill_element);
    CaveBoundaryFill(): CaveFill(GD_FLOODFILL_BORDER) {}
    virtual CaveBoundaryFill *clone() const {
        return new CaveBoundaryFill(*this);
    }
    virtual void draw(CaveRendered &cave) const;
    virtual std::string get_bdcff() const;
    virtual CaveBoundaryFill *clone_from_bdcff(const std::string &name, std::istream &is) const;

private:
    static PropertyDescription const descriptor[];

public:
    virtual PropertyDescription const *get_description_array() const;

    virtual std::string get_description_markup() const;
};


#endif
