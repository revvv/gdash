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
#include <memory>
#include <SDL2/SDL_image.h>

#include "sdl/sdlscreen.hpp"
#include "sdl/sdlpixbuf.hpp"
#include "gfx/pixbuffactory.hpp"

#include "misc/logger.hpp"
#include "settings.hpp"


void SDLScreen::configure_size() {
    /* close window, if already exists, to create a new one */
    window.reset();
    renderer.reset();
    texture.reset();

    /* init screen */
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        SDL_Init(SDL_INIT_VIDEO);

        /* when the text input is started for the first time, SDL seems to process the last keydown event.
         * this is a problem, because the inputtextactivity will see the initiating keypress as text.
         * the problem disappears on the next invocations of text input mode.
         * so when creating the sdl window, we enable text input for a moment. */
        start_text_input();
        stop_text_input();
    }

    /* create screen */
    surface.reset(SDL_CreateRGBSurface(0, w, h, 32, Pixbuf::rmask, Pixbuf::gmask, Pixbuf::bmask, Pixbuf::amask));
    if (gd_fullscreen)
        window.reset(SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP));
    else
        window.reset(SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, 0));
    if (!window)
        throw ScreenConfigureException("cannot initialize sdl video");
    renderer.reset(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    SDL_RenderSetLogicalSize(renderer.get(), w, h);
    texture.reset(SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h));
    
    /* do not show mouse cursor */
    SDL_ShowCursor(SDL_DISABLE);
    /* warp mouse pointer so cursor cannot be seen, if the above call did nothing for some reason */
    SDL_WarpMouseInWindow(window.get(), w - 1, h - 1);

    /* title & icon */
    set_title("GDash");
    SDL_RWops *rwop = SDL_RWFromConstMem(Screen::gdash_icon_32_png, Screen::gdash_icon_32_size);
    std::unique_ptr<SDL_Surface, Deleter<SDL_Surface, SDL_FreeSurface>> icon(IMG_Load_RW(rwop, 1));  // 1 = automatically closes rwop
    SDL_SetWindowIcon(window.get(), icon.get());
}


void SDLScreen::set_title(char const *title) {
    SDL_SetWindowTitle(window.get(), title);
}


bool SDLScreen::must_redraw_all_before_flip() const {
    return false;
}


void SDLScreen::flip() {
    SDL_UpdateTexture(texture.get(), NULL, surface->pixels, surface->w * sizeof(Uint32));
    SDL_RenderClear(renderer.get());
    SDL_RenderCopy(renderer.get(), texture.get(), NULL, NULL);
    SDL_RenderPresent(renderer.get());
}


std::unique_ptr<Pixmap> SDLScreen::create_pixmap_from_pixbuf(Pixbuf const &pb, bool keep_alpha) const {
    SDL_Surface *to_copy = static_cast<SDLPixbuf const &>(pb).get_surface();
    SDL_Surface *newsurface = SDL_ConvertSurface(to_copy, surface->format, 0);
    return std::make_unique<SDLPixmap>(newsurface);
}

void SDLScreen::start_text_input() {
    SDL_StartTextInput();
}

void SDLScreen::stop_text_input() {
    SDL_StopTextInput();
}

