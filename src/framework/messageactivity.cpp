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

#include "framework/messageactivity.hpp"
#include "framework/app.hpp"
#include "framework/commands.hpp"
#include "gfx/screen.hpp"
#include "gfx/fontmanager.hpp"
#include "misc/util.hpp"


MessageActivity::MessageActivity(App *app, std::string const &primary, std::string const &secondary, std::unique_ptr<Command> command_after_exit)
    :
    Activity(app),
    command_after_exit(std::move(command_after_exit)) {
    std::string text;
    if (secondary != "")
        text = Printf("%c%s\n\n%c%s", GD_COLOR_INDEX_WHITE, primary, GD_COLOR_INDEX_GRAY3, secondary);
    else
        text = Printf("%c%s", GD_COLOR_INDEX_WHITE, primary);
    wrapped_text = gd_wrap_text(text.c_str(), app->screen->get_width() / app->font_manager->get_font_width_narrow() - 6);
}


void MessageActivity::redraw_event(bool full) const {
    int height = (wrapped_text.size() + 2) * app->font_manager->get_line_height(); /* +2 empty lines */
    int y1 = (app->screen->get_height() - height) / 2; /* middle of the screen */
    int cx = 2 * app->font_manager->get_font_width_narrow(), cy = y1, cw = app->screen->get_width() - 2 * cx, ch = height;

    app->draw_window(cx, cy, cw, ch);
    app->screen->set_clip_rect(cx, cy, cw, ch);

    app->set_color(GD_GDASH_WHITE);
    for (size_t i = 0; i < wrapped_text.size(); ++i)
        app->font_manager->blittext_n(-1, y1 + (i + 1)*app->font_manager->get_line_height(), wrapped_text[i].c_str());

    app->screen->remove_clip_rect();

    app->screen->drawing_finished();
}


MessageActivity::~MessageActivity() {
    app->enqueue_command(std::move(command_after_exit));
}


void MessageActivity::keypress_event(KeyCode keycode, int gfxlib_keycode) {
    switch (keycode) {
        default:
            app->enqueue_command(std::make_unique<PopActivityCommand>(app));
            break;
        case 0:
            /* unknown key or modifier - do nothing */
            break;
    }
}
