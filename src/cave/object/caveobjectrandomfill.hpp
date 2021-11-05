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
#ifndef CAVEOBJECTRANDOMFILL_HPP_INCLUDED
#define CAVEOBJECTRANDOMFILL_HPP_INCLUDED

#include "config.h"

#include "cave/object/caveobject.hpp"
#include "cave/object/caveobjectrectangular.hpp"

/* RANDOM FILL OBJECT */
class CaveRandomFill : public CaveRectangular {
private:
    GdElement replace_only;
    GdBool c64_random;

    GdIntLevels seed;
    GdElement initial_fill;
    GdElement random_fill_1;                ///< Random fill element 1
    GdInt random_fill_probability_1;        ///< 0..255 "probability" of random fill element 1
    GdElement random_fill_2;                ///< Random fill element 2
    GdInt random_fill_probability_2;        ///< 0..255 "probability" of random fill element 2
    GdElement random_fill_3;                ///< Random fill element 3
    GdInt random_fill_probability_3;        ///< 0..255 "probability" of random fill element 3
    GdElement random_fill_4;                ///< Random fill element 4
    GdInt random_fill_probability_4;        ///< 0..255 "probability" of random fill element 4
public:
    CaveRandomFill(Coordinate _p1, Coordinate _p2);
    CaveRandomFill() = default;
    Type get_type() const { return GD_RANDOM_FILL; }
    virtual void draw(CaveRendered &cave, int order_idx) const;
    virtual std::unique_ptr<CaveObject> clone() const;
    void set_random_fill(GdElementEnum initial, GdElementEnum e1, GdElementEnum e2, GdElementEnum e3, GdElementEnum e4);
    void set_random_prob(int i1, int i2, int i3, int i4);
    void set_seed(int s1, int s2, int s3, int s4, int s5);
    void set_c64_random(bool rand) {
        c64_random = rand;
    }
    void set_replace_only(GdElementEnum repl) {
        replace_only = repl;
    }
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

