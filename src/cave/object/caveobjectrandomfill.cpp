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

#include "cave/object/caveobjectrandomfill.hpp"
#include "cave/elementproperties.hpp"

#include <glib/gi18n.h>

#include "fileops/bdcffhelper.hpp"
#include "cave/caverendered.hpp"
#include "misc/printf.hpp"
#include "misc/util.hpp"

std::string CaveRandomFill::get_bdcff() const {
    const char *type = c64_random ? "RandomFillC64" : "RandomFill";
    BdcffFormat f(type);

    f << p1 << p2;
    f << seed[0] << seed[1] << seed[2] << seed[3] << seed[4];
    f << initial_fill;
    if (random_fill_probability_1 != 0)
        f << random_fill_1 << random_fill_probability_1;
    if (random_fill_probability_2 != 0)
        f << random_fill_2 << random_fill_probability_2;
    if (random_fill_probability_3 != 0)
        f << random_fill_3 << random_fill_probability_3;
    if (random_fill_probability_4 != 0)
        f << random_fill_4 << random_fill_probability_4;
    if (replace_only != O_NONE)
        f << replace_only;

    return f;
}

std::unique_ptr<CaveObject> CaveRandomFill::clone_from_bdcff(const std::string &name, std::istream &is) const {
    Coordinate p1, p2;
    int seed[5];
    GdElementEnum initial_fill, replace_only;
    GdElementEnum random_fill[4] = {O_DIRT, O_DIRT, O_DIRT, O_DIRT};
    int random_prob[4] = {0, 0, 0, 0};
    std::string s1, s2;

    if (!(is >> p1 >> p2 >> seed[0] >> seed[1] >> seed[2] >> seed[3] >> seed[4] >> initial_fill))
        return NULL;
    int j = 0;
    /* now come fill element&probability pairs. read as we find two words */
    while (j < 4 && (is >> s1 >> s2)) {
        std::istringstream is1(s1), is2(s2);
        // read two strings - they must be fill & prob
        if (!(is1 >> random_fill[j]))
            return NULL;
        if (!(is2 >> random_prob[j]))
            return NULL;
        s1 = "";
        s2 = "";
        j++;
    }
    /* if we could read s1, but not read s2, now s1 contains a "replace only" element. */
    /* so process it later. */
    /* if j=4, no more reads took place, but there might be still one string at the end. */
    /* so read if j=4 is the case. */
    if (j == 4)
        is >> s1;
    /* ok now try if we have a replace only element. */
    if (s1 != "") {
        std::istringstream is(s1);
        // one string left - must be a replace only element
        if (!(is >> replace_only))
            return NULL;
    } else
        replace_only = O_NONE;

    CaveRandomFill r(p1, p2);
    r.set_replace_only(replace_only);
    r.set_seed(seed[0], seed[1], seed[2], seed[3], seed[4]);
    r.set_random_fill(initial_fill, random_fill[0], random_fill[1], random_fill[2], random_fill[3]);
    r.set_random_prob(random_prob[0], random_prob[1], random_prob[2], random_prob[3]);
    r.set_c64_random(gd_str_ascii_caseequal(name, "RandomFillC64"));
    return std::make_unique<CaveRandomFill>(std::move(r));
}


std::unique_ptr<CaveObject> CaveRandomFill::clone() const {
    return std::make_unique<CaveRandomFill>(*this);
}


CaveRandomFill::CaveRandomFill(Coordinate _p1, Coordinate _p2)
    :   CaveRectangular(_p1, _p2),
        replace_only(O_NONE),
        c64_random(false) {
    initial_fill = O_SPACE;
    random_fill_1 = O_DIRT;
    random_fill_probability_1 = 0;
    random_fill_2 = O_DIRT;
    random_fill_probability_2 = 0;
    random_fill_3 = O_DIRT;
    random_fill_probability_3 = 0;
    random_fill_4 = O_DIRT;
    random_fill_probability_4 = 0;
    for (int j = 0; j < 5; j++)
        seed[j] = -1;
}

void CaveRandomFill::set_random_fill(GdElementEnum initial, GdElementEnum e1, GdElementEnum e2, GdElementEnum e3, GdElementEnum e4) {
    initial_fill = initial;
    random_fill_1 = e1;
    random_fill_2 = e2;
    random_fill_3 = e3;
    random_fill_4 = e4;
}

void CaveRandomFill::set_random_prob(int i1, int i2, int i3, int i4) {
    random_fill_probability_1 = i1;
    random_fill_probability_2 = i2;
    random_fill_probability_3 = i3;
    random_fill_probability_4 = i4;
}

void CaveRandomFill::set_seed(int s1, int s2, int s3, int s4, int s5) {
    seed[0] = s1;
    seed[1] = s2;
    seed[2] = s3;
    seed[3] = s4;
    seed[4] = s5;
}

