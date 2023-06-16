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

#include <SDL2/SDL_image.h>
#include <stdexcept>

#include "cave/colors.hpp"
#include "sdl/sdlpixbuf.hpp"


int SDLPixbuf::get_width() const {
    return surface->w;
}


int SDLPixbuf::get_height() const {
    return surface->h;
}


SDLPixbuf::SDLPixbuf(int w, int h) {
    surface.reset(SDL_CreateRGBSurface(0, w, h, 32, rmask, gmask, bmask, amask));
    if (!surface)
        throw std::runtime_error(std::string("could not create surface: ") + SDL_GetError());
}


SDLPixbuf::SDLPixbuf(SDL_Surface *surface_)
    : surface(surface_) {
    /* if the r/g/b/masks do not match our preset values, the image is not stored in R,G,B,[A] byte order
     * in memory. so convert it. */
    if (surface->format->BytesPerPixel != 4 || surface->format->Rmask != rmask || surface->format->Gmask != gmask || surface->format->Bmask != bmask) {
        std::unique_ptr<SDL_Surface, Deleter<SDL_Surface, SDL_FreeSurface>> newsurface(SDL_CreateRGBSurface(0, surface->w, surface->h, 32, rmask, gmask, bmask, amask));
        SDL_BlitSurface(surface.get(), NULL, newsurface.get(), NULL);
        surface = std::move(newsurface);
    }
}


void SDLPixbuf::fill_rect(int x, int y, int w, int h, const GdColor &c) {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;
    unsigned char r, g, b;
    c.get_rgb(r, g, b);
    SDL_FillRect(surface.get(), &dst, SDL_MapRGB(surface->format, r, g, b));
}


void SDLPixbuf::blit_full(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const {
    SDL_Rect src, dst;
    src.x = x;
    src.y = y;
    src.w = w;
    src.h = h;
    dst.x = dx;
    dst.y = dy;
    SDL_BlitSurface(surface.get(), &src, static_cast<SDLPixbuf &>(dest).surface.get(), &dst);
}


void SDLPixbuf::copy_full(int x, int y, int w, int h, Pixbuf &dest, int dx, int dy) const {
    SDL_Rect src, dst;
    src.x = x;
    src.y = y;
    src.w = w;
    src.h = h;
    dst.x = dx;
    dst.y = dy;
    SDL_BlitSurface(surface.get(), &src, static_cast<SDLPixbuf &>(dest).surface.get(), &dst);
}

void SDLPixbuf::scale_full(Pixbuf &dest, double scaling_factor) const {
    SDL_Rect src, dst;
    src.x = 0;
    src.y = 0;
    src.w = get_width();
    src.h = get_height();
    dst.x = 0;
    dst.y = 0;
    dst.w = get_width() * scaling_factor;
    dst.h = get_height() * scaling_factor;
    SDL_BlitScaled(surface.get(), &src, static_cast<SDLPixbuf &>(dest).surface.get(), &dst);
}

unsigned char *SDLPixbuf::get_pixels() const {
    return static_cast<unsigned char *>(surface->pixels);
}


int SDLPixbuf::get_pitch() const {
    return surface->pitch;
}
