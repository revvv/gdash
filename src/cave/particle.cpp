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

#include <glib.h>

#include "cave/particle.hpp"

ParticleSet::ParticleSet(int count, float size, float opacity, float p0x, float p0y, float dp0x, float dp0y, float v0x, float v0y, float dvx, float dvy, const GdColor &color)
    : color(color)
    , life(1000)
    , is_new(true)
    , size(size)
    , opacity(opacity)
    , particles(count) {
    for (size_t i = 0; i < particles.size(); ++i) {
        Particle &p = particles[i];
        p.px = p0x + g_random_double_range(-dp0x, dp0x);
        p.py = p0y + g_random_double_range(-dp0y, dp0y);
        p.vx = v0x + g_random_double_range(-dvx, dvx);
        p.vy = v0y + g_random_double_range(-dvy, dvy);
    }
}


void ParticleSet::move(int dt_ms) {
    float dt = dt_ms / 1000.0;

    for (size_t i = 0; i < particles.size(); ++i) {
        Particle &p = particles[i];
        p.px += p.vx * dt;
        p.py += p.vy * dt;
    }

    life -= dt_ms;
}


void ParticleSet::normalize(double factor) {
    if (!is_new)
        return;
    is_new = false;

    size *= factor;
    for (size_t i = 0; i < particles.size(); ++i) {
        Particle &p = particles[i];
        p.px *= factor;
        p.py *= factor;
        p.vx *= factor;
        p.vy *= factor;
    }
}