void CaveRandomFill::draw(CaveRendered &cave, int order_idx) const {
    /* -1 means that it should be different every time played. */
    guint32 s;
    if (seed[cave.rendered_on] == -1)
        s = cave.random.rand_int();
    else
        s = seed[cave.rendered_on];

    RandomGenerator rand;
    rand.set_seed(s);
    /* for c64 random, use the 2*8 lsb. */
    C64RandomGenerator c64rand;
    c64rand.set_seed(s);

    /* change coordinates if not in correct order */
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    if (y1 > y2)
        std::swap(y1, y2);
    if (x1 > x2)
        std::swap(x1, x2);

    for (int y = y1; y <= y2; y++)
        for (int x = x1; x <= x2; x++) {
            int randm;

            if (c64_random) /* use c64 random generator */
                randm = c64rand.random();
            else    /* use the much better glib random generator */
                randm = rand.rand_int_range(0, 256);

            GdElementEnum element = initial_fill;
            if (randm < random_fill_probability_1)
                element = random_fill_1;
            if (randm < random_fill_probability_2)
                element = random_fill_2;
            if (randm < random_fill_probability_3)
                element = random_fill_3;
            if (randm < random_fill_probability_4)
                element = random_fill_4;
            if (replace_only == O_NONE || cave.map(x, y) == replace_only)
                cave.store_rc(x, y, element, order_idx);
        }
}

PropertyDescription const CaveRandomFill::descriptor[] = {
    {"", GD_TAB, 0, N_("Random Fill")},
    {"", GD_TYPE_BOOLEAN_LEVELS, 0, N_("Levels"), GetterBase::create_new(&CaveRandomFill::seen_on), N_("Levels on which this object is visible.")},
    {"", GD_TYPE_COORDINATE, 0, N_("Start corner"), GetterBase::create_new(&CaveRandomFill::p1), N_("Specifies one of the corners of the object."), 0, 127},
    {"", GD_TYPE_COORDINATE, 0, N_("End corner"), GetterBase::create_new(&CaveRandomFill::p2), N_("Specifies one of the corners of the object."), 0, 127},
    {"", GD_TYPE_ELEMENT, 0, N_("Replace only"), GetterBase::create_new(&CaveRandomFill::replace_only), N_("If this setting is set to 'no element' but another one, only that will be replaced when drawing the fill, and other elements will not be overwritten. Otherwise the whole block is filled.")},
    {"", GD_TAB, 0, N_("Generator")},
    {"", GD_TYPE_BOOLEAN, 0, N_("C64 random"), GetterBase::create_new(&CaveRandomFill::c64_random), N_("Controls if the C64 random generator is used for the fill, or a more advanced one. By using the C64 generator, you can recreate the patterns of the original game.")},
    {"", GD_TYPE_INT_LEVELS, 0, N_("Random seed"), GetterBase::create_new(&CaveRandomFill::seed), N_("Random seed value controls the predictable random number generator, which fills the cave initially. If set to -1, cave is totally random every time it is played."), -1, GD_CAVE_SEED_MAX},
    {"", GD_TYPE_ELEMENT, 0, N_("Initial fill"), GetterBase::create_new(&CaveRandomFill::initial_fill), 0},
    {"", GD_TYPE_INT, GD_BD_PROBABILITY, N_("Probability 1"), GetterBase::create_new(&CaveRandomFill::random_fill_probability_1), 0, 0, 255},
    {"", GD_TYPE_ELEMENT, 0, N_("Random fill 1"), GetterBase::create_new(&CaveRandomFill::random_fill_1), 0},
    {"", GD_TYPE_INT, GD_BD_PROBABILITY, N_("Probability 2"), GetterBase::create_new(&CaveRandomFill::random_fill_probability_2), 0, 0, 255},
    {"", GD_TYPE_ELEMENT, 0, N_("Random fill 2"), GetterBase::create_new(&CaveRandomFill::random_fill_2), 0},
    {"", GD_TYPE_INT, GD_BD_PROBABILITY, N_("Probability 3"), GetterBase::create_new(&CaveRandomFill::random_fill_probability_3), 0, 0, 255},
    {"", GD_TYPE_ELEMENT, 0, N_("Random fill 3"), GetterBase::create_new(&CaveRandomFill::random_fill_3), 0},
    {"", GD_TYPE_INT, GD_BD_PROBABILITY, N_("Probability 4"), GetterBase::create_new(&CaveRandomFill::random_fill_probability_4), 0, 0, 255},
    {"", GD_TYPE_ELEMENT, 0, N_("Random fill 4"), GetterBase::create_new(&CaveRandomFill::random_fill_4), 0},
    {NULL},
};

std::string CaveRandomFill::get_description_markup() const {
    if (replace_only == O_NONE)
        return Printf(_("Random fill from %d,%d to %d,%d"), p1.x, p1.y, p2.x, p2.y);
    else
        return Printf(_("Random fill from %d,%d to %d,%d, replacing %ms"), p1.x, p1.y, p2.x, p2.y, visible_name_lowercase(replace_only));
}

GdElementEnum CaveRandomFill::get_characteristic_element() const {
    return O_NONE;
}
