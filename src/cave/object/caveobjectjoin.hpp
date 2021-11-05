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
#ifndef CAVEOBJECTJOIN_HPP_INCLUDED
#define CAVEOBJECTJOIN_HPP_INCLUDED

#include "config.h"

#include "cave/object/caveobject.hpp"

/// Cave join object.
/// This one checks the entire cave, looking for search_element; if it finds one,
/// it draws put_element in a distance determined by the vector dist.
/// Whether the algorithm checks the cave from top to bottom or from bottom to
/// top, is determined by the backwards variable.
/// When the dist vector is pointing to down (or right with y=0), the search
/// for elements should go backwards; otherwise an element already drawn
/// will be checked again (or maybe a search_element is overwritten).
/// However this choice cannot be automatized, as compatibility with C64 games
/// must be retained, where all joins worked in a top to bottom fashion.
class CaveJoin : public CaveObject {
private:
    Coordinate dist;            ///< Distance to draw the new element at.
    GdElement search_element;   ///< Search this element.
    GdElement put_element;      ///< Draw this element as a pair for every search_element found.
    GdBool backwards;           ///< If true, search goes from bottom to top.
public:
    CaveJoin(Coordinate _dist, GdElementEnum _search_element, GdElementEnum _put_element, bool _backward = false);
    CaveJoin() = default;
    Type get_type() const { return GD_JOIN; }
    virtual void draw(CaveRendered &cave, int order_idx) const;
    virtual std::unique_ptr<CaveObject> clone() const;
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

