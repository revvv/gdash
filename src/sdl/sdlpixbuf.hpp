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

#ifndef SDLPIXBUF_HPP_INCLUDED
#define SDLPIXBUF_HPP_INCLUDED

#include "config.h"

#include <SDL2/SDL.h>
#include <memory>

#include "misc/deleter.hpp"
#include "gfx/pixbuf.hpp"


/// A class which represents a 32-bit RGBA image in memory.
class SDLPixbuf: public Pixbuf {
private:
    std::unique_ptr<SDL_Surface, Deleter<SDL_Surface, SDL_FreeSurface>> surface;                       ///< SDL pixbuf data

public:
    explicit SDLPixbuf(SDL_Surface *surface_);
    explicit SDLPixbuf(int w, int h);

    /* not const-correct, but sdl does not use const */
    SDL_Surface *get_surface() const {
        return surface.get();
    }

    virtual int get_width() const;
    virtual int get_height() const;

    virtual void blit_full(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const;
    virtual void copy_full(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const;
    virtual void scale_full(Pixbuf &dest, double scaling_factor) const;
    virtual void fill_rect(int x, int y, int w, int h, const GdColor &c);

    virtual unsigned char *get_pixels() const;
    virtual int get_pitch() const;
};

#endif
