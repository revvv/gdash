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

#include <stdexcept>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glib.h>

#include "sdl/sdlpixbuffactory.hpp"
#include "sdl/sdlpixbuf.hpp"
#include "sdl/sdlscreen.hpp"
#include "cave/colors.hpp"



std::unique_ptr<Pixbuf> SDLPixbufFactory::create_from_file(const char *filename) const {
    SDL_Surface *surface = IMG_Load(filename);
    if (!surface)       // if could not load
        throw std::runtime_error(std::string("cannot load image ") + IMG_GetError());
    return std::make_unique<SDLPixbuf>(surface);
}


std::unique_ptr<Pixbuf> SDLPixbufFactory::create_from_inline(int length, unsigned char const *data) const {
    SDL_RWops *rwop = SDL_RWFromConstMem(data, length);
    SDL_Surface *surface = IMG_Load_RW(rwop, 1);  // 1 = automatically closes rwop
    if (!surface)       // if could not load
        throw std::runtime_error(std::string("cannot load image ") + IMG_GetError());
    return std::make_unique<SDLPixbuf>(surface);
}


std::unique_ptr<Pixbuf> SDLPixbufFactory::create(int w, int h) const {
    return std::make_unique<SDLPixbuf>(w, h);
}


std::unique_ptr<Pixbuf> SDLPixbufFactory::create_composite_color(Pixbuf const &src, const GdColor &c, unsigned char a) const {
    SDLPixbuf const &srcsdl = static_cast<SDLPixbuf const &>(src);
    SDL_Surface *rect = SDL_CreateRGBSurface(0, srcsdl.get_surface()->w, srcsdl.get_surface()->h, 32, srcsdl.rmask, srcsdl.gmask, srcsdl.bmask, 0); /* no amask, as we set overall alpha! */
    if (!rect)
        throw std::runtime_error(std::string("could not create surface: ") + SDL_GetError());
    unsigned char r, g, b;
    c.get_rgb(r, g, b);
    SDL_FillRect(rect, NULL, SDL_MapRGB(rect->format, r, g, b));

    SDL_Surface *ret = SDL_CreateRGBSurface(0, srcsdl.get_surface()->w, srcsdl.get_surface()->h, 32, srcsdl.rmask, srcsdl.gmask, srcsdl.bmask, srcsdl.amask);
    if (!ret)
        throw std::runtime_error(std::string("could not create surface: ") + SDL_GetError());
    SDL_BlitSurface(srcsdl.get_surface(), NULL, ret, NULL);
    SDL_BlitSurface(rect, NULL, ret, NULL);
    SDL_FreeSurface(rect);
    return std::make_unique<SDLPixbuf>(ret);
}


std::unique_ptr<Pixbuf> SDLPixbufFactory::create_subpixbuf(Pixbuf &src, int x, int y, int w, int h) const {
    g_assert(x >= 0 && y >= 0 && x + w <= src.get_width() && y + h <= src.get_height());
    SDL_Surface *sub = SDL_CreateRGBSurfaceFrom(&src(x, y), w, h, 32, src.get_pitch(), src.rmask, src.gmask, src.bmask, src.amask);
    return std::make_unique<SDLPixbuf>(sub);
}
