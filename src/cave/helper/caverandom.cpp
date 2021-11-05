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

#include <glib.h>
#include <algorithm>
#include "cave/helper/caverandom.hpp"

/// Constructor. Initializes generator to a random series.
C64RandomGenerator::C64RandomGenerator() {
    /* no seed given, but do something sensible */
    set_seed(g_random_int_range(0, 65536));
}

/// Generate random number.
/// @return Random number between 0 and 255.
unsigned int C64RandomGenerator::random() {
    unsigned int temp_rand_1, temp_rand_2, carry, result;

    temp_rand_1 = (rand_seed_1 & 0x0001) << 7;
    temp_rand_2 = (rand_seed_2 >> 1) & 0x007F;
    result = (rand_seed_2) + ((rand_seed_2 & 0x0001) << 7);
    carry = (result >> 8);
    result = result & 0x00FF;
    result = result + carry + 0x13;
    carry = (result >> 8);
    rand_seed_2 = result & 0x00FF;
    result = rand_seed_1 + carry + temp_rand_1;
    carry = (result >> 8);
    result = result & 0x00FF;
    result = result + carry + temp_rand_2;
    rand_seed_1 = result & 0x00FF;

    return rand_seed_1;
}
