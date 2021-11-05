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

#include <glib/gi18n.h>
#include <memory>

#include "framework/askyesnoactivity.hpp"
#include "framework/app.hpp"
#include "framework/commands.hpp"
#include "gfx/fontmanager.hpp"
#include "gfx/screen.hpp"


// TRANSLATORS: This string should be translated to the first letter of "yes" in your
// language, uppercase. For example, the English "yes" -> the strings is "Y".
// It should rather not contain an accented character, as the user might not be able
// to type it if he does not have the correct keyboard layout.
static char const *yesletter = N_("Y");
gunichar yeschar = 0;
// TRANSLATORS: This string should be translated to the first letter of "no" in your
// language, uppercase. For example, the English "no" -> the strings is "N".
// It might contain an accented character, but make sure the user can easily type it!
// It should rather not contain an accented character, as the user might not be able
// to type it if he does not have the correct keyboard layout.
static char const *noletter = N_("N");
gunichar nochar = 0;

AskYesNoActivity::AskYesNoActivity(App *app, char const *question, const char *yes_answer, char const *no_answer, std::unique_ptr<Command> command_when_yes, std::unique_ptr<Command> command_when_no)
    :
    Activity(app),
    question(question),
    yes_answer(yes_answer), no_answer(no_answer),
    command_when_yes(std::move(command_when_yes)), command_when_no(std::move(command_when_no)) {
    // These are global, but no problem if updated every time
    yeschar = g_utf8_get_char(_(yesletter));
    nochar = g_utf8_get_char(_(noletter));
}


void AskYesNoActivity::redraw_event(bool full) const {
    int height = 6 * app->font_manager->get_line_height();
    int y1 = (app->screen->get_height() - height) / 2; /* middle of the screen */
    int cx = 2 * app->font_manager->get_font_width_narrow(), cy = y1, cw = app->screen->get_width() - 2 * cx, ch = height;

    app->draw_window(cx, cy, cw, ch);
    app->screen->set_clip_rect(cx, cy, cw, ch);
    app->set_color(GD_GDASH_WHITE);
    app->font_manager->blittext_n(-1, y1 + app->font_manager->get_line_height(), question.c_str());
    app->font_manager->blittext_n(-1, y1 + 3 * app->font_manager->get_line_height(), "%s: %s, %s: %s", _(noletter), no_answer, _(yesletter), yes_answer);
    app->screen->remove_clip_rect();

    app->screen->drawing_finished();
}


void AskYesNoActivity::keypress_event(KeyCode keycode, int gfxlib_keycode) {
    if (g_unichar_toupper(keycode) == g_unichar_toupper(yeschar)) {
        app->enqueue_command(std::make_unique<PopActivityCommand>(app));
        app->enqueue_command(std::move(command_when_yes));
    }
    if (g_unichar_toupper(keycode) == g_unichar_toupper(nochar)) {
        app->enqueue_command(std::make_unique<PopActivityCommand>(app));
        app->enqueue_command(std::move(command_when_no));
    }
}
