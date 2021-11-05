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
#include <glib/gi18n.h>
#include <cstring>
#include <memory>

#include "framework/showtextactivity.hpp"
#include "framework/commands.hpp"
#include "framework/app.hpp"
#include "gfx/screen.hpp"
#include "gfx/fontmanager.hpp"
#include "misc/util.hpp"


ShowTextActivity::ShowTextActivity(App *app, char const *title_line, std::string const &text, std::unique_ptr<Command> command_after_exit)
    :
    Activity(app),
    command_after_exit(std::move(command_after_exit)),
    title_line(title_line) {
    wrapped_text = gd_wrap_text(text.c_str(), app->screen->get_width() / app->font_manager->get_font_width_narrow() - 4);
    linesavailable = app->screen->get_height() / app->font_manager->get_line_height() - 4;
    if ((int)wrapped_text.size() < linesavailable)
        scroll_max_y = 0;
    else
        scroll_max_y = wrapped_text.size() - linesavailable;
    scroll_y = 0;
}


void ShowTextActivity::redraw_event(bool full) const {
    app->clear_screen();

    app->title_line(title_line.c_str());
    // TRANSLATORS: 40 chars max
    app->status_line(_("Crsr: move     Space: exit"));

    // text & scrollbar
    app->set_color(GD_GDASH_LIGHTBLUE);
    for (int l = 0; l < linesavailable && scroll_y + l < (int)wrapped_text.size(); ++l)
        app->font_manager->blittext_n(app->font_manager->get_font_width_narrow() * 2,
                        l * app->font_manager->get_line_height() + app->font_manager->get_line_height() * 2, wrapped_text[scroll_y + l].c_str());

    app->draw_scrollbar(0, scroll_y, scroll_max_y);

    app->screen->drawing_finished();
}


ShowTextActivity::~ShowTextActivity() {
    app->enqueue_command(std::move(command_after_exit));
}


void ShowTextActivity::keypress_event(KeyCode keycode, int gfxlib_keycode) {
    switch (keycode) {
        case App::PageUp:
            scroll_y -= linesavailable - 1;
            if (scroll_y < 0)
                scroll_y = 0;
            queue_redraw();
            break;
        case App::Up:
            if (scroll_y > 0) {
                scroll_y--;
                queue_redraw();
            }
            break;
        case App::PageDown:
            scroll_y += linesavailable - 1;
            if (scroll_y > scroll_max_y)
                scroll_y = scroll_max_y;
            queue_redraw();
            break;
        case App::Down:
            if (scroll_y < scroll_max_y) {
                scroll_y++;
                queue_redraw();
            }
            break;
        case App::Home:
            if (scroll_y > 0) {
                scroll_y = 0;
                queue_redraw();
            }
            break;
        case App::End:
            if (scroll_y < scroll_max_y) {
                scroll_y = scroll_max_y;
                queue_redraw();
            }
            break;
        default:
            app->enqueue_command(std::make_unique<PopActivityCommand>(app));
            break;
        case 0:
            /* unknown key or modifier - do nothing */
            break;
    }
}
