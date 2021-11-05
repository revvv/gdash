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

#include "framework/inputtextactivity.hpp"
#include "framework/app.hpp"
#include "gfx/fontmanager.hpp"
#include "gfx/screen.hpp"

InputTextActivity::InputTextActivity(App *app, char const *title_line, const char *default_text, std::unique_ptr<Command1Param<std::string> > command_when_successful)
    :
    Activity(app),
    title(title_line),
    command_when_successful(std::move(command_when_successful)),
    ms(0), blink(false) {
    text = g_string_new(default_text);
}


InputTextActivity::~InputTextActivity() {
    g_string_free(text, TRUE);
}


void InputTextActivity::redraw_event(bool full) const {
    int height = 6 * app->font_manager->get_line_height();
    int y1 = (app->screen->get_height() - height) / 2; /* middle of the screen */
    int cx = 2 * app->font_manager->get_font_width_narrow(), cy = y1, cw = app->screen->get_width() - 2 * cx, ch = height;

    app->draw_window(cx, cy, cw, ch);
    app->screen->set_clip_rect(cx, cy, cw, ch);
    int width = cw / app->font_manager->get_font_width_narrow();

    app->set_color(GD_GDASH_WHITE);
    app->font_manager->blittext_n(-1, y1 + app->font_manager->get_line_height(), title.c_str());
    int len = g_utf8_strlen(text->str, -1);
    int x;
    if (len < width - 1)
        x = -1; /* if fits on screen (+1 for cursor), centered */
    else
        x = cx + cw - (len + 1) * app->font_manager->get_font_width_narrow(); /* otherwise show end, +1 for cursor */
    app->font_manager->blittext_n(x, y1 + 3 * app->font_manager->get_line_height(), "%s%c", text->str, blink ? '_' : ' ');
    app->screen->remove_clip_rect();
    app->screen->drawing_finished();
}


void InputTextActivity::keypress_event(KeyCode keycode, int gfxlib_keycode) {
    switch (keycode) {
        case App::Enter:
            command_when_successful->set_param1(text->str);
            app->enqueue_command(std::make_unique<PopActivityCommand>(app));
            app->enqueue_command(std::move(command_when_successful));
            break;
        case App::BackSpace:
            if (text->len != 0) {
                char *ptr = text->str + text->len; /* string pointer + length: points to the terminating zero */
                ptr = g_utf8_prev_char(ptr);  /* step back one utf8 character */
                g_string_truncate(text, ptr - text->str);
                queue_redraw();
            }
            break;
        case App::Escape:
            app->enqueue_command(std::make_unique<PopActivityCommand>(app));
            break;
        default:
            break;
    }
}


void InputTextActivity::textinput_event(char *appendtext) {
    g_string_append(text, appendtext);
    queue_redraw();
}

void InputTextActivity::textediting_event(char *overwritetext) {
    g_string_assign(text, overwritetext);
    queue_redraw();
}

void InputTextActivity::shown_event() {
    app->screen->start_text_input();
}    
    
void InputTextActivity::hidden_event() {
    app->screen->stop_text_input();
}


void InputTextActivity::timer_event(int ms_elapsed) {
    ms += ms_elapsed;
    if (ms > 400) {
        blink = !blink;
        ms -= 400;
        queue_redraw();
    }
}
