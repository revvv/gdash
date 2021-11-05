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

#ifndef SDLSCREEN_HPP_INCLUDED
#define SDLSCREEN_HPP_INCLUDED

#include "config.h"

#include "sdl/sdlabstractscreen.hpp"
#include "misc/deleter.hpp"

class PixbufFactory;


class SDLScreen: public SDLAbstractScreen {
private:
    std::unique_ptr<SDL_Window, Deleter<SDL_Window, SDL_DestroyWindow>> window;
    std::unique_ptr<SDL_Renderer, Deleter<SDL_Renderer, SDL_DestroyRenderer>> renderer;
    std::unique_ptr<SDL_Texture, Deleter<SDL_Texture, SDL_DestroyTexture>> texture;

public:
    SDLScreen(PixbufFactory &pixbuf_factory): SDLAbstractScreen(pixbuf_factory) {}
    virtual void configure_size() override;
    virtual void set_title(char const *title) override;
    virtual bool must_redraw_all_before_flip() const override;
    virtual void flip() override;
    virtual std::unique_ptr<Pixmap> create_pixmap_from_pixbuf(Pixbuf const &pb, bool keep_alpha) const override;

    virtual void start_text_input() override;
    virtual void stop_text_input() override;
};


#endif
