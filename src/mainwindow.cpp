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

#include <memory>

#include "cave/gamecontrol.hpp"
#include "gtk/gtkmainwindow.hpp"
#include "sdl/sdlmainwindow.hpp"
#include "misc/logger.hpp"
#include "mainwindow.hpp"
#include "settings.hpp"

/* The should be in the same order as the enum. */
const char *gd_graphics_engine_names[] = {
#ifdef HAVE_GTK
    "GTK+",
#endif
#ifdef HAVE_SDL
    "SDL",
    "OpenGL",
#endif
    NULL
};

void main_window_run_title_screen(CaveSet *caveset, NextAction &na) {
    /* if some unknown engine, switch to default */
    if (gd_graphics_engine >= GRAPHICS_ENGINE_MAX)
        gd_graphics_engine = GraphicsEngine(0);
        
    try {
        switch (GraphicsEngine(gd_graphics_engine)) {
            #ifdef HAVE_GTK
            case GRAPHICS_ENGINE_GTK:
                gd_main_window_gtk_run(caveset, na);
                break;
            #endif
            #ifdef HAVE_SDL
            case GRAPHICS_ENGINE_SDL:
                gd_main_window_sdl_run(caveset, na, false);
                break;
            case GRAPHICS_ENGINE_OPENGL:
                gd_main_window_sdl_run(caveset, na, true);
                break;
            #endif
            case GRAPHICS_ENGINE_MAX:
                g_assert_not_reached();
                break;
        }
    } catch (ScreenConfigureException const & ex) {
        gd_warning("Screen error: %s. Switching to default graphics engine.", ex.what());
        gd_graphics_engine = 0;
    }
}

void main_window_run_a_game(std::unique_ptr<GameControl> game) {
    /* if some unknown engine, switch to default */
    if (gd_graphics_engine >= GRAPHICS_ENGINE_MAX)
        gd_graphics_engine = GraphicsEngine(0);
        
    try {
        switch (GraphicsEngine(gd_graphics_engine)) {
            #ifdef HAVE_GTK
            case GRAPHICS_ENGINE_GTK:
                gd_main_window_gtk_run_a_game(std::move(game));
                break;
            #endif
            #ifdef HAVE_SDL
            case GRAPHICS_ENGINE_SDL:
                gd_main_window_sdl_run_a_game(std::move(game), false);
                break;
            case GRAPHICS_ENGINE_OPENGL:
                gd_main_window_sdl_run_a_game(std::move(game), true);
                break;
            #endif
            case GRAPHICS_ENGINE_MAX:
                g_assert_not_reached();
                break;
        }
    } catch (ScreenConfigureException const & ex) {
        gd_warning("Screen error: %s. Switching to default graphics engine.", ex.what());
        gd_graphics_engine = 0;
    }
}
