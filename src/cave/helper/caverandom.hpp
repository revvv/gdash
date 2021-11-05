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

#ifndef CAVERANDOM_HPP_INCLUDED
#define CAVERANDOM_HPP_INCLUDED

#include "config.h"

#include <glib.h>

/// maximum seed value for the cave random generator.
enum { GD_CAVE_SEED_MAX = 65535 };

/**
 * @brief Wraps a GLib random generator to make it a C++ class.
 *
 * This is the main random generator, which is used during
 * playing the cave. The C64 random generator is only used when
 * creating the cave.
 */
class RandomGenerator {
private:
    /// The GRand wrapped - stores the internal state.
    GRand *rand;

public:
    /// Create object; initialize randomly
    RandomGenerator() {
        rand = g_rand_new();
    }

    /// Create object.
    /// @param seed Random number seed to be used.
    explicit RandomGenerator(unsigned int seed) {
        rand = g_rand_new_with_seed(seed);
    }

    RandomGenerator(const RandomGenerator & other) {
        rand = g_rand_copy(other.rand);
    }
    RandomGenerator(RandomGenerator && other) {
        rand = other.rand;
        other.rand = NULL;
    }
    RandomGenerator &operator=(RandomGenerator rhs) {
        std::swap(rand, rhs.rand);
        return *this;
    }
    ~RandomGenerator() {
        if (rand != NULL)
            g_rand_free(rand);
    }

    /// Set seed to given number, to generate a series of random numbers.
    /// @param seed The seed value.
    void set_seed(unsigned int seed) {
        g_rand_set_seed(rand, seed);
    }
    
    /// Generater a random boolean. 50% false, 50% true.
    bool rand_boolean() {
        return g_rand_boolean(rand) != FALSE;
    }

    /// Generate a random integer, [begin, end).
    /// @param begin Start of interval, inclusive.
    /// @param end End of interval, non-inclusive.
    int rand_int_range(int begin, int end) {
        return g_rand_int_range(rand, begin, end);
    }

    /// Generate a random 32-bit unsigned integer.
    unsigned int rand_int() {
        return g_rand_int(rand);
    }
};

/**
 * @brief Random number generator, which is compatible with the original game.
 *
 * C64 BD predictable random number generator.
 * Used to load the original caves imported from c64 files.
 * Also by the predictable slime.
 */
class C64RandomGenerator {
private:
    /// Internal state of random number generator.
    int rand_seed_1;
    /// Internal state of random number generator.
    int rand_seed_2;

public:
    C64RandomGenerator();

    /// Set seed. The same as set_seed(int), but 2*8 bits must be given.
    /// @param seed1 First 8 bits of seed value.
    /// @param seed2 Second 8 bits of seed value.
    void set_seed(int seed1, int seed2) {
        rand_seed_1 = seed1 % 256;
        rand_seed_2 = seed2 % 256;
    }

    /// Set seed.
    /// @param seed Seed value. Only lower 16 bits will be used.
    void set_seed(int seed) {
        rand_seed_1 = seed / 256 % 256;
        rand_seed_2 = seed % 256;
    }

    unsigned int random();
};


#endif

