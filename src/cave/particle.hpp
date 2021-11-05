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

#ifndef PARTICLE_HPP_INCLUDED
#define PARTICLE_HPP_INCLUDED

#include "config.h"

#include <vector>
#include "cave/colors.hpp"

class ParticleSet {
public:
    /// One single particle.
    struct Particle {
        float px, py;         ///< Coordinate
        float vx, vy;         ///< Speed
    };

    /// This constructor creates a particle set, for the given cave coordinates.
    /// 0,0 is the top left corner of the cave; 1,1 is the bottom right corner of
    /// the top left cave cell. (So the max coordinates are the width and height
    /// of the cave.)
    /// @param cave_x Particle set starting x coordinate in cave coordinates.
    /// @param cave_y Particle set starting y coordinate in cave coordinates.
    /// @param dx Half the width of the region, in which originally particles are randomly generated.
    /// @param dy Half the height of the region, in which originally particles are randomly generated.
    /// @param vx Maximum original speed.
    /// @param vy Maximum original speed.
    ParticleSet(int count, float size, float opacity, float p0x, float p0y, float dp0x, float dp0y, float v0x, float v0y, float dvx, float dvy, const GdColor &color);
    /// Move the particles.
    /// @param dt_ms Time elapsed.
    void move(int dt_ms);
    /// Scale coordinates to screen cordinates, if is_new is true.
    /// @param factor The number of pixels per cell on the screen.
    void normalize(double factor);

    typedef std::vector<Particle> container;
    typedef container::iterator iterator;
    typedef container::const_iterator const_iterator;

    iterator begin() {
        return particles.begin();
    }
    iterator end() {
        return particles.end();
    }
    const_iterator begin() const {
        return particles.begin();
    }
    const_iterator end() const {
        return particles.end();
    }
    size_t num_particles() const {
        return particles.size();
    }

    GdColor color;
    int life;           ///< lifetime. starts from 1000, goes to 0.
    bool is_new;        ///< New particle set, the coordinates of which must be "normalized" to the cave screen coordinates
    float size;         ///< Size of the particles.
    float opacity;      ///< Opacity between 0 and 1. Values close to 1 not recommended.

private:
    container particles;
};

#endif
