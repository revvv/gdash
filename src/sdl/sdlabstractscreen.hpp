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

#ifndef SDLABSTRACTSCREEN_HPP_INCLUDED
#define SDLABSTRACTSCREEN_HPP_INCLUDED

#include "config.h"

#include <SDL2/SDL.h>
#include <memory>

#include "gfx/screen.hpp"
#include "misc/deleter.hpp"

class ParticleSet;
class GdColor;
class PixbufFactory;




class SDLPixmap: public Pixmap {
protected:
    std::unique_ptr<SDL_Surface, Deleter<SDL_Surface, SDL_FreeSurface>> surface;

public:
    friend class SDLAbstractScreen;
    explicit SDLPixmap(SDL_Surface *surface_) : surface(surface_) {}

    virtual int get_width() const;
    virtual int get_height() const;
};

class SDLAbstractScreen: public Screen {
protected:
    std::unique_ptr<SDL_Surface, Deleter<SDL_Surface, SDL_FreeSurface>> surface;

public:
    SDLAbstractScreen(PixbufFactory &pixbuf_factory): Screen(pixbuf_factory) {}
    virtual void fill_rect(int x, int y, int w, int h, const GdColor &c) override;
    virtual void blit(Pixmap const &src, int dx, int dy) const override;
    virtual void set_clip_rect(int x1, int y1, int w, int h) override;
    virtual void remove_clip_rect() override;
    virtual void draw_particle_set(int dx, int dy, ParticleSet const &ps) override;
};

#endif
